#pragma once
#include <functional>


namespace stream_hub {

  class Task {
  public:

    Task(const std::function<void()>& func) : function_(std::move(func)) {
    }

    ~Task() {

    }

    void Run() {
      function_();
    }

  private:

    std::function<void()> function_;
  };

} // namespace stream_hub



