#pragma once
#include <functional>

class Task
{
public:

    Task(std::function<void()> func) : function_(std::move(func))
    {
    }

    ~Task() = default;

    void Run()
    {
        function_();
    }

private:
    std::function<void()> function_;
};
