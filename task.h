#pragma once
#include <functional>
#include <chrono>

//template<typename >
//class TaskBase
//{
//public:
//    
//}

template<typename Func>
class Task
{
public:

    Task(Func&& func) : function_(std::move(func))
    {
    }

    ~Task() = default;

    template<typename... ArgTypes>
    auto operator()(ArgTypes... args) -> typename std::result_of(Func(ArgTypes...))::type
    {
        return function_(args...);
    }

    void SetDelay(std::chrono::milliseconds ms)
    {
        delayed_time_point_ = std::chrono::system_clock::now() + ms;
    }

    bool operator< (const Task& task) const
    {
        return this->delayed_time_point_ < task.delayed_time_point_;
    }

    std::chrono::system_clock::time_point DelayedRunTime() const
    {
        return delayed_time_point_;
    }

private:
    std::chrono::system_clock::time_point delayed_time_point_;
    Func function_;
};
