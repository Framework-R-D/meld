#include "meld/source.hpp"
#include "meld/model/product_store.hpp"

#include "spdlog/spdlog.h"

#include <ranges>

namespace meld::test {
  class source {
  public:
    source(meld::configuration const& config) : max_{config.get<std::size_t>("n_events")}
    {
      spdlog::info("Processing {} events", max_);
    }

    void next(framework_driver<product_store_ptr>& driver) const
    {
      auto job_store = meld::product_store::base();
      driver.yield(job_store);

      for (std::size_t i : std::views::iota(0u, max_)) {
        if (max_ > 10 && (i % (max_ / 10) == 0)) {
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

DEFINE_SOURCE(meld::test::source)
