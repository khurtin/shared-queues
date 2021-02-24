#ifndef SHARED_QUEUES_THREAD_SAFE_QUEUE_H
#define SHARED_QUEUES_THREAD_SAFE_QUEUE_H

#include <queue>
#include <mutex>

namespace shared_queues
{

template<typename TYPE>
class thread_safe_queue
{
public:
    void push(TYPE value)
    {
        std::lock_guard<std::mutex> guard(m_guard);
        m_queue.push(std::move(value));
    }

    bool try_pop(TYPE& value)
    {
        std::lock_guard<std::mutex> guard(m_guard);
        if (m_queue.empty())
            return false;
        value = std::move(m_queue.front());
        m_queue.pop();
        return true;
    }

private:
    std::mutex m_guard;
    std::queue<TYPE> m_queue;
}; // class thread_safe_queue

} // namespace shared_queues

#endif // SHARED_QUEUES_THREAD_SAFE_QUEUE_H
