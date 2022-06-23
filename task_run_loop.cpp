#include "task_run_loop.h"

#include "task.h"
#include "task_queue.h"


TaskRunLoop::TaskRunLoop()
    : task_queue_(std::make_unique<TaskQueue>())
{
}

TaskRunLoop::~TaskRunLoop()
{
    // Stop必须是同步的，等待Loop结束后才能结束，以保证task_queue的可用性
    StopWithClosure();
    task_queue_ = nullptr;
}

void TaskRunLoop::Start()
{
    running_ = true;
    auto thread_funciton = std::bind(&TaskRunLoop::ThreadMain, this);
    thread_ = std::move(std::thread(thread_funciton));
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
        // 线程本身没有等待，在PopTask的时候，如果队列内没有任务会wait
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
    if (thread_.joinable())
        thread_.join();
    // after join the thread, running can be set false.
    running_ = false;
}

void TaskRunLoop::StopTask()
{
    // Post this task to child thread, so this variable will be set in child thread.
    is_stoped_ = true;
}

void TaskRunLoop::PostTask(const Task& task, bool as_soon_as_possible)
{
    if (as_soon_as_possible)
        task_queue_->PushTask(task, true);
    else
        task_queue_->PushTask(task, false);
}

bool TaskRunLoop::IsRunning()
{
    // maybe called in another thread
    std::unique_lock<std::mutex> lck(thread_lock_);
    return running_;
}

