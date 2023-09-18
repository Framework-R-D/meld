#include "meld/model/handle.hpp"
#include "meld/model/level_id.hpp"
#include "meld/model/product_store.hpp"

#include "catch2/catch.hpp"

#include <concepts>
#include <string>
#include <vector>

using namespace meld;

namespace {
  struct Composer {
    std::string name;
  };
}

TEST_CASE("Handle type conversions (compile-time checks)", "[data model]")
{
  static_assert(std::same_as<handle_for<int>, handle<int>>);
  static_assert(std::same_as<handle_for<int const>, handle<int>>);
  static_assert(std::same_as<handle_for<int const&>, handle<int>>);
  static_assert(std::same_as<handle_for<int const*>, handle<int>>);
}

TEST_CASE("Can only create handles with compatible types", "[data model]")
{
  static_assert(std::constructible_from<handle<int>, product<int>>);
  static_assert(std::constructible_from<handle<int>, product<int>, level_id>);
  static_assert(not std::constructible_from<handle<int>, product<double>>);
  static_assert(not std::constructible_from<handle<int>, product<double>, level_id>);
}

TEST_CASE("Handle type conversions (run-time checks)", "[data model]")
{
  handle<double> empty;
  CHECK(not empty);
  CHECK_THROWS_WITH(*empty, Catch::Contains("Cannot dereference empty handle of type 'double'."));

  product<int> const number{3};
  handle const h{number};
  CHECK(handle<int const>{number} == h);
  CHECK(h.level_id() == level_id::base());

  int const& num_ref = h;
  int const* num_ptr = h;
  CHECK(static_cast<bool>(h));
  CHECK(num_ref == 3);
  CHECK(*num_ptr == 3);

  product<Composer> const composer{{"Elgar"}};
  CHECK(handle { composer } -> name == "Elgar");
}
