#pragma once

#include <thread>
#include <mutex>



class TaskQueue;
class Task;

class Thread
{
public:
    Thread();

    ~Thread();

    void Start();

    void StopWithClosure(bool as_soon_as_possible = false);

    void PostTask(const Task& task, bool as_soon_as_possible = false);
    bool IsRunning();

protected:
    void StopTask();
    void ThreadMain();

    // Run series function will be called in child thread
    void BeforeRun();
    void Run();
    void AfterRun();

private:
    // used for multithread, can not use this varible in loop!
    bool running_ = false;
    // only used for child thread loop, do not access this in other thread!
    bool is_stoped_ = true;
    std::thread thread_;
    std::unique_ptr<TaskQueue> task_queue_;

    std::mutex task_queue_lock_;

    std::mutex thread_lock_;

};

