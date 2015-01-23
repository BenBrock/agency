#pragma once

#include <future>
#include <agency/execution_categories.hpp>
#include <functional>

namespace agency
{


class vector_executor
{
  public:
    using execution_category = vector_execution_tag;

    template<class Function, class T>
    void bulk_invoke(Function f, size_t n, T shared_arg)
    {
      // ivdep requires gcc 4.9+
#if !defined(__INTEL_COMPILER) && !defined(__NVCC__) && (__GNUC__ >= 4) && (__GNUC_MINOR__ >= 9)
      #pragma GCC ivdep
#elif defined(__INTEL_COMPILER)
      #pragma simd
#endif
      for(size_t i = 0; i < n; ++i)
      {
        f(i, shared_arg);
      }
    }

    template<class Function, class T>
    std::future<void> bulk_async(Function f, size_t n, T shared_arg)
    {
      return std::async(std::launch::deferred, [=]
      {
        bulk_invoke(f, n, shared_arg);
      });
    }
};


} // end agency

