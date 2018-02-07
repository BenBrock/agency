#pragma once

#include <agency/detail/config.hpp>
#include <agency/detail/requires.hpp>
#include <agency/execution/executor/detail/adaptors/adaptations/bulk_then_execute_via_bulk_twoway_execute.hpp>
#include <agency/execution/executor/executor_traits/executor_future.hpp>
#include <agency/execution/executor/executor_traits/detail/is_bulk_twoway_executor.hpp>
#include <agency/execution/executor/executor_traits/detail/is_bulk_then_executor.hpp>
#include <agency/execution/executor/executor_traits/executor_execution_depth.hpp>
#include <agency/execution/executor/executor_traits/executor_shape.hpp>


namespace agency
{
namespace detail
{


__agency_exec_check_disable__
template<class Executor, class Function, class Future, class ResultFactory, class... Factories,
         __AGENCY_REQUIRES(is_bulk_then_executor<Executor>::value)>
__AGENCY_ANNOTATION
executor_future_t<Executor, result_of_t<ResultFactory()>>
  bulk_then_execute(const Executor& ex, Function f, executor_shape_t<Executor> shape, Future& predecessor, ResultFactory result_factory, Factories... shared_factories)
{
  return ex.bulk_then_execute(f, shape, predecessor, result_factory, shared_factories...);
}
         
template<class Executor, class Function, class Future, class ResultFactory, class... Factories,
         __AGENCY_REQUIRES(
           !is_bulk_then_executor<Executor>::value and
           is_bulk_twoway_executor<Executor>::value
        )>
__AGENCY_ANNOTATION
executor_future_t<Executor, result_of_t<ResultFactory()>>
  bulk_then_execute(const Executor& ex, Function f, executor_shape_t<Executor> shape, Future& predecessor, ResultFactory result_factory, Factories... shared_factories)
{
  return detail::bulk_then_execute_via_bulk_twoway_execute(ex, f, shape, predecessor, result_factory, shared_factories...);
}

// XXX consider introducing an adaptation for bulk oneway executors here

// XXX consider introducing adaptations for non-bulk executors here


} // end detail
} // end agency

