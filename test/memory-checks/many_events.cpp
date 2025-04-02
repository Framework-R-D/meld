#include "meld/core/framework_driver.hpp"
#include "meld/core/framework_graph.hpp"
#include "meld/model/product_store.hpp"
#include "test/products_for_output.hpp"

using namespace meld;

namespace {
  unsigned pass_on(unsigned number) { return number; }
}

int main()
{
  constexpr auto max_events{100'000u};
  // constexpr auto max_events{1'000'000u};
  // spdlog::flush_on(spdlog::level::trace);

  auto levels_to_process = [](framework_driver<product_store_ptr>& driver){
    auto job_store = product_store::base();
    driver.yield(job_store);

    for (unsigned int i : std::views::iota(1u, max_events + 1)) {
      auto event_store = job_store->make_child(i, "event", "Source");
      event_store->add_product("number", i);
      driver.yield(event_store);
    }
  };


  framework_graph g{levels_to_process};
  g.with(pass_on, concurrency::unlimited).transform("number").to("different");
  g.execute();
}
