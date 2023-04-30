#pragma once
#include <thread>
#include <windows.h>
#include <functional>
#include <mutex>
#include <queue>


namespace SA {
    typedef std::function<void()> Task;
    class TaskExecutor {
    private:
        volatile bool isShuttingDownTaskQueue = false;
        std::queue<std::shared_ptr<Task>> queue;
        mutable std::mutex mutex;
        std::condition_variable signal;
        std::vector<std::thread> threads;

        void dispatchTasks();

    public:
        explicit TaskExecutor(unsigned int numThreads = std::thread::hardware_concurrency());

        ~TaskExecutor();

        void run(const Task &task);

        void shutdown();
    };
}