#pragma once

#include <queue>
#include <memory>
#include <mutex>
#include <condition_variable>
#include "task.h"

class TaskQueue {
public:
  TaskQueue();
  ~TaskQueue();

  void PushTask(const Task& task);

  const Task PopTask();

  bool Empty() {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    return queue_->empty();
  }



private:

  std::mutex queue_mutex_;
  std::unique_ptr<std::queue<Task>> queue_;
  bool need_notify_ = false;
  std::condition_variable cond_;
};
