#include "server.h"
int server::_sock_pair[2] = { 0 };
void server::sig_function(int sig)
{
	clog << "[sig]:get signal" << endl;
	int save_errno = errno;
	int msg = sig;
	int ret = send(_sock_pair[1], (void*)&msg, 1, 0);
	if (ret < 0)
	{
		clog << "[err]:signal handler send msg to pipe error" << endl;
		perror("[err]:signal handler send msg :");
	}
	errno = save_errno;
}

server::server(int listenPort, int sqlPort, string sqlName, string sqlPass, string databaseName) :_sqlpool(sqlPort, sqlName, sqlPass, databaseName), _threadpool(12), _epoller(),mut_of_map()
{
	clog << "[log]:server:server()" << endl;
	int ret = 0;
	//signal
	ret = socketpair(PF_UNIX, SOCK_STREAM, 0, _sock_pair);
	if (ret < 0)
	{
		clog << "[err]:server:server():socketpair error" << endl;
		perror("[err]:server:server():socketpair:");
		assert(ret > 0);
	}
	auto addsig = [](int signal)
	{
		struct sigaction sig { 0 };
		sig.sa_flags = SA_RESTART | SA_NODEFER;
		sig.sa_handler = sig_function;
		int ret = sigaction(SIGINT, &sig, nullptr);
		if (ret < 0)
		{
			clog << "[err]:server:server():sigaction error" << endl;
			perror("[err]:server:server():sigaction:");
			assert(ret > 0);
		}
	};
	addsig(SIGINT);
	addsig(SIGCHLD);
	addsig(SIGHUP);
	addsig(SIGTERM);
	addsig(SIGURG);
	addsig(SIGALRM);
	addsig(SIGPIPE);

	//init socket
	ret=socket(PF_INET, SOCK_STREAM, 0);
	if (ret < 0)
	{
		clog << "[err]:server:server():socket error" << endl;
		perror("[err]:server:server():socket:");
		assert(ret > 0);
	}

	_sock = ret;

	//set reuseaddr
	int reuseOpt = 1;
	ret = setsockopt(_sock, SOL_SOCKET, SO_REUSEADDR, (void*)&reuseOpt, (socklen_t)sizeof(reuseOpt));
	if (ret < 0)
	{
		clog << "[log]:server:server():setsockopt:reuse port error" << endl;
		perror("[log]:server:server():setsockopt:");
		assert(ret>0);
	}

	struct linger _linger{0};
	_linger.l_linger = 1;
	_linger.l_onoff = 1;

	ret = setsockopt(_sock, SOL_SOCKET, SO_LINGER, (void*)&_linger, (socklen_t)sizeof(_linger));
	if (ret < 0)
	{
		clog << "[err]:server:server():setsockopt:linger error" << endl;
		perror("[err]:server:server():setsockopt:");
		assert(ret>0);
	}

	ret = fcntl(_sock, F_SETFL, fcntl(_sock, F_GETFL, 0) | O_NONBLOCK);
	if (ret < 0)
	{
		clog << "[err]:server:server():fcntl:set nonblock error" << endl;
		perror("[err]:server:server():fcntl:");
		assert(ret > 0);
	}

	//bind
	sockaddr_in addr{ 0 };
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port = htons(listenPort);
	addr.sin_family = AF_INET;

	ret = bind(_sock, (sockaddr*)&addr, (socklen_t)sizeof(addr));
	if (ret < 0)
	{
		clog << "[err]:server:server():bind error" << endl;
		perror("[err]:server:server():bind:");
		assert(ret > 0);
	}

	//listen
	ret = listen(_sock, 6);
	if (ret < 0)
	{
		clog << "[err]:server:server():listen error" << endl;
		perror("[err]:server:server():listen:");
		assert(ret > 0);
	}

	//epoller add sock to listen
	ret = _epoller.add_fd(_sock, _listen_event_mode);
	if (ret < 0)
	{
		clog << "[err]:server:server():epoll add fd error" << endl;
		perror("[err]:server:server():epoll:");
		assert(ret > 0);
	}
	ret = _epoller.add_fd(_sock_pair[0], _listen_event_mode);
	if (ret < 0)//not right , can not handle multi signals
	{
		clog << "[err]:server:server():epoll add _sock_pair error" << endl;
		perror("[err]:server:server():epoll:");
		assert(ret > 0);
	}

	clog << "[log]:server:server():server initial ended" << endl;
	clog << "[log]:server:server():server bind port"<<listenPort << endl;
	clog << "[log]:server:server():initial successfully!" << endl;
}

void server::work()
{
	clog << "[log]:server::work() begin" << endl;
	while (!_need_stop)
	{
		int nums = _epoller.wait(-1);
		clog<<"[log]:server::work: new event nums :"<<nums<<endl;
		if (nums < 0 && errno != EINTR)
		{
			clog << "[err]:server::work():epoll_wait error" << endl;
			perror("[err]:server::work():epoll_wait :");
			_need_stop = true;
		}
		else
		{
			for (int i = 0; i < nums; i++)
			{
				int sockfd = _epoller[i].data.fd;
				__uint32_t events = _epoller[i].events;
				if (events & EPOLLIN)
				{
					clog << "[log]:server::work():EPOLLIN" << endl;
					if (sockfd == _sock)
					{//accept...
						clog << "[log]:server::work(): accept" << endl;
						int ret = 1;
						sockaddr_in addr{ 0 };
						socklen_t len = 1;
						while (ret = accept(_sock, (sockaddr*) &addr, &len))
						{
							if(ret<0)
							{
								clog << "[log]:server::work()::epoller:accept error" << endl;
								perror("[log]:server::work()::epoller:accept :");
								break;
							}
							if (!_epoller.add_fd(ret, _conn_event_mode|EPOLLIN))
							{
								clog << "[err]:server::work()::epoller:add_fd error" << endl;
								perror("[err]:server::work()::epoller:add_fd :");
								break;
							}
							httpconn* _ptr;
							{
							unique_lock<mutex> locker(mut_of_map);
							_ptr=_map_of_fd_httpconn[ret]=new httpconn(ret);//sock resource is pass to httpconn to close
							}
							if(_ptr==nullptr)
							{
								clog << "[err]:server::work()::initial new httpconn ptr failed" << endl;
								break;
							}
							fcntl(ret, F_SETFL, fcntl(ret, F_GETFL, 0) | O_NONBLOCK);
							all_requests_received++;
							current_users++;
							//clog << "[log]:server::work():new connection " << inet_ntoa(addr.sin_addr) << endl;
						}
					}
					else if (sockfd == _sock_pair[0])
					{
						clog << "[sig]:server::work(): handle" << endl;
						int t=0;
						recv(sockfd,(char*)&t,sizeof(t),0);
						switch (t)
						{
						case SIGINT:
						{
							//clog << "[sig]:server::work(): SIGINT signal,may be ctrl^c" << endl;
							//clog<<"[log]:information: all requests received: "<<all_requests_received<<endl;
							//clog<<"[log]:information: current users: "<<current_users<<endl;
							//clog << "[sig]:server::work(): stop running,please input 'q' and enter to exit process,other char to continue" << endl;
							char _ch{ 0 };
							cin >> _ch;
							if (_ch == 'q')
							{
								//clog << "[log]:server::work(): wait to exit..." << endl;
								_need_stop = true;
							}
							break;
						}
						case SIGALRM:
							//clog << "[sig]:server::work(): SIGALRM signal" << endl;
							break;
						case SIGCHLD:
							//clog << "[sig]:server::work(): SIGCHLD signal" << endl;
							break;
						case SIGHUP:
							//clog << "[sig]:server::work(): SIGHUP signal" << endl;
							break;
						case SIGTERM:
							//clog << "[sig]:server::work(): SIGTERM signal" << endl;
							//clog << "[sig]:server::work(): wait to exit" << endl;
							_need_stop = true;
						case SIGURG:
							//clog << "[sig]:server::work(): SIGURG signal" << endl;
							break;
						case SIGPIPE:
							//clog << "[sig]:server::work(): SIGPIPE signal" << endl;
							break;
						default:
							break;
						}
					}
					else {//read
						//clog << "[log]:server::work(): read" << endl;

						httpconn* conn_ptr;
						int _count;
						{
						unique_lock<mutex> locker(mut_of_map);
						_count=_map_of_fd_httpconn.count(sockfd);
						if(_count>0)conn_ptr = _map_of_fd_httpconn[sockfd];
						}
						if(_count==0)
						{
							clog<<"[err]:server::work():read: the sockfd was not found in map"<<endl;
							continue;
						}
						
						if(conn_ptr==nullptr)
						{
							clog<<"[err]:server::work():read: the conn_ptr is nullptr"<<endl;
							continue;
						}
						if(conn_ptr->get_sock()<=0)
						{
							clog<<"[err]:server::work():read: the sockfd which is recorded in conn_ptr was not initial(sockfd<0)"<<endl;
							continue;
						}
						//clog<<"[log]:server::work():read: from conn "<<conn_ptr<<" with the socket: "<<conn_ptr->get_sock()<<endl;
						_threadpool.add_task(std::bind(&server::read_form_conn,this,conn_ptr));//mod fd in func
					}
				}
				else if (events & EPOLLOUT)
				{//write
					//clog << "[log]:server::work():EPOLLOUT" << endl;
					httpconn* conn_ptr{};
					int _count{};
					{
						unique_lock<mutex> locker(mut_of_map);
						_count=_map_of_fd_httpconn.count(sockfd);
						if(_count>0)conn_ptr = _map_of_fd_httpconn[sockfd];
					}
					if(_count==0)
						{
							clog<<"[err]:server::work():write: the sockfd was not found in map"<<endl;
							continue;
						}
						if(conn_ptr==nullptr)
						{
							clog<<"[err]:server::work():write: the conn_ptr is nullptr"<<endl;
							continue;
						}
						if(conn_ptr->get_sock()<=0)
						{
							clog<<"[err]:server::work():write: the sockfd which is recorded in conn_ptr was not initial(sockfd<0)"<<endl;
							continue;
						}
						//clog<<"[log]:server::work():write: from conn "<<conn_ptr<<" with the socket: "<<conn_ptr->get_sock()<<endl;
						try{_threadpool.add_task(std::bind(&server::write_to_conn, this, conn_ptr));//del fd in func
						}catch(exception& ex)
						{
							clog<<"[err]:server::work():write: "<<ex.what()<<endl;
						}
				}
				else {//delete sockfd
					//clog << "[log]:server::work():EPOLLERR" << endl;
					httpconn* conn_ptr{};
					int _count{};
					{
						unique_lock<mutex> locker(mut_of_map);
						_count=_map_of_fd_httpconn.count(sockfd);
						if(_count>0)conn_ptr = _map_of_fd_httpconn[sockfd];
					}
					if (_count > 0)
					{
						conn_ptr->_need_close = true;
						check_for_close_conn(conn_ptr);
					}
				}
			}
		}
	}

}

server::~server()
{
	unique_lock<mutex> locker(mut_of_map);
	clog << "[log]:server::~server()" << endl;
	for (auto& x : _map_of_fd_httpconn)
	{
		delete x.second;
	}
}

void server::read_form_conn(httpconn* conn)//need more argument
{
	//clog << "[log]:server:read_form_conn() begin" << endl;
	if(conn==nullptr)
	{
		clog<<"[err]:server:read_form_conn() conn is nullptr"<<endl;
		return;
	}

	//this function is asynchronous from main thread,so do not return results;
	int ret = 1;
	ret = conn->read_data();
	if (ret < 0)
	{
		clog << "[err]:server:read_form_conn():read_data error" << endl;
		perror("[err]:server:read_form_conn():read_data ");
		conn->_need_close = true;
		check_for_close_conn(conn);
		return;
	}
	ret = conn->parse_http();
	if (ret < 0)
	{
		clog << "[err]:server:read_form_conn():parse_http error" << endl;
		conn->_need_close = true;
		check_for_close_conn(conn);
		return;
	}
	_epoller.mod_fd(conn->get_sock(), _conn_event_mode | EPOLLOUT);
}

void server::write_to_conn(httpconn* conn)
{
	//clog << "[log]:server:write_to_conn() begin" << endl;
	if(!conn->process())
	{
		clog<<"[err]:server:write_to_conn():conn->process() error"<<endl;
		return;
	}
	int ret = -1;
	ret = conn->send_data();
	if (ret == 0)
	{
		//clog << "[log]:server:write_to_conn() need next write" << endl;
		_epoller.mod_fd(conn->get_sock(), _conn_event_mode | EPOLLOUT);
	}
	else if (ret < 0)
	{
		clog << "[err]:server:write_to_conn():error" << endl;
		conn->_need_close = true;
		check_for_close_conn(conn);
	}
	else if (ret > 0) {
		//clog << "[log]:server:write_to_conn() succeed" << endl;
		conn->_need_close = true;
		check_for_close_conn(conn);
	}
}



bool server::check_for_close_conn(httpconn* conn)
{
	if(conn==nullptr)
	{
		clog<<"[err]:server:check_for_close_conn : httpconn*conn==nullptr"<<endl;
		return false;
	}
	int sock = conn->get_sock();
	if (!conn->_need_close)return false;
	clog << "[log]:server::check_for_close_conn close a connection" << endl;
	_epoller.del_fd(sock);
	if(conn)delete conn;
	httpconn* conn_ptr{};
	int _count{};
	{
	unique_lock<mutex> locker(mut_of_map);
	_count=_map_of_fd_httpconn.count(sock);
	if(_count>0) _map_of_fd_httpconn.erase(sock);
	current_users--;
	}
	return true;
}
