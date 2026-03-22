/*
 * @Author: lenomirei lenomirei@163.com
 * @Date: 2026-03-09 16:05:50
 * @LastEditors: lenomirei lenomirei@163.com
 * @LastEditTime: 2026-03-20 14:23:23
 * @FilePath: \task-queue\unittests\task_unittest.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "task_run_loop.h"
#include "task.h"

class TaskTests : public testing::Test {
 protected:
  void SetUp() override {
    // do nothing
    run_loop_ = std::make_unique<TaskRunLoop>();
  }

  void TearDown() override {
    // do nothing
    run_loop_.reset();
  }

  void Method() {
    std::cout << "This is a method!" << std::endl;
  }

  int MethodWithReturnValue(int a, int b) {
    return a + b;
  }

 protected:
  std::unique_ptr<TaskRunLoop> run_loop_ = nullptr;
};

int add(int a, int b) {
  return a + b;
}

void FunWithCallback(OnceClosure callback) {

}

TEST_F(TaskTests, BaseTest) {
  OnceClosure task(BindClosure(&add, 1, 2));

  run_loop_->PostTask(task);
}

TEST_F(TaskTests, MethodTest) {
  

}

TEST_F(TaskTests, MehotdWithReturnValueTest) {


}

TEST_F(TaskTests, LambdaTest) {
  OnceClosure task(BindClosure([]() {
    std::cout << "this is a lambda" << std::endl;
  }));

  run_loop_->PostTask(std::move(task));
}