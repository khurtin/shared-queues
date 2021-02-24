#ifndef SHARED_QUEUES_PROCESSOR_H
#define SHARED_QUEUES_PROCESSOR_H

#include <memory>
#include <map>
#include <functional>
#include <shared_mutex>
#include "callable.h"
#include "thread_pool.h"

namespace shared_queues
{

template<typename KEY, typename VALUE>
class processor
{
public:
    template<typename T, typename FUNCTION>
    bool subscribe(const KEY& key, std::shared_ptr<T> object, FUNCTION func)
    {
        if (!object)
        {
            return false;
        }

        auto ptr = std::make_shared<callable<T, KEY, VALUE>>(std::move(object), std::move(std::function<void(T*, const KEY&, const VALUE&)>(func)));
        if (!ptr)
        {
            return false;
        }

        std::lock_guard<std::shared_mutex> guard(m_consumers_guard);
        auto iter = m_consumers.find(key);
        if (iter != m_consumers.end())
        {
            if (!(*iter->second))
            {
                iter->second = std::move(ptr);
                return true;
            }

            return false;
        }

        m_consumers.emplace(key, std::move(ptr));
        return true;
    }

    void unsubscribe(const KEY& key)
    {
        std::lock_guard<std::shared_mutex> guard(m_consumers_guard);
        m_consumers.erase(key); 
    }

    void push(const KEY& key, const VALUE& value)
    {
        std::shared_lock<std::shared_mutex> guard(m_consumers_guard);
        std::shared_ptr<callable_base<KEY, VALUE>> ptr;
        auto iter = m_consumers.find(key);
        if (iter != m_consumers.end())
        {
            ptr = iter->second;
        }
        else
        {
            ptr = std::make_shared<callable_fake<KEY, VALUE>>();
        }

        m_thread_pool.submit(std::move(ptr), key, value);
    }

private:
    std::shared_mutex m_consumers_guard;
    std::map<KEY, std::shared_ptr<callable_base<KEY, VALUE>>> m_consumers;
    thread_pool<KEY, VALUE> m_thread_pool;

}; // class processor

} // namespace shared_queues

#endif // SHARED_QUEUES_PROCESSOR_H
