#include "task_queue.h"

namespace stream_hub {

  TaskQueue::TaskQueue() {
    queue_.reset(new std::queue<Task>());
  }

  TaskQueue::~TaskQueue() {
    queue_.reset();
  }

  void TaskQueue::PushTask(const Task& task) {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    if (queue_->empty())
      need_notify_ = true;
    queue_->push(task);

    if (need_notify_) {
      cond_.notify_one();
      need_notify_ = false;
    }
  }

  const Task TaskQueue::PopTask() {
    // multi thread attention!
    std::unique_lock<std::mutex> lck(queue_mutex_);
    while (queue_->empty())
      cond_.wait(lck);

    Task task = queue_->front();
    queue_->pop();
    return std::move(task);
  }

} // namespace stream_hub

