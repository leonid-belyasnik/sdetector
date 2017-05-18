/**
* \file		usepool.h
* \brief	Implement concept of thread pool.
*
* \author	Leonid Belyasnik (leonid.belyasnik@gmail.com)
* \date		08/05/2017
*/

#ifndef USEPOOL_H
#define USEPOOL_H

#include <queue>
#include <vector>
#include <atomic>
#include <condition_variable>
#include <thread>
#include <future>
#include <functional>

namespace CITOOL {

	class UsePool
	{
		std::vector<std::thread> _thread_pool;		///< Thread pool storage
		std::queue<std::function<void()>> _tasks;	///< Queue to keep track of incoming tasks
		std::mutex _tasks_mutex;					///< Task queue mutex
		std::condition_variable _condition;			///< Condition variable on task
		std::atomic_bool _terminate{ false };		///< Indicates that pool needs to be shut down
		std::atomic_bool _stopped{ true };			///< Indicates that pool has been terminated
	protected:
		void enqueue(std::function<void()> f);		///< Adds task to a task queue
		void start_pool();
	public:
		UsePool();
		virtual ~UsePool();
		virtual void start() = 0;

		bool is_terminate() const;
		bool is_running() const;
		size_t count() const;
		void terminate();
		void unterminate();
		void stop();
		void abort();
		void set_stop(bool value);	
	};

} // CITOOL

#endif // USEPOOL_H