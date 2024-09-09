#pragma once

#include "Macros.h"

#include <thread>
#include <vector>
#include <queue>
#include <functional>
#include <future>
#include <mutex>
#include <condition_variable>


PSAPI_NAMESPACE_BEGIN

namespace Internal
{

    /// An extremely barebones ThreadPool, it is not encouraged to use this for your project and 
    /// instead use one of the many implementations out there.
    class ThreadPool {
    public:

        /// Constructor: Initialize the ThreadPool with the given number of threads
        // ---------------------------------------------------------------------------------------------------------------------
        // ---------------------------------------------------------------------------------------------------------------------
        inline explicit ThreadPool(size_t numThreads)
        {
            for (size_t i = 0; i < numThreads; ++i) {
                m_Workers.emplace_back([this]() {
                    while (true) {
                        std::function<void()> task;
                        {
                            std::unique_lock<std::mutex> lock(this->m_QueueMutex);
                            this->m_Condition.wait(lock, [this]() { return this->m_Stopped || !this->m_Tasks.empty(); });
                            if (this->m_Stopped && this->m_Tasks.empty()) return;
                            task = std::move(this->m_Tasks.front());
                            this->m_Tasks.pop();
                        }
                        task();
                    }
                    });
            }
        }

        /// Default constructor: uses the hardware concurrency to set the number of threads
        // ---------------------------------------------------------------------------------------------------------------------
        // ---------------------------------------------------------------------------------------------------------------------
        inline ThreadPool() : ThreadPool(std::thread::hardware_concurrency()) {}

        // Destructor: waits for all threads to finish
        // ---------------------------------------------------------------------------------------------------------------------
        // ---------------------------------------------------------------------------------------------------------------------
        inline ~ThreadPool()
        {
            {
                std::unique_lock<std::mutex> lock(m_QueueMutex);
                m_Stopped = true;
            }
            m_Condition.notify_all();
            for (std::thread& worker : m_Workers) worker.join();
        }

        // Enqueue a task onto the ThreadPools worker queue
        // ---------------------------------------------------------------------------------------------------------------------
        // ---------------------------------------------------------------------------------------------------------------------
        template <class F>
        std::future<void> enqueue(F&& f) 
        {
            auto taskPtr = std::make_shared<std::packaged_task<void()>>(std::forward<F>(f));

            std::future<void> res = taskPtr->get_future();
            {
                std::unique_lock<std::mutex> lock(m_QueueMutex);
                if (m_Stopped) {
                    throw std::runtime_error("enqueue on stopped ThreadPool");
                }
                m_Tasks.emplace([taskPtr]() { (*taskPtr)(); });
            }
            m_Condition.notify_one(); // Notify one worker thread that there is a new task

            return res;
        }


    private:
        std::vector<std::thread> m_Workers;
        std::queue<std::function<void()>> m_Tasks;

        // Synchronization primitives
        std::mutex m_QueueMutex;
        std::condition_variable m_Condition;

        // Stop flag to signal thread termination
        bool m_Stopped = false;
    };

}

PSAPI_NAMESPACE_END