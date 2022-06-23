#include "task_queue.h"

TaskQueue::TaskQueue()
{
    queue_.reset(new std::deque<Task>());
}

TaskQueue::~TaskQueue()
{

}

void TaskQueue::PushTask(const Task& task, bool front)
{
    std::lock_guard<std::mutex> lock(queue_mutex_);
    if (queue_->empty())
        need_notify_ = true;

    if (front)
        queue_->emplace_front(task);
    else
        queue_->emplace_back(task);

    if (need_notify_)
    {
        cond_.notify_one();
        need_notify_ = false;
    }
}

const Task TaskQueue::PopTask()
{
    // multi thread attention!
    std::unique_lock<std::mutex> lck(queue_mutex_);
    while (queue_->empty())
        cond_.wait(lck);
    Task task = queue_->front();
    queue_->pop_front();
    return std::move(task);
}

