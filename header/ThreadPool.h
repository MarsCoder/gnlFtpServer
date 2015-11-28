/*
 * ThreadPool.h
 *
 *  Created on: Apr 19, 2015
 *      Author: mars
 */

#ifndef THREADPOOL_H_
#define THREADPOOL_H_

#include <functional>
#include <vector>
#include <thread>
#include <memory>
#include <deque>
#include <mutex>
#include <condition_variable>

namespace gnl {

class ThreadPool
{
public:
	typedef std::function<void ()> Task;
	//typedef std::vector<std::shared_ptr<std::thread>>	SPVThread;
	ThreadPool(unsigned int Qsize, unsigned int Tsize);
	ThreadPool(ThreadPool& ) = delete;				//禁止拷贝构造
	virtual ~ThreadPool();
	void addTask(const Task& task);					//添加任务
	void start();									//创建并线程，开始执行任务队列中的任务
	void stop();									//立刻退出所有线程
	void finish();									//等待完成任务缓冲区内全部任务后退出所有线程
	ThreadPool& operator=(ThreadPool& ) = delete;	//禁止赋直/拷贝

private:
	void runInThread();
	Task takeTask();

private:
	std::vector<std::shared_ptr<std::thread>> threads;	//线程池
	std::deque<Task> taskQueue;			//任务阻塞队列
	std::mutex mu;
	std::condition_variable	empty;		//任务队列空条件变量
	std::condition_variable full;		//任务队列满条件变量
	unsigned int		threadNumber;			//线程池线程个数
	unsigned int		maxQueueSize;			//任务队列最大任务个数
	bool		running;				//线程运行状态

};

} /* namespace gnl */

#endif /* THREADPOOL_H_ */
