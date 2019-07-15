#pragma once
#include "task.h"
#include <chrono>


struct PendingTask {
  PendingTask(const std::function<void()>& func,
              std::chrono::time_point<std::chrono::system_clock> delayed_run_time) 
    : task(std::move(func)),
      delayed_run_time(delayed_run_time){

  }
  ~PendingTask() = default;

  Task task;
  std::chrono::time_point<std::chrono::system_clock> delayed_run_time;
};