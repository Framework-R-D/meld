// =======================================================================================
// This test executes the following graph
//
//     Multiplexer
//      |       |
//   get_time square
//      |       |
//      |      add(*)
//      |       |
//      |     scale
//      |       |
//     print_result [also includes output module]
//
// where the asterisk (*) indicates a fold step.  In terms of the data model,
// whenever the add node receives the flush token, a product is inserted at one level
// higher than the level processed by square and add nodes.
// =======================================================================================

#include "meld/core/cached_product_stores.hpp"
#include "meld/core/framework_driver.hpp"
#include "meld/core/framework_graph.hpp"
#include "meld/model/level_id.hpp"
#include "meld/model/product_store.hpp"
#include "test/products_for_output.hpp"

#include "catch2/catch_all.hpp"
#include "spdlog/spdlog.h"

#include <atomic>
#include <cmath>
#include <ctime>
#include <string>
#include <vector>

using namespace meld;

namespace {
  constexpr auto index_limit = 2u;
  constexpr auto number_limit = 5u;

  void levels_to_process(framework_driver<level_id_ptr>& driver)
  {
    driver.yield(level_id::base_ptr());
    for (unsigned i = 0u; i != index_limit; ++i) {
      auto id = level_id::base().make_child(i, "run");
      driver.yield(id);
      for (unsigned j = 0u; j != number_limit; ++j) {
        driver.yield(id->make_child(j, "event"));
      }
    }
  }

  auto square(unsigned int const num) { return num * num; }

  struct data_for_rms {
    unsigned int total;
    unsigned int number;
  };

  struct threadsafe_data_for_rms {
    std::atomic<unsigned int> total;
    std::atomic<unsigned int> number;
  };

  data_for_rms send(threadsafe_data_for_rms const& data)
  {
    return {meld::send(data.total), meld::send(data.number)};
  }

  void add(threadsafe_data_for_rms& redata, unsigned squared_number)
  {
    redata.total += squared_number;
    ++redata.number;
  }

  double scale(data_for_rms data)
  {
    return std::sqrt(static_cast<double>(data.total) / data.number);
  }

  std::string strtime(std::time_t tm)
  {
    char buffer[32];
    std::strncpy(buffer, std::ctime(&tm), 26);
    return buffer;
  }

  void print_result(handle<double> result, std::string const& stringized_time)
  {
    spdlog::debug("{}: {} @ {}",
                  result.level_id().to_string(),
                  *result,
                  stringized_time.substr(0, stringized_time.find('\n')));
  }
}

TEST_CASE("Hierarchical nodes", "[graph]")
{
  framework_driver drive{levels_to_process};
  framework_graph g{[&drive](cached_product_stores& cached_stores) mutable -> product_store_ptr {
    auto next_id = drive();
    if (not next_id) {
      return nullptr;
    }
    auto const& id = *next_id;
    auto store = cached_stores.get_store(id);

    if (id->level_name() == "run") {
      store->add_product<std::time_t>("time", std::time(nullptr));
    }
    if (id->level_name() == "event") {
      store->add_product<unsigned>("number", id->number() + id->parent()->number());
    }
    return store;
  }};

  g.with("get_the_time", strtime, concurrency::unlimited).when().transform("time").to("strtime");
  g.with(square, concurrency::unlimited).transform("number").to("squared_number");
  g.with(add, concurrency::unlimited)
    .when()
    .fold("squared_number")
    .partitioned_by("run")
    .to("added_data")
    .initialized_with(15u);

  g.with(scale, concurrency::unlimited).transform("added_data").to("result");
  g.with(print_result, concurrency::unlimited).observe("result", "strtime");

  g.make<test::products_for_output>().output_with(&test::products_for_output::save).when();

  g.execute("hierarchical_nodes_t");

  CHECK(g.execution_counts("square") == index_limit * number_limit);
  CHECK(g.execution_counts("add") == index_limit * number_limit);
  CHECK(g.execution_counts("get_the_time") >= index_limit);
  CHECK(g.execution_counts("scale") == index_limit);
  CHECK(g.execution_counts("print_result") == index_limit);
}
