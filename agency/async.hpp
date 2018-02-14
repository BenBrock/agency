#pragma once

#include <agency/detail/config.hpp>
#include <agency/detail/control_structures/bind.hpp>
#include <agency/execution/executor/executor_traits.hpp>
#include <agency/execution/executor/detail/utility/twoway_execute.hpp>
#include <agency/execution/executor/parallel_executor.hpp>
#include <agency/detail/type_traits.hpp>
#include <utility>

namespace agency
{


template<class Executor, class Function, class... Args>
__AGENCY_ANNOTATION
executor_future_t<
  Executor,
  detail::result_of_t<
    typename std::decay<Function&&>::type(typename std::decay<Args&&>::type...)
  >
>
async(const Executor& exec, Function&& f, Args&&... args)
{
  auto g = detail::bind(std::forward<Function>(f), std::forward<Args>(args)...);

  return detail::twoway_execute(exec, std::move(g));
}


template<class Function, class... Args>
executor_future_t<
  agency::detail::thread_pool_executor,
  detail::result_of_t<
    typename std::decay<Function&&>::type(typename std::decay<Args&&>::type...)
  >
>
  async(Function&& f, Args&&... args)
{
  agency::detail::thread_pool_executor exec;
  return agency::async(exec, std::forward<Function>(f), std::forward<Args>(args)...);
}


} // end agency

