#pragma once

#include <queue>
#include <vector>
#include <functional>

#include <thread>
#include <future>
#include <mutex>
#include <condition_variable>
#include <atomic>

namespace Core
{
	class ThreadPool
	{
	private:
		std::vector<std::thread> threads;
		std::queue<std::function<void()>> jobs;
		std::mutex job_mutex;
		std::condition_variable cv;
		std::atomic<bool> is_terminated = false;

	private:
		void ThreadLoop()
		{
			std::function<void()> job;

			while (!is_terminated)
			{
				{
					std::unique_lock<std::mutex> job_lock(job_mutex);

					cv.wait(job_lock, [this] {
						return !jobs.empty() || is_terminated;
						});

					if (is_terminated)
					{
						return;
					}

					job = std::move(jobs.front());
					jobs.pop();
				}
				job();
			}
		}

		void PoolSetup()
		{
			threads.clear();

			for (unsigned i = 0; i < std::thread::hardware_concurrency(); ++i)
			{
				threads.emplace_back(std::thread(&ThreadPool::ThreadLoop, this));
			}
		}

	public:
		ThreadPool()
		{
			PoolSetup();
		}

		~ThreadPool()
		{
			for (auto& thread : threads)
			{
				if (thread.joinable())
				{
					thread.join();
				}
			}
		}

	public:
		void Join()
		{
			for (auto& thread : threads)
			{
				if (thread.joinable())
				{
					thread.join();
				}
			}
		}

		template<typename F, typename... Args>
		auto QueueJob(F&& f, Args&&... args) -> std::future<decltype(f(args...))>
		{
			//TODO(): use c++23 by adding move_only_function

			auto func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
			auto task_ptr = std::make_shared<std::packaged_task<decltype(f(args...))()>>(func);
			auto wrapper = [task_ptr]() {(*task_ptr)(); };

			{
				std::lock_guard<std::mutex> job_lock(job_mutex);
				jobs.push(wrapper);
				cv.notify_one();
			}

			return task_ptr->get_future();
		}

		template<typename F>
		void QueueJob(F job)
		{
			if (is_terminated)
				return;
			{
				std::unique_lock<std::mutex> jobs_lock(job_mutex);
				jobs.push(std::function<void()>(job));
			}
			cv.notify_one();
		}

		template<typename F, typename... Args>
		void QueueJob(const F& job, const Args&... args) { QueueJob([job, args...]{ job(args...); }); }

		void DropJobs()
		{
			is_terminated = true;
			Join();
			is_terminated = false;
			std::queue<std::function<void()>> empty;
			std::swap(jobs, empty);
			PoolSetup();
		}
	};

	namespace Instance
	{
		inline ThreadPool thread_pool;
	}
}