#pragma once

#include <agency/detail/config.hpp>
#include <agency/execution/execution_agent/experimental/detail/basic_static_execution_agent.hpp>
#include <agency/execution/execution_agent/parallel_agent.hpp>
#include <cstddef>

namespace agency
{
namespace experimental
{


template<std::size_t group_size, std::size_t grain_size = 1>
using static_parallel_agent = detail::basic_static_execution_agent<agency::parallel_agent, group_size, grain_size>;


} // end experimental
} // end agency

