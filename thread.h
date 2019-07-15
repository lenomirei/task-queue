#pragma once

#include <thread>
#include <mutex>
#include <chrono>



class TaskQueue;
class Task;

class Thread {
public:
  Thread();

  ~Thread();

  void Run();

  void Stop();

  void PostTask(const Task& task);

  void PostDelayTask(const Task& task, const std::chrono::duration<int>& delta_time);

protected:

  void StopTask();

  void Loop();

  void DoDelayedWork(const std::chrono::time_point<std::chrono::system_clock>& next_delayed_work_time);

  void CalculateDelayRuntime(std::chrono::duration<int> delay);

private:

  bool is_stoped_;

  std::thread thread_;

  std::unique_ptr<TaskQueue> task_queue_;

  std::unique_ptr<TaskQueue> delayed_task_queue_;

  std::mutex task_queue_lock_;

  std::mutex thread_lock_;

  std::chrono::time_point<std::chrono::system_clock> delayed_work_time_;

  std::chrono::time_point<std::chrono::system_clock> recent_time_;

};

