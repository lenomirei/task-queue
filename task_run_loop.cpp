#include "task_run_loop.h"

#include "task.h"
#include "task_queue.h"

TaskRunLoop::TaskRunLoop()
    : task_queue_(std::make_unique<TaskQueue>())
{
}

TaskRunLoop::~TaskRunLoop()
{
    StopWithClosure(true);
    task_queue_ = nullptr;
    thread_ = nullptr;
}

void TaskRunLoop::Start()
{
    running_ = true;
    if (thread_)
    {
        StopWithClosure();
    }

    auto thread_funciton = std::bind(&TaskRunLoop::ThreadMain, this);
    thread_ = std::make_unique<std::thread>(thread_funciton);
}

void TaskRunLoop::ThreadMain()
{
    BeforeRun();
    Run();
    AfterRun();
}

void TaskRunLoop::BeforeRun()
{
    std::unique_lock<std::mutex> lck(thread_lock_);
    is_stoped_ = false;
}

void TaskRunLoop::Run()
{
    while (1)
    {
        if (is_stoped_)
        {
            // need to clear task queue?
            break;
        }
        // Thread handle wait. Task queue don't handle wait anymore.
        if (!task_queue_)
        {
            // should never run here
            break;
        }
        task_queue_->MoveReadyDelayedTaskToQueue();

        if (task_queue_->Empty())
        {
            std::chrono::system_clock::duration sleep_time = task_queue_->GetNextDesiredWakeUp();
            std::unique_lock<std::mutex> lck(thread_lock_);
            cond_.wait_for(lck, sleep_time);
            continue;
        }

        Task task = task_queue_->PopTask();

        task.Run();
    }
}

void TaskRunLoop::AfterRun()
{
    // do nothing now
    // std::unique_lock<std::mutex> lck(thread_lock_);
}

void TaskRunLoop::StopWithClosure(bool as_soon_as_possible)
{
    // 不能在子线程中对自身join，不然会触发abort
    Task stop_task(static_cast<std::function<void()>>(std::bind(&TaskRunLoop::StopTask, this)));

    PostTask(stop_task, as_soon_as_possible);
    // this function run in another thread join the thread here
    if (thread_ && thread_->joinable())
    {
        thread_->join();
    }
    // after join the thread, running can be set false.
    running_ = false;
}

void TaskRunLoop::StopTask()
{
    // Post this task to child thread, so this variable will be set in child thread.
    is_stoped_ = true;
}

void TaskRunLoop::PostTask(Task task, bool as_soon_as_possible)
{
    if (task_queue_ && IsRunning())
    {
        task_queue_->PushTask(task, as_soon_as_possible);
    }
}

void TaskRunLoop::PushDelayedTask(Task task, size_t ms)
{
    if (task_queue_ && IsRunning())
    {
        task_queue_->PushDelayedTask(task, std::chrono::milliseconds(ms));
    }
}

bool TaskRunLoop::IsRunning()
{
    // maybe called in another thread
    std::unique_lock<std::mutex> lck(thread_lock_);
    return running_;
}

