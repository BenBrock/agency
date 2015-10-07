#include <agency/cuda/grid_executor.hpp>
#include <agency/cuda/block_executor.hpp>
#include <agency/flattened_executor.hpp>
#include <agency/cuda/parallel_executor.hpp>
#include <agency/cuda/concurrent_executor.hpp>
#include <agency/nested_executor.hpp>
#include <agency/concurrent_executor.hpp>
#include <agency/sequential_executor.hpp>
#include <agency/parallel_executor.hpp>
#include <agency/executor_traits.hpp>
#include <iostream>

int main()
{
  std::cout << "is_executor<agency::sequential_executor>: " << agency::is_executor<agency::sequential_executor>::value << std::endl;
  std::cout << "has_then_execute<agency::sequential_executor>: " << agency::detail::has_any_multi_agent_then_execute<agency::sequential_executor>::value << std::endl;

  std::cout << "is_executor<agency::concurrent_executor>: " << agency::is_executor<agency::concurrent_executor>::value << std::endl;
  std::cout << "has_then_execute<agency::concurrent_executor>: " << agency::detail::has_any_multi_agent_then_execute<agency::concurrent_executor>::value << std::endl;

  std::cout << "is_executor<agency::parallel_executor>: " << agency::is_executor<agency::parallel_executor>::value << std::endl;
  std::cout << "has_then_execute<agency::parallel_executor>: " << agency::detail::has_any_multi_agent_then_execute<agency::parallel_executor>::value << std::endl;

  std::cout << "is_executor<agency::nested_executor<agency::concurrent_executor,agency::sequential_executor>>: " << agency::is_executor<agency::nested_executor<agency::concurrent_executor,agency::sequential_executor>>::value << std::endl;
  std::cout << "has_then_execute<agency::nested_executor<agency::concurrent_executor,agency::sequential_executor>>: " << agency::detail::has_any_multi_agent_then_execute<agency::nested_executor<agency::concurrent_executor,agency::sequential_executor>>::value << std::endl;


  std::cout << "is_executor<grid_executor>: " << agency::is_executor<agency::cuda::grid_executor>::value << std::endl;
  std::cout << "has_then_execute<grid_executor>: " << agency::detail::has_any_multi_agent_then_execute<agency::cuda::grid_executor>::value << std::endl;

  std::cout << "is_executor<block_executor>: " << agency::is_executor<agency::cuda::block_executor>::value << std::endl;
  std::cout << "has_then_execute<block_executor>: " << agency::detail::has_any_multi_agent_then_execute<agency::cuda::block_executor>::value << std::endl;

  std::cout << "is_executor<cuda::parallel_executor>: " << agency::is_executor<agency::cuda::parallel_executor>::value << std::endl;
  std::cout << "has_then_execute<cuda::parallel_executor>: " << agency::detail::has_any_multi_agent_then_execute<agency::cuda::parallel_executor>::value << std::endl;

  std::cout << "is_executor<cuda::concurrent_executor>: " << agency::is_executor<agency::cuda::concurrent_executor>::value << std::endl;
  std::cout << "has_then_execute<cuda::concurrent_executor>: " << agency::detail::has_any_multi_agent_then_execute<agency::cuda::concurrent_executor>::value << std::endl;

  return 0;
}
