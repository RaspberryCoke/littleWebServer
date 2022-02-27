#include "epoller.h"
epoller::epoller(int num) 
{
	clog << "[log]:epoller:epoller()" << endl;
	epfd=epoll_create(100);
	if (epfd <= 0)
	{
		clog << "[err]:epoller:epoller():epoll_create error" << endl;
		perror("[err]:epoller:epoller():epoll_create ");
	}
	_events.resize(100, {});
}
bool epoller::add_fd(int fd, __uint32_t  events)
{
	epoll_event event{ 0 };
	event.data.fd = fd;
	event.events = events;
	int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event);
	if (ret < 0)
	{
		clog << "[err]:epoller:addfd(): error" << endl;
		perror("[err]:epoller:epoller():addfd ");
		return false;
	}
	return true;
}
bool epoller::mod_fd(int fd, __uint32_t events)
{
	epoll_event event{ 0 };
	event.data.fd = fd;
	event.events = events;
	int ret = epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &event);
	if (ret < 0)
	{
		clog << "[err]:epoller:modfd(): error" << endl;
		perror("[err]:epoller:modfd: ");
		return false;
	}
	return true;
}
bool epoller::del_fd(int fd)
{
	int ret = epoll_ctl(epfd, EPOLL_CTL_DEL, fd, nullptr);
	if (ret < 0)
	{
		clog << "[err]:epoller:modfd(): error" << endl;
		perror("[err]:epoller:modfd: ");
		return false;
	}
	return true;
}
int epoller::wait(int timeout)
{
	clog<<"[log]:epoller:wait"<<endl;
	return epoll_wait(epfd, &*_events.begin(), _events.size(), timeout);
}
epoll_event epoller::operator[](int i)
{
	return _events[i];
}