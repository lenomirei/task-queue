#include "task_queue.h"

TaskQueue::TaskQueue()
{
    incoming_queue_.reset(new std::deque<Task>());
    delayed_queue_.reset(new std::priority_queue<Task>());
}

TaskQueue::~TaskQueue() = default;

void TaskQueue::PushTask(const Task& task, bool front)
{
    std::lock_guard<std::mutex> lock(queue_mutex_);
    if (incoming_queue_->empty())
        need_notify_ = true;

    if (front)
        incoming_queue_->emplace_front(task);
    else
        incoming_queue_->emplace_back(task);
}

const Task TaskQueue::PopTask()
{
    // multi thread attention!
    std::unique_lock<std::mutex> lck(queue_mutex_);
    Task task = incoming_queue_->front();
    incoming_queue_->pop_front();
    return std::move(task);
}

void TaskQueue::PushDelayedTask(Task& task, std::chrono::milliseconds ms)
{
    std::unique_lock<std::mutex> lck(queue_mutex_);
    task.SetDelay(ms);
    delayed_queue_->push(task);

    // need to move ready task after push
    MoveReadyDelayedTaskToQueue();
}

std::chrono::system_clock::duration TaskQueue::GetNextDesiredWakeUp()
{
    const Task& earliesr_task = delayed_queue_->top();
    return earliesr_task.DelayedRunTime() - std::chrono::system_clock::now();
}

void TaskQueue::MoveReadyDelayedTaskToQueue()
{
    while (!delayed_queue_->empty())
    {
        const Task& task = delayed_queue_->top();
        if (task.DelayedRunTime() > std::chrono::system_clock::now())
        {
            break;
        }


        const Task& ready_task = delayed_queue_->top();

        incoming_queue_->emplace_back(ready_task);

        delayed_queue_->pop();
    }
}