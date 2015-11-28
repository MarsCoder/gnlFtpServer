/*
 * ThreadPool.cpp
 *
 *  Created on: Apr 19, 2015
 *      Author: mars
 */

#include <assert.h>
#include <unistd.h>


#include "../header/ThreadPool.h"

namespace gnl {

//第一个参数位线程池中线程的个数，第二个参数位线程池中任务队列缓冲区的的大小（默认0为无限大）
ThreadPool::ThreadPool(unsigned int Tsize, unsigned int Qsize)
	:threadNumber(Tsize),maxQueueSize(Qsize),running(false) {
	// TODO Auto-generated constructor stub

}

ThreadPool::~ThreadPool() {
	// TODO Auto-generated destructor stub
	if(running){
		stop();
	}
}

/***************************************************
 * 			线程入口函数
 ***************************************************/
void ThreadPool::runInThread()
{
	while(running){
		Task task(takeTask());		//从缓冲队列中取出一个任务
		if(task){
			task();						//执行任务
		}
	}
}

/***************************************************
 * 		从任务缓冲对列中取出一个任务
 ***************************************************/
ThreadPool::Task ThreadPool::takeTask()
{
	Task	task;
	std::unique_lock<std::mutex> lock(mu);
	while(taskQueue.empty() && running){
		empty.wait(lock);
	}
	if(!taskQueue.empty()){
		task = taskQueue.front();	//取出一个任务
		taskQueue.pop_front();		//将已取出的任务重队列中删除
		full.notify_one();			//唤醒一个等待添加任务到缓冲队列的阻塞进程
	}
	return task;				//返回取出的任务
}

/***************************************************
 * 		 	添加任务到任务缓冲队列中
 ***************************************************/
void ThreadPool::addTask(const Task& task)
{
	if(threads.empty()){
		task();		//如果线程池中没有线程，则直接执行任务
	}
	else{
		std::unique_lock<std::mutex> lock(mu);
		while(taskQueue.size() >= maxQueueSize){
			full.wait(lock);	//任务缓冲队列满，则进入阻塞状态
		}
		taskQueue.push_back(task);	//将任务放进任务缓冲队列
		empty.notify_one();			//唤醒一个空闲线程执行任务
	}
}

/***************************************************
 * 		 	创建并线程，开始执行任务队列中的任务
 ***************************************************/
void ThreadPool::start()
{
	assert(threads.empty());
	running = true;
	threads.reserve(threadNumber);	//一次性分配线程池内存空间
	for(unsigned short i=0; i<threadNumber; ++i){
		//创建新的线程并放进进程池
		threads.push_back(std::shared_ptr<std::thread>
			(new std::thread(&ThreadPool::runInThread, this)));
	}
}

/***************************************************
 * 		 	立刻停止所有线程
 ***************************************************/
void ThreadPool::stop()
{
	{
		std::unique_lock<std::mutex> lock(mu);
		running = false;
		empty.notify_all();		//唤醒所有线程
	}
	//等待每一个线程退出
	for(std::vector<std::shared_ptr<std::thread>>::iterator i = threads.begin();
						i != threads.end(); ++i)
	{
		(*i)->join();
	}
}

/***************************************************
 * 		 	等待完成任务缓冲区内全部任务后退出所有线程
 ***************************************************/
void ThreadPool::finish()
{
	while(!taskQueue.empty()){
		//缓冲队列还有任务，则等待执行完成
		usleep(100);	//休眠100毫秒
	}
	{
		std::unique_lock<std::mutex> lock(mu);
		running = false;
		empty.notify_all();		//唤醒所有线程
	}
	//等待每一个线程退出
	for(std::vector<std::shared_ptr<std::thread>>::iterator i = threads.begin();
					i != threads.end(); ++i)
	{
		(*i)->join();
	}

}

} /* namespace gnl */
