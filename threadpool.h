#pragma once
#include<iostream>
#include<mutex>
#include<thread>
#include<functional>
#include <condition_variable>
#include <queue>

using namespace std;
class threadpool
{
public:
	threadpool(int num = 1) :mut(), cv(), que(), isClose(false) {
		for (int i = 0; i < num; i++)
		{
			thread([this]()
				{
					unique_lock<mutex> locker(mut);
					clog<<"[log]:new thread created"<<endl;
					while (true)
					{
						while (!que.empty()&&isClose==false)
						{
							clog<<"[log]:thread is ready to run"<<endl;
							auto task = move(que.front());
							que.pop();
							locker.unlock();
							clog<<"[log]:thread is running"<<endl;
							task();
							locker.lock();
						}
						if (isClose)
						{
							clog<<"[log]:thread return"<<endl;
							return;
						}
						clog<<"[log]:thread start to wait"<<endl;
						cv.wait(locker);
					}
				}
			).detach();
		}
	}
	~threadpool()
	{
		{
			unique_lock<mutex> locker(mut);
			while (!que.empty())
			{
				que.pop();
			}
		}
		isClose = true;
		cv.notify_all();
	}
	template<class F> void add_task(F&& task)
	{
		{
			unique_lock<mutex> locker(mut);
			que.push(task);
		}
		cv.notify_one();
	}
private:
	mutex mut;
	condition_variable cv;
	queue<function<void()>> que;
	bool isClose;
};

