#pragma once

#include <agency/detail/config.hpp>
#include <agency/new_executor_traits.hpp>
#include <agency/detail/executor_traits/check_for_member_functions.hpp>
#include <agency/detail/executor_traits/ignore_tail_parameters_and_invoke.hpp>
#include <type_traits>

namespace agency
{
namespace detail
{
namespace new_executor_traits_detail
{
namespace new_multi_agent_execute_returning_user_specified_container_implementation_strategies
{


__agency_hd_warning_disable__
template<class Executor, class Function, class Factory>
__AGENCY_ANNOTATION
typename std::result_of<Factory(typename new_executor_traits<Executor>::shape_type)>::type
  multi_agent_execute_returning_user_specified_container(std::true_type, Executor& ex, Function f, Factory result_factory, typename new_executor_traits<Executor>::shape_type shape)
{
  return ex.new_execute(f, result_factory, shape);
} // end multi_agent_execute_returning_user_specified_container()




template<size_t... Indices, class Executor, class Function, class Factory, class Tuple>
__AGENCY_ANNOTATION
typename std::result_of<Factory(typename new_executor_traits<Executor>::shape_type)>::type
  multi_agent_execute_returning_user_specified_container_impl(detail::index_sequence<Indices...>,
                                                              Executor& ex, Function f, Factory result_factory, typename new_executor_traits<Executor>::shape_type shape,
                                                              const Tuple& tuple_of_unit_factories)
{
  return new_executor_traits<Executor>::new_execute(ex, ignore_tail_parameters_and_invoke<Function>{f}, result_factory, shape, std::get<Indices>(tuple_of_unit_factories)...);
} // end multi_agent_execute_returning_user_specified_container()


template<class Executor, class Function, class Factory>
__AGENCY_ANNOTATION
typename std::result_of<Factory(typename new_executor_traits<Executor>::shape_type)>::type
  multi_agent_execute_returning_user_specified_container(std::false_type, Executor& ex, Function f, Factory result_factory, typename new_executor_traits<Executor>::shape_type shape)
{
  auto tuple_of_unit_factories = new_executor_traits_detail::make_tuple_of_unit_factories(ex);

  return multi_agent_execute_returning_user_specified_container_impl(detail::make_index_sequence<std::tuple_size<decltype(tuple_of_unit_factories)>::value>(), ex, f, result_factory, shape, tuple_of_unit_factories);
} // end multi_agent_execute_returning_user_specified_container()


} // end new_multi_agent_execute_returning_user_specified_container_implementation_strategies
} // end new_executor_traits_detail
} // end detail


template<class Executor>
  template<class Function, class Factory>
__AGENCY_ANNOTATION
typename std::result_of<Factory(typename new_executor_traits<Executor>::shape_type)>::type new_executor_traits<Executor>
  ::new_execute(typename new_executor_traits<Executor>::executor_type& ex,
                Function f,
                Factory result_factory,
                typename new_executor_traits<Executor>::shape_type shape)
{
  namespace ns = detail::new_executor_traits_detail::new_multi_agent_execute_returning_user_specified_container_implementation_strategies;

  using check_for_member_function = agency::detail::new_executor_traits_detail::new_has_multi_agent_execute_returning_user_specified_container<
    Executor,
    Function,
    Factory
  >;

  return ns::multi_agent_execute_returning_user_specified_container(check_for_member_function(), ex, f, result_factory, shape);
} // end new_executor_traits::execute()


} // end agency

