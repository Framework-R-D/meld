#include "meld/source.hpp"
#include "meld/model/product_store.hpp"

#include <ranges>

namespace {
  class number_generator {
  public:
    number_generator(meld::configuration const& config) : n_{config.get<int>("max_numbers")} {}

    void next(meld::framework_driver<meld::product_store_ptr>& driver) const
    {
      auto job_store = meld::product_store::base();
      driver.yield(job_store);

      for (int i : std::views::iota(1, n_ + 1)) {
        auto store = job_store->make_child(i, "event");
        store->add_product("i", i);
        store->add_product("j", -i);
        driver.yield(store);
      }
    }

  private:
    int n_;
  };
}

DEFINE_SOURCE(number_generator)
