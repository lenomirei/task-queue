#include "thread.h"

#include "task.h"
#include "task_queue.h"


Thread::Thread()
  : is_stoped_(true),
  task_queue_(std::make_unique<TaskQueue>()) {
}

Thread::~Thread() {
  // Stop必须是同步的，等待Loop结束后才能结束，以保证task_queue的可用性
  if (!is_stoped_)
    Stop();
  std::move(thread_);
}

void Thread::Run() {
  is_stoped_ = false;
  auto thread_funciton = std::bind(&Thread::Loop, this);
  thread_ = std::move(std::thread(thread_funciton));
}

void Thread::Loop() {
  while (1) {
    if (is_stoped_ && task_queue_->Empty()) {
      // need to clear task queue?
      return;
    }
    Task task = task_queue_->PopTask();
    task.Run();
  }
}

void Thread::Stop() {
  // 不能在子线程中对自身join，不然会触发abort
  Task stop_task(std::bind(&Thread::StopTask, this));
  PostTask(stop_task);
  if (thread_.joinable())
    thread_.join();
}

void Thread::StopTask() {
  is_stoped_ = true;
}

void Thread::PostTask(const Task& task) {
  task_queue_->PushTask(task);
}

