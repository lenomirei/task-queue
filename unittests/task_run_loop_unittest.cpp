#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "task_run_loop.h"
#include "task.h"

namespace unit_tests {

class TaskRunLoopUnitTests : public testing::Test {
 protected:
  void SetUp() override {
    // do nothing
  }

  void TearDown() override {
    // do nothing
  }
};

TEST_F(TaskRunLoopUnitTests, DeconstructTest) {
  std::unique_ptr<TaskRunLoop> run_loop = std::make_unique<TaskRunLoop>();
  run_loop->Start();

  // should exit normally
}

// TEST_F(TaskRunLoopUnitTests, BaseTest) {
//   std::unique_ptr<TaskRunLoop> run_loop = std::make_unique<TaskRunLoop>();
//   run_loop->Start();

//   // should exit normally
// }

// TEST_F(TaskRunLoopUnitTests, LambdaTest) {
//   std::unique_ptr<TaskRunLoop> run_loop = std::make_unique<TaskRunLoop>();
//   run_loop->Start();
// }



}  // namespace unit_tests