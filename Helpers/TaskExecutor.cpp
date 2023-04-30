#include <sstream>
#include "Logger/Logger.h"
#include "TaskExecutor.h"

namespace SA {
    TaskExecutor::TaskExecutor(unsigned int numThreads) : queue(), mutex(), signal() {
        for (unsigned int i = 0; i < numThreads; i++) {
            threads.emplace_back(&TaskExecutor::dispatchTasks, this);
        }
    }

    TaskExecutor::~TaskExecutor() = default;

    void TaskExecutor::run(const Task &task) {
        std::lock_guard<std::mutex> lock(mutex);
        if (isShuttingDownTaskQueue) {
            try {
                task();
            } catch (std::exception &e) {
                Log::GetLog()->error("Error Task run: " + std::string(e.what()));
            }
        } else {
            queue.push(std::make_shared<Task>(task));
            signal.notify_one();
        }
    }

    void TaskExecutor::shutdown() {
        {
            std::lock_guard<std::mutex> lock(mutex);
            if (!isShuttingDownTaskQueue) {
                isShuttingDownTaskQueue = true;
                signal.notify_all();
            }
        }
        for (auto &item: threads) {
            item.join();
        }
    }

    inline std::string getThreadId() {
        std::stringstream ss;
        ss << std::this_thread::get_id();
        return ss.str();
    }

    void TaskExecutor::dispatchTasks() {
        std::string threadId = getThreadId();
        do {
            try {
                std::shared_ptr<Task> task;
                {
                    std::unique_lock<std::mutex> lock(mutex);
                    while (!isShuttingDownTaskQueue && queue.empty()) {
                        signal.wait(lock);
                    }
                    if (queue.empty()) {
                        if (isShuttingDownTaskQueue) break;
                        else continue;
                    }
                    task = queue.front();
                    queue.pop();
                }
                task->operator()();
            } catch (std::exception &e) {
                Log::GetLog()->error(getThreadId() + " Error Task dispatchTasks: " + std::string(e.what()));
            }
        } while (true);
    }
}