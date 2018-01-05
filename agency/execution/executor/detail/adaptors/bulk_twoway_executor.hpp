// Copyright (c) 2018, NVIDIA CORPORATION. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//  * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//  * Neither the name of NVIDIA CORPORATION nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#pragma once

#include <agency/detail/config.hpp>
#include <agency/detail/requires.hpp>
#include <agency/detail/type_traits.hpp>
#include <agency/execution/executor/detail/adaptors/basic_executor_adaptor.hpp>
#include <agency/execution/executor/executor_traits/executor_future.hpp>
#include <agency/execution/executor/executor_traits/detail/is_bulk_twoway_executor.hpp>
#include <agency/future.hpp>


namespace agency
{
namespace detail
{


template<class Executor>
class bulk_twoway_executor : public basic_executor_adaptor<Executor>
{
  private:
    using super_t = basic_executor_adaptor<Executor>;

  public:
    __AGENCY_ANNOTATION
    bulk_twoway_executor(const Executor& ex) noexcept : super_t{ex} {}

    template<class Function, class ResultFactory, class... Factories,
             __AGENCY_REQUIRES(executor_execution_depth<Executor>::value == sizeof...(Factories))
            >
    __AGENCY_ANNOTATION
    executor_future_t<Executor, result_of_t<ResultFactory()>>
      bulk_twoway_execute(Function f, executor_shape_t<Executor> shape, ResultFactory result_factory, Factories... shared_factories) const
    {
      return bulk_twoway_execute_impl(f, shape, result_factory, shared_factories...);
    }

  private:
    template<class Function, class ResultFactory, class... Factories,
             __AGENCY_REQUIRES(is_bulk_twoway_executor<super_t>::value)
            >
    __AGENCY_ANNOTATION
    executor_future_t<Executor, result_of_t<ResultFactory()>>
      bulk_twoway_execute_impl(Function f, executor_shape_t<Executor> shape, ResultFactory result_factory, Factories... shared_factories) const
    {
      return super_t::bulk_twoway_execute(f, shape, result_factory, shared_factories...);
    }

    __agency_exec_check_disable__
    template<class Function, class ResultFactory, class... Factories,
             __AGENCY_REQUIRES(!is_bulk_twoway_executor<super_t>::value and is_bulk_then_executor<super_t>::value)
            >
    __AGENCY_ANNOTATION
    executor_future_t<Executor, result_of_t<ResultFactory()>>
      bulk_twoway_execute_impl(Function f, executor_shape_t<Executor> shape, ResultFactory result_factory, Factories... shared_factories) const
    {
      using void_future_type = executor_future_t<Executor,void>;

      // XXX we might want to actually allow the executor to participate here
      auto predecessor = future_traits<void_future_type>::make_ready();

      return super_t::bulk_then_execute(f, shape, predecessor, result_factory, shared_factories...);
    }

    // XXX consider introducing an adaptation for bulk oneway executors here

    // XXX consider introducing adaptations for non-bulk executors here
};


} // end detail
} // end agency

