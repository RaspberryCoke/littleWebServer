#pragma once
#include<unistd.h>
#include<iostream>
#include<sys/epoll.h> 
#include<vector>
#include"assert.h"
using namespace std;
class epoller {
public:
	epoller(int num = 100);
	bool add_fd(int fd, __uint32_t  events);
	bool mod_fd(int fd, __uint32_t events);
	bool del_fd(int fd);
	int wait(int);
	epoll_event operator[](int i);
private:
	int epfd;
	vector<epoll_event> _events;
};