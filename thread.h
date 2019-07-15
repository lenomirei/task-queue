#pragma once

#include <thread>
#include <mutex>



class TaskQueue;
class Task;

class Thread {
public:
  Thread();

  ~Thread();

  void Run();

  void Stop();

  void PostTask(const Task& task);

protected:

  void StopTask();

  void Loop();

private:

  bool is_stoped_;

  std::thread thread_;

  std::unique_ptr<TaskQueue> task_queue_;

  std::mutex task_queue_lock_;

  std::mutex thread_lock_;

};

