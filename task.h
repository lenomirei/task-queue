/*
 * @Author: lenomirei lenomirei@163.com
 * @Date: 2023-03-31 14:55:59
 * @FilePath: \task-queue\task.h
 * @Description:
 *
 */
#pragma once

#include <functional>
#include <tuple>

template <typename Signature>
class Task;

// OnceClosure is a task type which do not need return valud for task run loop
using OnceClosure = Task<void()>;

template <typename T>
using PassingType = std::conditional_t<std::is_scalar_v<T>, T, T&&>;

class TaskStateBase {
 public:
  using InvokeFuncStorage = void (*)();
  TaskStateBase(InvokeFuncStorage invoke_func) : invoke_func_(invoke_func) {}
  InvokeFuncStorage invoke_func_;
};

template <typename Functor, typename... BoundArgs>
struct TaskState final : public TaskStateBase {
 public:
  using BoundArgsTuple = std::tuple<BoundArgs...>;
  template <typename ForwardFunctor, typename... ForwardBoundArgs>
  TaskState(TaskStateBase::InvokeFuncStorage invoke_func_storage, ForwardFunctor&& functor, ForwardBoundArgs&&... bound_args) : TaskStateBase(invoke_func_storage) {
    func_ = functor;
    bound_args_ = BoundArgsTuple(bound_args...);
  }
  template <typename ForwardFunctor, typename... ForwardBoundArgs>
  static std::shared_ptr<TaskState> Create(TaskStateBase::InvokeFuncStorage invoke_func_storage, ForwardFunctor&& func, ForwardBoundArgs&&... bound_args) {
    return std::make_shared<TaskState<Functor, BoundArgs...>>(invoke_func_storage, std::forward<ForwardFunctor>(func), std::forward<ForwardBoundArgs>(bound_args)...);
  }
  BoundArgsTuple bound_args_;
  Functor func_;
};

template <typename Traits, typename StorageType, typename UnboundRunType>
struct Invoker;

template <typename Traits, typename R, typename StorageType, typename... UnboundArgs>
struct Invoker<Traits, StorageType, R(UnboundArgs...)> {
    using Indices = std::make_index_sequence<
        std::tuple_size_v<decltype(StorageType::bound_args_)>>;
  static R RunOnce(std::shared_ptr<TaskStateBase> base, UnboundArgs&&... unbound_args) {
    std::shared_ptr<StorageType> storage = std::static_pointer_cast<StorageType>(base);
    
    return RunImpl(std::move(storage->func_), std::move(storage->bound_args_), Indices(), std::forward<UnboundArgs>(unbound_args)...);
  }
  template <typename Functor, typename BoundArgsTuple, size_t... Indices>
  static R RunImpl(Functor&& functor, BoundArgsTuple&& bound, std::index_sequence<Indices...>, UnboundArgs&&... unbound_args) {  
    return Traits::Invoke(std::forward<Functor>(functor), std::get<Indices>(std::forward<BoundArgsTuple>(bound))..., std::forward<UnboundArgs>(unbound_args)...);
  }
};

template <typename R, typename... Args>
class Task<R(Args...)> {
 public:
  using ResultType = R;
  using RunType = R(Args...);
  using Func = R (*)(std::shared_ptr<TaskStateBase>, PassingType<Args>...);
  Task(std::shared_ptr<TaskStateBase> task_state)
      : base_(task_state) {}
  virtual ~Task() = default;
  Task(const Task& task) = default;
  R Run(Args&&... unbound_args) {
    Func f = reinterpret_cast<Func>(base_->invoke_func_);
    return f(base_, std::forward<Args>(unbound_args)...);
  }

 private:
  std::shared_ptr<TaskStateBase> base_;
};

template <typename... Types>
struct TypeList {};

template <typename R, typename ArgList>
struct MakeFuncTypeImpl;

template <typename R, typename... Args>
struct MakeFuncTypeImpl<R, TypeList<Args...>> {
  using Type = R(Args...);
};

template <typename R, typename ArgList>
using MakeFuncType = typename MakeFuncTypeImpl<R, ArgList>::Type;

template <size_t n, typename List>
struct DropTypeListItemImpl;

template <size_t n, typename T, typename... List>
struct DropTypeListItemImpl<n, TypeList<T, List...>>
    : DropTypeListItemImpl<n - 1, TypeList<List...>> {};

// specialization
template <typename T, typename... List>
struct DropTypeListItemImpl<0, TypeList<T, List...>> {
  using Type = TypeList<T, List...>;
};

template <>
struct DropTypeListItemImpl<0, TypeList<>> {
  using Type = TypeList<>;
};

template <size_t n, typename List>
using DropTypeListItem = typename DropTypeListItemImpl<n, List>::Type;

template <typename Signature>
struct Traits;

template <typename R, typename... Args>
struct Traits<R (*)(Args...)> {
  using ReturnType = R;
  using ArgList = TypeList<Args...>;
   
  template<typename Function, typename... RunArgs>
  static R Invoke(Function&& function, RunArgs&&... args) {
      return std::forward<Function>(function)(std::forward<RunArgs>(args)...);
  }
};

template <typename R, typename Receiver, typename... Args>
struct Traits<R (Receiver::*)(Args...)> {
  using ReturnType = R;
  using ArgList = TypeList<Receiver*, Args...>;

  template<typename Method, typename Receiver, typename... RunArgs>
  static R Invoke(Method method, Receiver&& receiver, RunArgs&&... args) {
      return ((*receiver).*method)(std::forward<RunArgs>(args)...);
  }
};

// bound args
template <typename Functor, typename... Args>
static OnceClosure BindClosure(Functor&& func, Args&&... args) {
  using FunctorTraits = Traits<Functor>;
  using ValidateTaskStateType = TaskState<Functor, Args...>;
  using UnboundFuncType = MakeFuncType<typename FunctorTraits::ReturnType, DropTypeListItem<sizeof...(Args), typename FunctorTraits::ArgList>>;
  typename Task<UnboundFuncType>::Func invoke_func_storage = Invoker<FunctorTraits, ValidateTaskStateType, UnboundFuncType>::RunOnce;
  auto task_state = ValidateTaskStateType::Create(reinterpret_cast<TaskStateBase::InvokeFuncStorage>(invoke_func_storage), std::forward<Functor>(func), std::forward<Args>(args)...);
  return OnceClosure(task_state);
}

template <typename Functor, typename... Args>
static auto BindOnce(Functor&& func, Args&&... args) {
    using FunctorTraits = Traits<Functor>;
    using ValidateTaskStateType = TaskState<Functor, Args...>;
    using UnboundFuncType = MakeFuncType<typename FunctorTraits::ReturnType, DropTypeListItem<sizeof...(Args), typename FunctorTraits::ArgList>>;
    using TaskType = Task<UnboundFuncType>;
    typename Task<UnboundFuncType>::Func invoke_func_storage = Invoker<FunctorTraits, ValidateTaskStateType, UnboundFuncType>::RunOnce;
    auto task_state = ValidateTaskStateType::Create(reinterpret_cast<TaskStateBase::InvokeFuncStorage>(invoke_func_storage), std::forward<Functor>(func), std::forward<Args>(args)...);
    return TaskType(task_state);
}