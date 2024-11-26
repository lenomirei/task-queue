#pragma once

#include <deque>
#include <queue>
#include <vector>
#include <memory>
#include <mutex>
#include <condition_variable>
#include "task.h"

class TaskQueue
{
public:
    TaskQueue();
    ~TaskQueue();

    void PushTask(const Task& task, bool front = false);

    const Task PopTask();

    void PushDelayedTask(Task& task, std::chrono::milliseconds ms);

    bool Empty()
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        return incoming_queue_->empty();
    }

    void MoveReadyDelayedTaskToQueue();
    std::chrono::system_clock::duration GetNextDesiredWakeUp();

private:
    std::mutex queue_mutex_;
    std::unique_ptr<std::deque<Task>> incoming_queue_;
    std::unique_ptr<std::priority_queue<Task>> delayed_queue_;
    bool need_notify_ = false;
};
