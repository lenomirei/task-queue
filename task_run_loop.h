#pragma once

#include <condition_variable>
#include <deque>
#include <mutex>
#include <thread>

class Task;

class TaskRunLoop {
 public:
  TaskRunLoop();

  ~TaskRunLoop();

  void Start();

  void StopWithClosure(bool as_soon_as_possible = false);

  void PostTask(Task task, bool as_soon_as_possible = false);
  bool IsRunning();

 protected:
  void StopTask();
  void ThreadMain();

  // Run series function will be called in child thread
  void BeforeRun();
  void Run();
  void AfterRun();

 private:
  // used for another thread, can not use this varible in loop!
  bool running_ = false;
  // only used for child thread loop, do not access this in other thread!
  bool is_stoped_ = true;
  std::unique_ptr<std::thread> thread_ = nullptr;
  std::unique_ptr<std::deque<Task>> task_queue_ = nullptr;

  std::mutex thread_lock_;
  std::mutex queue_mutex_;
  std::condition_variable cond_;
  bool need_notify_ = false;
};
