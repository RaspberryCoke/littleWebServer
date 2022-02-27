#include"httprequest.h"

const unordered_set<string> httprequest::DEFAULT_HTML{
            "/index.html", };

const unordered_map<string, int> httprequest::DEFAULT_HTML_TAG {
            {"/register.html", 0}, {"/login.html", 1},  };

bool httprequest::parse(buffer& buff)
{
    const char* crlf="\r\n";
    int size=buff.get_s_pos()-buff.get_f_pos();
    if(size<=0)return false;
    while(buff.get_s_pos()-buff.get_f_pos()>0&&_state!=FINISH)
    {
        char* line_end=search(buff.get_f_ptr(),buff.get_s_ptr(),crlf,crlf+2);
        string line(buff.get_f_ptr(),line_end);//get the line
        switch(_state)
        {
            case REQUEST_LINE:
                if(!parse_request_line(line))
                {
                    return false;
                }
                parse_path(_path);
                break;

            case HEADERS:
                parse_header(line);
                if(buff.get_s_pos()-buff.get_f_pos()<=2)
                {
                    _state=FINISH;
                }
                break;

            case BODY:
                parse_body(line);
                break;

            default:
                break;
        }
        if(line_end==buff.get_s_ptr())break;
        buff.retrieve_until(line_end+2);
    }
    clog<<"[log]:httprequest:parse succeed"<<endl;
    return true;
}

bool httprequest::parse_request_line(const string & line)
{
    regex patten("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
    smatch match;
    if(regex_match(line,match,patten))
    {
        _method=match[1];
        _path=match[2];
        _version=match[3];
        _state=HEADERS;
        clog<<"[tmp]:parse method :"<<_method<<endl;
        clog<<"[tmp]:parse path :"<<_path<<endl;
        clog<<"[tmp]:parse version :"<<_version<<endl;
        return true;
    }
    return false;
}

void httprequest::parse_header(const string& line)
{
    regex patten("^([^ ]*):([^ ]*)$");
    smatch match;
    if(regex_match(line,match,patten))
    {
        _headers[match[1]]=match[2];
    }else{
        _state=FINISH;
    }
}

void httprequest::parse_path(const string& line)
{
    if(line=="/")
    {
        _path="/index.html";
        return;
    }
    if(DEFAULT_HTML.count(line)!=0)
    {
        _path=line;
    }
    else{
        _path="/index.html";
        clog<<"[log]:httprequest:parse_path: client send a wrong path,default index.html"<<endl;
    }
}

void httprequest::parse_body(const string& line){}