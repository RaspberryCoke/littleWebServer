#include<iostream>
#include<regex>
#include<string>
#include<unordered_map>
#include<unordered_set>
#include"buffer.h"
#include"sqlpool.h"
using namespace std;

class httprequest{
public:
    enum PARSE_STATE {
        REQUEST_LINE,
        HEADERS,
        BODY,
        FINISH,        
    };

    enum HTTP_CODE {
        NO_REQUEST = 0,
        GET_REQUEST,
        BAD_REQUEST,
        NO_RESOURSE,
        FORBIDDENT_REQUEST,
        FILE_REQUEST,
        INTERNAL_ERROR,
        CLOSED_CONNECTION,
    };

    httprequest(){clog<<"[log]:httprequest()"<<endl;}
    ~httprequest(){clog<<"[log]:~httprequest()"<<endl;}

    bool parse(buffer&);

private:
    bool parse_request_line(const string&);
    void parse_header(const string&);
    void parse_body(const string&);

    void parse_path(const string&);
    void parse_post(const string&);
    void parse_from_url();
public:
    PARSE_STATE _state=REQUEST_LINE;
    string _method={},_path={},_version={},_body={};
    unordered_map<string,string> _headers;
    unordered_map<string,string> _post;

    static const std::unordered_set<std::string> DEFAULT_HTML;
    static const std::unordered_map<std::string, int> DEFAULT_HTML_TAG;

    static int ConverHex(char ch);
};