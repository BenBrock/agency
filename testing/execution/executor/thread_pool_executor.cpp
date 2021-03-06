#include <iostream>
#include <type_traits>
#include <vector>

// XXX use parallel_executor.hpp instead of thread_pool.hpp due to circular #inclusion problems
#include <agency/execution/executor/parallel_executor.hpp>
#include <agency/execution/executor/executor_traits.hpp>
#include <agency/execution/executor/executor_traits/detail/is_bulk_then_executor.hpp>
#include <agency/execution/executor/customization_points.hpp>
#include <agency/execution/executor/properties/bulk_guarantee.hpp>


int main()
{
  using namespace agency;

  static_assert(detail::is_bulk_then_executor<detail::thread_pool_executor>::value,
    "thread_pool_executor should be a bulk then executor");

  static_assert(bulk_guarantee_t::static_query<detail::thread_pool_executor>() == bulk_guarantee_t::parallel_t(),
    "thread_pool_executor should have parallel static bulk guarantee");

  static_assert(detail::is_detected_exact<size_t, executor_shape_t, detail::thread_pool_executor>::value,
    "thread_pool_executor should have size_t shape_type");

  static_assert(detail::is_detected_exact<size_t, executor_index_t, detail::thread_pool_executor>::value,
    "thread_pool_executor should have size_t index_type");

  static_assert(detail::is_detected_exact<std::future<int>, executor_future_t, detail::thread_pool_executor, int>::value,
    "thread_pool_executor should have std::future future");

  static_assert(executor_execution_depth<detail::thread_pool_executor>::value == 1,
    "thread_pool_executor should have execution_depth == 1");

  detail::thread_pool_executor exec;


  {
    // bulk_then_execute() with non-void predecessor
    
    std::future<int> predecessor_fut = agency::make_ready_future<int>(exec, 7);

    size_t shape = 10;
    
    auto f = exec.bulk_then_execute(
      [](size_t idx, int& predecessor, std::vector<int>& results, std::vector<int>& shared_arg)
      {
        results[idx] = predecessor + shared_arg[idx];
      },
      shape,
      predecessor_fut,
      [=]{ return std::vector<int>(shape); },     // results
      [=]{ return std::vector<int>(shape, 13); }  // shared_arg
    );
    
    auto result = f.get();
    
    assert(std::vector<int>(10, 7 + 13) == result);
  }


  {
    // bulk_then_execute() with void predecessor
    
    std::future<void> predecessor_fut = agency::make_ready_future<void>(exec);

    size_t shape = 10;
    
    auto f = exec.bulk_then_execute(
      [](size_t idx, std::vector<int>& results, std::vector<int>& shared_arg)
      {
        results[idx] = shared_arg[idx];
      },
      shape,
      predecessor_fut,
      [=]{ return std::vector<int>(shape); },     // results
      [=]{ return std::vector<int>(shape, 13); }  // shared_arg
    );
    
    auto result = f.get();
    
    assert(std::vector<int>(10, 13) == result);
  }

  std::cout << "OK" << std::endl;

  return 0;
}


