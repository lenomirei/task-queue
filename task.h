#pragma once
#include <functional>

using OnceFunc = std::function<void()>;

class Task {
 public:
  Task(OnceFunc func)
      : function_(std::move(func)) {
  }

  ~Task() = default;

  void Run() {
    std::forward<OnceFunc>(function_)();
  }

 private:
  OnceFunc function_;
};
