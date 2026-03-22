#include "task_run_loop.h"

#include "task.h"

TaskRunLoop::TaskRunLoop()
    : task_queue_(std::make_unique<std::deque<OnceClosure>>()) {
}

TaskRunLoop::~TaskRunLoop() {
  // Stop必须是同步的，等待Loop结束后才能结束，以保证task_queue的可用性
  StopWithClosure();
  task_queue_ = nullptr;
  thread_ = nullptr;
}

std::thread::id TaskRunLoop::Start() {
  running_ = true;
  if (thread_) {
    StopWithClosure();
  }

  auto thread_funciton = std::bind(&TaskRunLoop::ThreadMain, this);
  thread_ = std::make_unique<std::thread>(thread_funciton);
  current_thread_id_ = thread_->get_id();
  return current_thread_id_;
}

void TaskRunLoop::ThreadMain() {
  BeforeRun();
  Run();
  AfterRun();
}

void TaskRunLoop::BeforeRun() {
  std::unique_lock<std::mutex> lck(thread_lock_);
  // only set it's initial value here
  is_stoped_ = false;
}

void TaskRunLoop::Run() {
  while (1) {
    if (is_stoped_) {
      // need to clear task queue?
      break;
    }

    std::unique_lock<std::mutex> lock(queue_mutex_);
    if (!task_queue_) {
      // should never run here
      break;
    }
    while (task_queue_->empty()) {
      cond_.wait(lock);
    }
    OnceClosure task = std::move(task_queue_->front());
    task_queue_->pop_front();

    lock.unlock();
    task.Run();
  }
}

void TaskRunLoop::AfterRun() {
  // do nothing now
  // std::unique_lock<std::mutex> lck(thread_lock_);
}

void TaskRunLoop::StopWithClosure(bool as_soon_as_possible) {
  // can not join in itself
  OnceClosure stop_task = BindClosure(&TaskRunLoop::StopTask, this);

  PostTask(stop_task, as_soon_as_possible);
  // this function run in another thread join the thread here
  if (thread_ && thread_->joinable())
    thread_->join();
  // after join the thread, running can be set false.
  running_ = false;
}

// This function will only be run in looped child thread
void TaskRunLoop::StopTask() {
  // Post this task to child thread, so this variable will be set in child thread.
  is_stoped_ = true;
}

void TaskRunLoop::PostTask(OnceClosure task, bool as_soon_as_possible) {
  {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    if (task_queue_->empty())
      need_notify_ = true;

    if (task_queue_) {
      if (as_soon_as_possible)
        task_queue_->emplace_front(std::move(task));
      else
        task_queue_->emplace_back(std::move(task));
    }
  }

  if (need_notify_) {
    cond_.notify_one();
    need_notify_ = false;
  }
}

bool TaskRunLoop::IsRunning() {
  // maybe called in another thread
  std::unique_lock<std::mutex> lck(thread_lock_);
  return running_;
}

bool TaskRunLoop::IsInCurrentThread() {
  std::thread::id now_id = std::this_thread::get_id();
  return now_id == current_thread_id_;
}
