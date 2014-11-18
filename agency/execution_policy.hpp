#pragma once

#include <future>
#include <utility>
#include <functional>
#include <type_traits>
#include <agency/execution_agent.hpp>
#include <functional>
#include <agency/executor_traits.hpp>
#include <agency/sequential_executor.hpp>
#include <agency/concurrent_executor.hpp>
#include <agency/parallel_executor.hpp>
#include <agency/vector_executor.hpp>
#include <agency/nested_executor.hpp>
#include <memory>
#include <tuple>
#include <initializer_list>
#include <agency/detail/shape_cast.hpp>
#include <agency/detail/index_cast.hpp>

namespace agency
{


// XXX add is_execution_policy_v
// XXX maybe this should simply check for the existence of the
//     nested type T::execution_category?
// XXX the problem is that execution agent as well as execution_agent_traits
//     also define this typedef
template<class T> struct is_execution_policy : std::false_type {};


namespace detail
{


template<class T, class Result = void>
struct enable_if_execution_policy
  : std::enable_if<is_execution_policy<T>::value,Result>
{};


template<class T, class Result = void>
struct disable_if_execution_policy
  : std::enable_if<!is_execution_policy<T>::value,Result>
{};


template<class... Types>
struct last_type_impl
{
  typedef typename std::tuple_element<sizeof...(Types) - 1, std::tuple<Types...>>::type type;
};


template<>
struct last_type_impl<>
{
  typedef void type;
};


template<class... Types>
using last_type = typename last_type_impl<Types...>::type;


template<class Result, class... Args>
struct enable_if_nested_call
  : enable_if_execution_policy<last_type<Args...>, Result>
{};


template<class Result, class... Args>
struct disable_if_nested_call
  : disable_if_execution_policy<
      decay_t<last_type<Args...>>,
      Result
    >
{};


// XXX we should assert that ExecutionCategory is stronger than the category of ExecutionAgent
// XXX another way to order these parameters would be
// ExecutionCategory, Asyncable, ExecutionGroup = __default_execution_group<ExecutionCategory>
template<class ExecutionAgent,
         class BulkExecutor,
         class ExecutionCategory = typename execution_agent_traits<ExecutionAgent>::execution_category>
class basic_execution_policy
{
  public:
    using execution_agent_type = ExecutionAgent;
    using executor_type        = BulkExecutor;
    using execution_category   = ExecutionCategory;

  private:
    using agent_traits         = execution_agent_traits<execution_agent_type>;

  public:
    using param_type           = typename agent_traits::param_type;

    basic_execution_policy() = default;

    basic_execution_policy(const param_type& param, const executor_type& executor = executor_type{})
      : param_(param),
        executor_(executor)
    {}

    const param_type& param() const
    {
      return param_;
    }

    executor_type& executor() const
    {
      return executor_;
    }

    template<class OtherBulkExecutor>
    basic_execution_policy<ExecutionAgent,OtherBulkExecutor,ExecutionCategory> on(const OtherBulkExecutor& executor) const
    {
      return basic_execution_policy<ExecutionAgent,OtherBulkExecutor,ExecutionCategory>(param(), executor);
    }

    template<class Arg1, class... Args>
    typename detail::disable_if_nested_call<basic_execution_policy, Arg1, Args...>::type
      operator()(Arg1&& arg1, Args&&... args) const
    {
      return basic_execution_policy(param_type{std::forward<Arg1>(arg1), std::forward<Args>(args)...}, executor());
    }

    template<class Arg1, class... Args>
    basic_execution_policy operator()(std::initializer_list<Arg1> arg1, std::initializer_list<Args>... args) const
    {
      return basic_execution_policy(param_type{std::move(arg1), std::move(args)...}, executor());
    }

  protected:
    param_type param_;

    // executor_ needs to be mutable, because:
    // * the global execution policy objects are constexpr
    // * executor.bulk_add() is a non-const member function
    mutable executor_type executor_;
};


// ExecutionCategory1 corresponds to the category of the executor
// XXX ExecutionCategory2 corresponds to the category of the policy
// XXX this overload should only be selected if ExecutionCategory1 is stronger than ExecutionCategory2
template<class ExecutionCategory1, class ExecutionAgent, class BulkExecutor, class ExecutionCategory2, class Function>
void bulk_invoke(ExecutionCategory1,
                 const basic_execution_policy<ExecutionAgent,BulkExecutor,ExecutionCategory2>& exec,
                 Function f)
{
  using traits = execution_agent_traits<ExecutionAgent>;

  auto param = exec.param();
  auto agent_shape = traits::domain(param).shape();
  auto shared_init = traits::make_shared_initializer(param);

  using executor_index_type = typename executor_traits<BulkExecutor>::index_type;
  using shared_param_type = typename executor_traits<BulkExecutor>::template shared_param_type<decltype(shared_init)>;

  // convert the shape of the agent into the type of the executor's shape
  using executor_shape_type = typename executor_traits<BulkExecutor>::shape_type;
  executor_shape_type executor_shape = detail::shape_cast<executor_shape_type>(agent_shape);

  return executor_traits<BulkExecutor>::bulk_invoke(exec.executor(), [=](executor_index_type executor_idx, shared_param_type shared_params)
  {
    // convert the index of the executor into the type of the agent's index
    using agent_index_type = typename traits::index_type;
    auto agent_idx = detail::index_cast<agent_index_type>(executor_idx, executor_shape, agent_shape);

    traits::execute(f, agent_idx, param, shared_params);
  },
  executor_shape,
  shared_init);
}


// ExecutionCategory1 corresponds to the category of the executor
// XXX ExecutionCategory2 corresponds to the category of the policy
// XXX this overload should only be selected if ExecutionCategory1 is stronger than ExecutionCategory2
template<class ExecutionCategory1, class ExecutionAgent, class BulkExecutor, class ExecutionCategory2, class Function>
std::future<void> bulk_async(ExecutionCategory1,
                             const basic_execution_policy<ExecutionAgent,BulkExecutor,ExecutionCategory2>& exec,
                             Function f)
{
  using traits = execution_agent_traits<ExecutionAgent>;

  auto param = exec.param();
  auto agent_shape = traits::domain(param).shape();
  auto shared_init = traits::make_shared_initializer(param);

  using executor_index_type = typename executor_traits<BulkExecutor>::index_type;
  using shared_param_type = typename executor_traits<BulkExecutor>::template shared_param_type<decltype(shared_init)>;

  // convert the shape of the agent into the type of the executor's shape
  using executor_shape_type = typename executor_traits<BulkExecutor>::shape_type;
  executor_shape_type executor_shape = detail::shape_cast<executor_shape_type>(agent_shape);

  return executor_traits<BulkExecutor>::bulk_async(exec.executor(), [=](executor_index_type executor_idx, shared_param_type shared_params)
  {
    // convert the index of the executor into the type of the agent's index
    using agent_index_type = typename traits::index_type;
    auto agent_idx = detail::index_cast<agent_index_type>(executor_idx, executor_shape, agent_shape);

    traits::execute(f, agent_idx, param, shared_params);
  },
  executor_shape,
  shared_init);
}


template<class ExecutionPolicy1, class ExecutionPolicy2>
class nested_execution_policy
  : public basic_execution_policy<
      execution_group<
        typename ExecutionPolicy1::execution_agent_type,
        typename ExecutionPolicy2::execution_agent_type
      >,
      nested_executor<
        typename ExecutionPolicy1::executor_type,
        typename ExecutionPolicy2::executor_type
      >,
      nested_execution_tag<
        typename ExecutionPolicy1::execution_category,
        typename ExecutionPolicy2::execution_category
      >
    >
{
  private:
    using super_t = basic_execution_policy<
      execution_group<
        typename ExecutionPolicy1::execution_agent_type,
        typename ExecutionPolicy2::execution_agent_type
      >,
      nested_executor<
        typename ExecutionPolicy1::executor_type,
        typename ExecutionPolicy2::executor_type
      >,
      nested_execution_tag<
        typename ExecutionPolicy1::execution_category,
        typename ExecutionPolicy2::execution_category
      >
    >;


  public:
    using outer_execution_policy_type = ExecutionPolicy1;
    using inner_execution_policy_type = ExecutionPolicy2;
    using typename super_t::execution_agent_type;
    using typename super_t::executor_type;

    nested_execution_policy(const outer_execution_policy_type& outer_exec,
                            const inner_execution_policy_type& inner_exec)
      : super_t(typename execution_agent_type::param_type(outer_exec.param(), inner_exec.param()),
                executor_type(outer_exec.executor(), inner_exec.executor()))
    {}
};


} // end detail



template<class ExecutionAgent, class BulkExecutor, class ExecutionCategory>
struct is_execution_policy<detail::basic_execution_policy<ExecutionAgent,BulkExecutor,ExecutionCategory>> : std::true_type {};


template<class T1, class T2>
struct is_execution_policy<detail::nested_execution_policy<T1,T2>>
  : std::integral_constant<
      bool,
      is_execution_policy<T1>::value && is_execution_policy<T2>::value
    >
{};


class sequential_execution_policy : public detail::basic_execution_policy<sequential_agent, sequential_executor>
{
  private:
    using super_t = detail::basic_execution_policy<sequential_agent, sequential_executor>;

  public:
    using super_t::basic_execution_policy;

    // XXX we shouldn't have to include either of these constructors due to the
    // using above
    sequential_execution_policy() = default;
    sequential_execution_policy(const super_t& other)
      : super_t(other)
    {}

    // this is the nested form of operator()
    // XXX we should try to move the implementation of this into the base class
    template<class OtherExecutionPolicy>
    detail::nested_execution_policy<sequential_execution_policy, OtherExecutionPolicy>
      operator()(size_t n, const OtherExecutionPolicy &exec) const
    {
      return detail::nested_execution_policy<sequential_execution_policy,OtherExecutionPolicy>(operator()(n), exec);
    }

    // only enable this operator() if the last parameter in (Arg1, Args...) is not an execution policy
    // this is the flat form of operator()
    template<class Arg1, class... Args>
    typename detail::disable_if_nested_call<sequential_execution_policy, Arg1, Args...>::type
      operator()(Arg1&& arg1, Args&&... args) const
    {
      // note we convert the result of super_t::operator() to sequential_execution_policy
      return super_t::operator()(std::forward<Arg1>(arg1), std::forward<Args>(args)...);
    }
};


template<> struct is_execution_policy<sequential_execution_policy> : std::true_type {};


const sequential_execution_policy seq{};


class concurrent_execution_policy : public detail::basic_execution_policy<concurrent_agent, concurrent_executor>
{
  private:
    using super_t = detail::basic_execution_policy<concurrent_agent, concurrent_executor>;

  public:
    using super_t::basic_execution_policy;

    // XXX we shouldn't have to include either of these constructors due to the
    // using above
    concurrent_execution_policy() = default;
    concurrent_execution_policy(const super_t& other)
      : super_t(other)
    {}

    // this is the nested form of operator()
    // XXX we should try to move the implementation of this into the base class
    template<class OtherExecutionPolicy>
    detail::nested_execution_policy<concurrent_execution_policy, OtherExecutionPolicy>
      operator()(size_t n, const OtherExecutionPolicy &exec) const
    {
      return detail::nested_execution_policy<concurrent_execution_policy,OtherExecutionPolicy>(operator()(n), exec);
    }

    // only enable this operator() if the last parameter in (Arg1, Args...) is not an execution policy
    // this is the flat form of operator()
    template<class Arg1, class... Args>
    typename detail::disable_if_nested_call<concurrent_execution_policy, Arg1, Args...>::type
      operator()(Arg1&& arg1, Args&&... args) const
    {
      return super_t::operator()(std::forward<Arg1>(arg1), std::forward<Args>(args)...);
    }
};


template<> struct is_execution_policy<concurrent_execution_policy> : std::true_type {};


const concurrent_execution_policy con{};


class parallel_execution_policy : public detail::basic_execution_policy<parallel_agent, parallel_executor>
{
  private:
    using super_t = detail::basic_execution_policy<parallel_agent, parallel_executor>;

  public:
    using super_t::basic_execution_policy;

    // XXX we shouldn't have to include either of these constructors due to the
    // using above
    parallel_execution_policy() = default;
    parallel_execution_policy(const super_t& other)
      : super_t(other)
    {}

    // this is the nested form of operator()
    // XXX we should try to move the implementation of this into the base class
    template<class OtherExecutionPolicy>
    detail::nested_execution_policy<parallel_execution_policy, OtherExecutionPolicy>
      operator()(size_t n, const OtherExecutionPolicy &exec) const
    {
      return detail::nested_execution_policy<parallel_execution_policy,OtherExecutionPolicy>(operator()(n), exec);
    }

    // only enable this operator() if the last parameter in (Arg1, Args...) is not an execution policy
    // this is the flat form of operator()
    template<class Arg1, class... Args>
    typename detail::disable_if_nested_call<parallel_execution_policy, Arg1, Args...>::type
      operator()(Arg1&& arg1, Args&&... args) const
    {
      return super_t::operator()(std::forward<Arg1>(arg1), std::forward<Args>(args)...);
    }
};


template<> struct is_execution_policy<parallel_execution_policy> : std::true_type {};


const parallel_execution_policy par{};


class vector_execution_policy : public detail::basic_execution_policy<vector_agent, vector_executor>
{
  private:
    using super_t = detail::basic_execution_policy<vector_agent, vector_executor>;

  public:
    using super_t::basic_execution_policy;

    // XXX we shouldn't have to include either of these constructors due to the
    // using above
    vector_execution_policy() = default;
    vector_execution_policy(const super_t& other)
      : super_t(other)
    {}

    // this is the nested form of operator()
    // XXX we should try to move the implementation of this into the base class
    template<class OtherExecutionPolicy>
    detail::nested_execution_policy<vector_execution_policy, OtherExecutionPolicy>
      operator()(size_t n, const OtherExecutionPolicy &exec) const
    {
      return detail::nested_execution_policy<vector_execution_policy,OtherExecutionPolicy>(operator()(n), exec);
    }

    // only enable this operator() if the last parameter in (Arg1, Args...) is not an execution policy
    // this is the flat form of operator()
    template<class Arg1, class... Args>
    typename detail::disable_if_nested_call<vector_execution_policy, Arg1, Args...>::type
      operator()(Arg1&& arg1, Args&&... args) const
    {
      return super_t::operator()(std::forward<Arg1>(arg1), std::forward<Args>(args)...);
    }
};


template<> struct is_execution_policy<vector_execution_policy> : std::true_type {};


const vector_execution_policy vec{};


template<class ExecutionPolicy, class Function, class... Args>
typename detail::enable_if_execution_policy<detail::decay_t<ExecutionPolicy>>::type
bulk_invoke(ExecutionPolicy&& exec, Function&& f, Args&&... args)
{
  // XXX the execution_category we dispatch on should be the category of the policy's executor
  using execution_category = typename detail::decay_t<ExecutionPolicy>::execution_category;
  return detail::bulk_invoke(execution_category(), exec, std::bind(f, std::placeholders::_1, args...));
}


template<class ExecutionPolicy, class Function, class... Args>
typename detail::enable_if_execution_policy<detail::decay_t<ExecutionPolicy>,std::future<void>>::type
bulk_async(ExecutionPolicy&& exec, Function&& f, Args&&... args)
{
  // XXX the execution_category we dispatch on should be the category of the policy's executor
  using execution_category = typename detail::decay_t<ExecutionPolicy>::execution_category;
  return detail::bulk_async(execution_category(), exec, std::bind(f, std::placeholders::_1, args...));
}


} // end agency

