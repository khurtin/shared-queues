#ifndef SHARED_QUEUES_THREAD_POOL_H
#define SHARED_QUEUES_THREAD_POOL_H

#include <vector>
#include <atomic>
#include <thread>
#include <memory>
#include <tuple>
#include "callable.h"
#include "thread_safe_queue.h"

namespace shared_queues
{

template<typename KEY, typename VALUE>
class thread_pool
{
public:
    thread_pool() : m_keep_alive(true)
    {
         unsigned int thread_number = std::thread::hardware_concurrency();
         thread_number = thread_number == 0 ? 2 : thread_number;
         try
         {
            for (unsigned int i = 0; i < thread_number; i++)
            {
                m_threads.emplace_back(std::thread(&thread_pool::worker_thread, this));
            }
         }
         catch(...)
         {
            m_keep_alive = false;
            throw;
         }
    }

    ~thread_pool()
    {
        m_keep_alive = false;
        for (unsigned int i = 0; i < m_threads.size(); i++)
        {
            if (m_threads[i].joinable())
            {
                m_threads[i].join();
            }
        } 
    }

    void submit(std::shared_ptr<callable_base<KEY, VALUE>> callable_object, KEY key, VALUE value)
    {
        m_queue.push(std::move(std::make_tuple<std::weak_ptr<callable_base<KEY, VALUE>>, KEY, VALUE>(callable_object, std::move(key), std::move(value))));
    }

private:
    void worker_thread()
    {
        while (m_keep_alive)
        {
            std::tuple<std::weak_ptr<callable_base<KEY, VALUE>>, KEY, VALUE> tuple;
            std::shared_ptr<callable_base<KEY, VALUE>> callable_object;
            if (m_queue.try_pop(tuple))
            {
                callable_object = std::get<0>(tuple).lock();
            }

            if (callable_object)
            {
                (*callable_object)(std::get<1>(tuple), std::get<2>(tuple));
            }
            else
            {
                std::this_thread::yield();
            }
        }
    }

private:
    std::atomic_bool m_keep_alive;
    thread_safe_queue<std::tuple<std::weak_ptr<callable_base<KEY, VALUE>>, KEY, VALUE>> m_queue;
    std::vector<std::thread> m_threads;

}; // class thread_pool

} // namespace shared_queues

#endif // SHARED_QUEUES_THREAD_POOL_H
