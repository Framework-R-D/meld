#ifndef test_cached_execution_source_hpp
#define test_cached_execution_source_hpp

// ===================================================================
// This source creates:
//
//  1 run
//    2 subruns per run
//      5000 events per subrun
// ===================================================================

#include "meld/core/framework_driver.hpp"

#include <ranges>

namespace test {
  inline constexpr std::size_t n_runs{1};
  inline constexpr std::size_t n_subruns{2u};
  inline constexpr std::size_t n_events{5000u};

  class cached_execution_source {
  public:
    // Uncopyable due to iterator
    cached_execution_source(cached_execution_source const&) = delete;
    cached_execution_source& operator=(cached_execution_source const&) = delete;

    void levels_to_process(meld::framework_driver<meld::product_store_ptr>& driver)
    {
      using namespace meld;

      auto job_store = product_store::base();
      driver.yield(job_store);

      for (std::size_t i : std::views::iota(0u, n_runs)) {
        auto run_store = job_store->make_child(i, "run");
        run_store->add_product<int>("number", 2 * i);
        driver.yield(run_store);

        for (std::size_t j : std::views::iota(0u, n_subruns)) {
          auto subrun_store = run_store->make_child(j, "subrun");
          subrun_store->add_product<int>("another", 3 * j);
          driver.yield(subrun_store);

          for (std::size_t k : std::views::iota(0u, n_events)) {
            auto event_store = subrun_store->make_child(k, "event");
            event_store->add_product<int>("still", 4 * k);
            driver.yield(event_store);
          }
        }
      }
    }

    cached_execution_source() : driver_{[this](auto& driver) { this->levels_to_process(driver); }}
    {
    }

    meld::product_store_ptr next()
    {
      if (auto next = driver_()) {
        return *next;
      }
      return nullptr;
    }

  private:
    meld::framework_driver<meld::product_store_ptr> driver_;
  };
}

#endif // test_cached_execution_source_hpp
