#pragma once

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

// 异步日志队列
template<typename T>
class LogQueue
{
public:
    void Push(const T &data)
    {
        std::unique_lock<std::mutex> lock(m_mtx);
        m_queue.push(data);
        m_convariable.notify_one();
    }

    T Pop()
    {
        while (m_queue.empty())
        {
            std::unique_lock<std::mutex> lock(m_mtx);
            // 日志队列为空，线程进入wait状态
            m_convariable.wait(lock);

        }

        T data = m_queue.front();
        m_queue.pop();

        return data;
    }
private:
    std::queue<T> m_queue;
    std::mutex m_mtx;
    std::condition_variable m_convariable;
};