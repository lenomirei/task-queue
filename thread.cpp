#include "thread.h"

#include "task.h"
#include "task_queue.h"


Thread::Thread()
    : task_queue_(std::make_unique<TaskQueue>())
{
}

Thread::~Thread()
{
    // Stop必须是同步的，等待Loop结束后才能结束，以保证task_queue的可用性
    StopWithClosure();
    if (thread_.joinable())
        thread_.join();
}

void Thread::Start()
{
    running_ = true;
    auto thread_funciton = std::bind(&Thread::ThreadMain, this);
    thread_ = std::move(std::thread(thread_funciton));
}

void Thread::ThreadMain()
{
    BeforeRun();
    Run();
    AfterRun();
}

void Thread::BeforeRun()
{
    std::unique_lock<std::mutex> lck(thread_lock_);
    is_stoped_ = false;
}

void Thread::Run()
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

void Thread::AfterRun()
{
    // do nothing now
    // std::unique_lock<std::mutex> lck(thread_lock_);
}

void Thread::StopWithClosure(bool as_soon_as_possible)
{
    // 不能在子线程中对自身join，不然会触发abort
    Task stop_task(static_cast<std::function<void()>>(std::bind(&Thread::StopTask, this)));

    PostTask(stop_task, as_soon_as_possible);
    // this function run in another thread join the thread here
    if (thread_.joinable())
        thread_.join();
    // after join the thread, running can be set false.
    running_ = false;
}

void Thread::StopTask()
{
    // Post this task to child thread, so this variable will be set in child thread.
    is_stoped_ = true;
}

void Thread::PostTask(const Task& task, bool as_soon_as_possible)
{
    if (as_soon_as_possible)
        task_queue_->PushTask(task, true);
    else
        task_queue_->PushTask(task, false);
}

bool Thread::IsRunning()
{
    // maybe called in another thread
    std::unique_lock<std::mutex> lck(thread_lock_);
    return running_;
}

