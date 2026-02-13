/*
 * Licensed under the Apache License Version 2.0 with LLVM Exceptions
 * (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 *
 *   https://llvm.org/LICENSE.txt
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <catch2/catch.hpp>
#include <stdexec/execution.hpp>

STDEXEC_PRAGMA_IGNORE_GNU("-Wdeprecated-declarations")
STDEXEC_PRAGMA_IGNORE_EDG(deprecated_entity_with_custom_message)
STDEXEC_PRAGMA_IGNORE_MSVC(4996) // 'foo': was declared deprecated

namespace ex = STDEXEC;

namespace {

  struct my_forwarding_query_t {
    [[nodiscard]]
    constexpr auto query(ex::forwarding_query_t) const noexcept -> bool {
      return true;
    }
  };

  inline constexpr my_forwarding_query_t my_forwarding_query{};

  struct my_derived_forwarding_query_t : ex::forwarding_query_t { };

  inline constexpr my_derived_forwarding_query_t my_derived_forwarding_query{};

  struct my_non_forwarding_query_t { };

  inline constexpr my_non_forwarding_query_t my_non_forwarding_query{};

  constexpr struct FwdFoo
    : STDEXEC::__query<FwdFoo>
    , STDEXEC::forwarding_query_t {
    using STDEXEC::__query<FwdFoo>::operator();
  } fwd_foo{};

  constexpr struct Foo : STDEXEC::__query<Foo> {
  } foo{};

  constexpr struct Bar : STDEXEC::__query<Bar> {
  } bar{};

  TEST_CASE("exec.queries are forwarding queries", "[exec.queries][forwarding_queries]") {
    static_assert(ex::forwarding_query(ex::get_allocator));
    static_assert(ex::forwarding_query(ex::get_stop_token));
    static_assert(ex::forwarding_query(ex::get_scheduler));
    static_assert(ex::forwarding_query(ex::get_delegation_scheduler));
    static_assert(
      std::is_same_v<ex::get_delegatee_scheduler_t, ex::get_delegation_scheduler_t>,
      "Deprecated type is the same type.");
    static_assert(&ex::get_delegatee_scheduler == &ex::get_delegation_scheduler);
    static_assert(ex::forwarding_query(ex::get_completion_scheduler<ex::set_value_t>));
    static_assert(ex::forwarding_query(ex::get_completion_scheduler<ex::set_error_t>));
    static_assert(ex::forwarding_query(ex::get_completion_scheduler<ex::set_stopped_t>));

    static_assert(ex::forwarding_query(my_forwarding_query));
    static_assert(ex::forwarding_query(my_derived_forwarding_query));
    static_assert(!ex::forwarding_query(my_non_forwarding_query));
  }

  TEST_CASE("nested properties are not treated equal", "[exec.queries][forwarding_queries]") {
    static_assert([]() {
      auto env = STDEXEC::env{
        STDEXEC::env{STDEXEC::prop{fwd_foo, 42.}, STDEXEC::prop{foo, 'F'}},
        STDEXEC::prop{                        bar,                   31415}
      };

      static_assert(!STDEXEC::__queryable_with<decltype(env), Foo>);

      return fwd_foo(env) == 42. && bar(env) == 31415;
    }());
  }
} // namespace
