#include "meld/core/concurrency_tags.hpp"

#include "catch2/catch.hpp"

#include <string_view>

using namespace meld;

namespace {

  struct Data {
    static constexpr std::string_view
    name()
    {
      return "Data";
    }
  };
  struct More {
    static constexpr std::string_view
    name()
    {
      return "More";
    }
  };
  struct Maybe {
    static constexpr std::string_view
    name()
    {
      return "Maybe";
    }
  };

  struct A {
    void
    process(More const&, concurrency::serial)
    {
    }
  };
  struct B {
    void
    process(Data const&, concurrency::unlimited) const
    {
    }
  };
  struct C {
    void
    process(Data const&, concurrency::max<4>)
    {
    }
  };
  struct D {
    void
    process(Data const&, concurrency::unlimited)
    {
    }
    void
    process(More const&, concurrency::serial)
    {
    }
    void
    process(Maybe const&, concurrency::max<4>)
    {
    }
  };
  struct E {
    void
    process(Maybe const&) const
    {
    }
  };

  constexpr auto a_tag = concurrency_tag_for_process<A, More>();
  constexpr auto b_tag = concurrency_tag_for_process<B, Data>();
  constexpr auto c_tag = concurrency_tag_for_process<C, Data>();
  constexpr auto d_tag1 = concurrency_tag_for_process<D, Data>();
  constexpr auto d_tag2 = concurrency_tag_for_process<D, More>();
  constexpr auto d_tag3 = concurrency_tag_for_process<D, Maybe>();
  constexpr auto e_tag = concurrency_tag_for_process<E, Maybe>();

  static_assert(a_tag.value == 1);
  static_assert(b_tag.value == 0);
  static_assert(c_tag.value == 4);
  static_assert(d_tag1.value == 0);
  static_assert(d_tag2.value == 1);
  static_assert(d_tag3.value == 4);
  static_assert(e_tag.value == 1);

  static_assert(a_tag.is_specified);
  static_assert(b_tag.is_specified);
  static_assert(c_tag.is_specified);
  static_assert(d_tag1.is_specified);
  static_assert(d_tag2.is_specified);
  static_assert(d_tag3.is_specified);
  static_assert(not e_tag.is_specified);

  constexpr concurrencies<stage::process, A, Data, More, Maybe> a_cons;
  static_assert(a_cons.get<Data>() == std::nullopt);
  static_assert(a_cons.get<More>().value() == 1);
  static_assert(a_cons.get<Maybe>() == std::nullopt);
}

TEST_CASE("Concurreny tags", "[multithreading]")
{
  using namespace std::literals;
  CHECK(a_cons.get(0) == concurrency_values{"Data"sv, std::nullopt});
  CHECK(a_cons.get(1) == concurrency_values{"More"sv, 1});
  CHECK(a_cons.get(2) == concurrency_values{"Maybe"sv, std::nullopt});

  CHECK(a_cons.get("Data") == std::nullopt);
  CHECK(a_cons.get("More").value() == 1);
  CHECK(a_cons.get("Maybe") == std::nullopt);
}
