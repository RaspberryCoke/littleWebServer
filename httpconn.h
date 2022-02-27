#pragma once
#include<sys/socket.h>
#include<unistd.h>
#include <sys/uio.h> 
#include"buffer.h"
#include"httprequest.h"
#include"httpresponse.h"
class httpconn
{
public:
	httpconn(int sockfd);
	~httpconn();
	const int get_sock();
	int read_data();
	int send_data();
	bool _need_close = false;
	int parse_http() ;
	bool process();

	static const char* srcDir;
    static std::atomic<int> userCount;

private:
	buffer _in_buff;
	buffer _out_buff;
	int _sockfd;

	struct iovec _iov[2];
	string path;// todo
public:
	httprequest _request;
	HttpResponse _response;
};

