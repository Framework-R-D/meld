// ===================================================================
// This source creates 1M events.
// ===================================================================

#include "meld/source.hpp"

#include "spdlog/spdlog.h"

#include <ranges>

namespace test {
  class benchmarks_source {
  public:
    benchmarks_source(meld::configuration const& config) : max_{config.get<std::size_t>("n_events")}
    {
      spdlog::info("Processing {} events", max_);
    }

    void next(meld::framework_driver& driver) const
    {
      auto job_store = meld::product_store::base();
      driver.yield(job_store);

      for (std::size_t i : std::views::iota(0u, max_)) {
        if (i % (max_ / 10) == 0) {
          spdlog::debug("Reached {} events", i);
        }

        auto store = job_store->make_child(i, "event");
        store->add_product("id", *store->id());
        driver.yield(store);
      }
    }

  private:
    std::size_t max_;
  };
}

DEFINE_SOURCE(test::benchmarks_source)
