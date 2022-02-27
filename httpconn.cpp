#include "httpconn.h"
httpconn::httpconn(int sock) : _in_buff(), _out_buff(), _sockfd(sock)
{
	clog << "[log]:httpconn:httpconn() with socket: " << sock << endl;
	if (sock < 0)
		clog << "[log]:httpconn:httpconn() error " << endl;
	_need_close = false;
}

httpconn::~httpconn()
{
	clog << "[log]:httpconn:~httpconn()" << endl;
	if (_sockfd > 0)
		close(_sockfd);
}

const int httpconn::get_sock()
{
	if (_sockfd > 0)
		return _sockfd;
	else
		return -1;
}

int httpconn::send_data()
{
	if (_need_close)
	{
		return -1;
	}
	//clog << "[log]:httpconn:send_data begin" << endl;
	int ret = false;
	if (_sockfd < 0)
	{
		//clog << "[err]:httpconn:send_data:socket fd error" << endl;
		return -1;
	}
	ssize_t len = -1;
    do {
        len = writev(_sockfd, _iov, 2);
        if(len <= 0) {
            break;
        }
        if(_iov[0].iov_len + _iov[1].iov_len  == 0) { break; } /* 传输结束 */
        else if(static_cast<size_t>(len) > _iov[0].iov_len) {
            _iov[1].iov_base = (uint8_t*) _iov[1].iov_base + (len - _iov[0].iov_len);
            _iov[1].iov_len -= (len - _iov[0].iov_len);
            if(_iov[0].iov_len) {
                _out_buff.retrieve_all();
                _iov[0].iov_len = 0;
            }
        }
        else {
            _iov[0].iov_base = (uint8_t*)_iov[0].iov_base + len; 
            _iov[0].iov_len -= len; 
            _out_buff.retrieve(len);
        }
    } while(true);
	if(errno==EWOULDBLOCK||errno==EAGAIN)
	{
		return 1;
	}else{
		if(len>0)return 0;
		else return -1;
	}
}

int httpconn::read_data()
{
	//check close status
	if (_need_close)
	{
		return -1;
	}
	//clog << "[log]:httpconn:read_data() begin" << endl;
	int ret = false;
	int sock = _sockfd;
	if (sock < 0)
	{
		//clog << "[err]:httpconn:read_data:get_sock() error" << endl;
		return -1;
	}
	while (true)
	{
		
		//clog << "[log]:httpconn:read_data() run" << endl;
		int size = _in_buff.size() - _in_buff.get_f_pos();//check size of buffer
		if (size <= 0)
		{
			//clog<<"[log]:httpconn:read_data():buffer too small,extend()"<<endl;
			if (!_in_buff.extend()) // system memory is full
			{
				//clog << "[err]:httpconn:read_data():_in_buff.extend() error" << endl;
				_need_close = true;
				_in_buff.clear();
				return -1;
			}
			size = _in_buff.size() - _in_buff.get_f_pos();//get new size
		}
		//clog<<"[log]:httpconn:read_data():buffer size is"<<size<<" begin to read"<<endl;
		ret = read(sock, (void*)_in_buff.get_s_ptr(), size);// one time read
		if (ret > 0)
		{
			//clog<<"[log]:httpconn:read_data():get new data "<<ret<<" bytes"<<endl;
			size -= ret;
			_in_buff.add_s_pos(ret);//the second ptr add
		}
		else if (ret < 0 && (errno != EAGAIN || errno != EWOULDBLOCK))
		{
			//clog << "[err]:httpconn:read_data(): error" << endl;
			_need_close = true;
			_in_buff.clear(); // clear() is O(n^2)
			return -1;
		}
		else
		{
			_in_buff.shrink_to_fit();
			//clog << "[log]:httpconn:read_data() succeed" << endl;
			//clog<<"[data]:"<<endl;
			//cout<<_in_buff<<endl;
			return 1;
		}
		
	}
}

int httpconn::parse_http()
{
	//clog<<"[log]:httpconn:parse_http begin"<<endl;
	if(_in_buff.get_s_pos()-_in_buff.get_f_pos()<=0)
	{
		//clog<<"[log]:httpconn:parse_http failed:buffer has been read"<<endl;
		_need_close=1;
		return -1;
	}
	if(!_request.parse(_in_buff))
	{
		//clog<<"[log]:httpconn:parse_http failed:http parse failed"<<endl;
		_need_close=1;
		return -1;
	}
	return 0;
}

bool httpconn::process()
{
	if(_in_buff.get_s_pos()-_in_buff.get_f_pos()==0)return false;
	if(_request.parse(_in_buff))
	{
		_response.Init("/home/a/code/f_server/resource",_request._path,0,200);
	}else{
		_response.Init("/home/a/code/f_server/resource",_request._path,0,400);
	}

	_response.MakeResponse(_out_buff);
	_iov[0].iov_base=(void*)_out_buff.get_begin_ptr();
	_iov[0].iov_len=_out_buff.get_s_ptr()-_out_buff.get_begin_ptr();
	cout<<_iov[0].iov_len<<endl;
	if(_response.FileLen() > 0  && _response.File()) {
        _iov[1].iov_base = _response.File();
        _iov[1].iov_len = _response.FileLen();
    }
	return true;
}