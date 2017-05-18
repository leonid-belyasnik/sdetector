#include "usepool.h"

using namespace CITOOL;


UsePool::UsePool()
{
}

UsePool::~UsePool()
{
}

void UsePool::start_pool()
{
	uint32_t numCPU = std::thread::hardware_concurrency();
	if (numCPU == 0)
		numCPU = 1;

	// use medium optimal
	uint32_t threads = numCPU * 2;

	for (uint32_t i = 0; i < threads; i++)
	{
		_thread_pool.emplace_back(
			[this]
		{
			for (;;)
			{
				std::function<void()> task;
				{
					// lock on task mutex
					std::unique_lock<std::mutex> lock(this->_tasks_mutex);
					// wait until queue is not empty or termination signal is sent
					this->_condition.wait(lock, [this] { return (this->_terminate || !this->_tasks.empty()); });

					// if termination signal received and queue is empty then exit
					if (this->_terminate && this->_tasks.empty()) 
					{
						return;
					}

					// get next task in the queue
					task = this->_tasks.front();
					// remove it from the queue
					this->_tasks.pop();
				}
				// execute the task
				task();
			}
		}
		);
	}
}

void UsePool::terminate()
{
	if (_terminate)
		return;

	{
		std::unique_lock<std::mutex> lock_tasks(_tasks_mutex);
		_terminate = true;
	}

	_condition.notify_all();
}

void UsePool::unterminate()
{
	if (!_terminate)
		return;

	{
		std::unique_lock<std::mutex> lock_tasks(_tasks_mutex);
		_terminate = false;
	}
	
	_condition.notify_all();
}

void UsePool::stop()
{
	if (_stopped)
		return;

	terminate();

	// join all threads
	for (std::thread &thread : _thread_pool)
	{
		if (thread.joinable())
			thread.join();
	}

	// delete pool workers
	_thread_pool.clear();

	// indicate that the pool has been stopped
	_stopped = true;
}

void UsePool::abort()
{
	{
		std::unique_lock<std::mutex> lock_tasks(_tasks_mutex);
		while (!_tasks.empty())
		{
			_tasks.pop();
		}
	}
	stop();
}

void UsePool::enqueue(std::function<void()> f)
{
	{
		// lock on task mutex
		std::unique_lock<std::mutex> lock(_tasks_mutex);
		// push task into queue
		_tasks.push(f);
	}
	// wake up one thread
	_condition.notify_one();
}

bool UsePool::is_terminate() const
{
	return _terminate;
}

bool UsePool::is_running() const
{
	return !_stopped;
}

void UsePool::set_stop(bool value)
{
	_stopped = value;
}

size_t UsePool::count() const
{
	return _tasks.size();
}