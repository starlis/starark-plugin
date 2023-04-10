#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>

// A threadsafe-queue.
/**
 * @author https://stackoverflow.com/a/16075550
 * @license CC-BY-SA 4.0
 * @tparam T
 */
template <class T>
class SafeQueue
{
public:
    SafeQueue()
            : q()
            , m()
            , c()
    {}

    ~SafeQueue()
    = default;

    // Add an element to the queue.
    void enqueue(T t)
    {
        std::lock_guard<std::mutex> lock(m);
        q.push(t);
        c.notify_one();
    }

    // Get the "front"-element.
    // If the queue is empty, wait till a element is avaiable.
    T dequeue(void)
    {
        std::unique_lock<std::mutex> lock(m);
        while(!_shutdown && q.empty())
        {
            // release lock as long as the wait and reaquire it afterwards.
            c.wait(lock);
        }
        T val = q.front();
        q.pop();
        return val;
    }

    void shutdown() {
        _shutdown = true;
        c.notify_one();
    }

private:
    volatile bool _shutdown = false;
    std::queue<T> q;
    mutable std::mutex m;
    std::condition_variable c;
};

