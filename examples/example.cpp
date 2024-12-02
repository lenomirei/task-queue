/*
 * @Author: lenomirei lenomirei@163.com
 * @Date: 2023-04-25 11:59:12
 * @FilePath: \task-queue\examples\example.cpp
 * @Description: 
 * 
 */
#include <iostream>

#include <functional>

#include "../task.h"
#include "../task_run_loop.h"

void TaskFunction() {
    std::cout << "This is a task" << std::endl;
}

int add(int a, int b) {
    return a + b;
}

int main()
{
    std::unique_ptr<TaskRunLoop> run_loop = std::make_unique<TaskRunLoop>();
    run_loop->Start();

     OnceClosure task(BindClosure(&TaskFunction));

     run_loop->PostTask(task);

    std::this_thread::sleep_for(std::chrono::seconds(5));

    run_loop->StopWithClosure();


    Task<int()> add_func = BindOnce(&add, 1, 2);
    int teaaast = add_func.Run();

    run_loop = nullptr;


    return 0;
}