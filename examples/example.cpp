#include <iostream>

#include <functional>

#include "../task.h"
#include "../task_run_loop.h"

void TaskFunction() {
    std::cout << "This is a task" << std::endl;
}

int main()
{
    std::unique_ptr<TaskRunLoop> run_loop = std::make_unique<TaskRunLoop>();
    run_loop->Start();

    Task task(std::bind(&TaskFunction));

    run_loop->PostTask(task);

    std::this_thread::sleep_for(std::chrono::seconds(5));

    run_loop->StopWithClosure();

    run_loop = nullptr;


    return 0;
}