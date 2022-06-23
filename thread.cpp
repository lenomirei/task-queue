#include "thread.h"

#include "task.h"
#include "task_queue.h"


Thread::Thread()
    : is_stoped_(true),
    task_queue_(std::make_unique<TaskQueue>())
{
}

Thread::~Thread()
{
    // Stop必须是同步的，等待Loop结束后才能结束，以保证task_queue的可用性
    StopWithClosure();
    if (thread_.joinable())
        thread_.join();
}

void Thread::Run()
{
    is_stoped_ = false;
    auto thread_funciton = std::bind(&Thread::Loop, this);
    thread_ = std::move(std::thread(thread_funciton));
}

void Thread::Loop()
{
    while (1)
    {
        if (is_stoped_)
        {
            // need to clear task queue?
            return;
        }
        // 线程本身没有等待，在PopTask的时候，如果队列内没有任务会wait
        Task task = task_queue_->PopTask();
        task.Run();
    }
}

void Thread::StopWithClosure(bool as_soon_as_possible)
{
    // 不能在子线程中对自身join，不然会触发abort
    Task stop_task(std::bind(&Thread::StopTask, this));

    PostTask(stop_task, as_soon_as_possible);
}

void Thread::StopTask()
{
    is_stoped_ = true;
}

void Thread::PostTask(const Task& task, bool as_soon_as_possible)
{
    if (as_soon_as_possible)
        task_queue_->PushTask(task, true);
    else
        task_queue_->PushTask(task, false);
}

