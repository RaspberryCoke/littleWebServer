#pragma once
#include<iostream>
#include<string>
#include<unistd.h>
#include<sys/socket.h>
#include<unordered_set>
#include<functional>
#include<assert.h>
#include<string>
#include<stdlib.h>
#include<assert.h>
#include<sys/types.h>
#include<unistd.h>
#include<memory>
#include<cstring>
#include <errno.h>
#include<mutex>
#include<signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unordered_map>
#include <sys/epoll.h> 
#include<fcntl.h>
#include"epoller.h"
#include"threadpool.h"
#include"httpconn.h"
#include"sqlpool.h"
using namespace std;
class server
{
public:
	server(int listenPort = 80, int sqlPort = 3036, string sqlName = "root", string sqlPass = "", string databaseName = "users");
	~server();
	void work();
private:
	void read_form_conn(httpconn*);
	void write_to_conn(httpconn*);

	bool check_for_close_conn(int sock);
	bool check_for_close_conn(httpconn*);

	epoller _epoller;
	sqlpool _sqlpool;
	threadpool _threadpool;

	int _sock;
	int __received_requests_nums;
	int __current_users_nums;
	bool _need_stop = false;

	unordered_map<int,httpconn*> _map_of_fd_httpconn;

	__uint32_t _listen_event_mode = EPOLLRDHUP | EPOLLET | EPOLLIN;
	__uint32_t _conn_event_mode = EPOLLRDHUP | EPOLLET | EPOLLONESHOT;

	static int _sock_pair[2];
	static void sig_function(int sig);
	long long all_requests_received=0;
	long long current_users=0;

	mutex mut_of_map;
};

