// =======================================================================================
// This plugin contains *both* a source and a observe.  This is not normally what anyone
// would want to do.  But Boost's DLL support is robust enough to handle this
// circumstance.
//
// The goal is to test whether the maximum allowed parallelism (as specified by either the
// meld command line, or configuration) agrees with what is expected.
// =======================================================================================

#include "meld/concurrency.hpp"
#include "meld/model/product_store.hpp"
#include "meld/module.hpp"
#include "meld/source.hpp"

#include <cassert>

using namespace meld;

namespace {
  class send_parallelism {
  public:
    void next(framework_driver<product_store_ptr>& driver)
    {
      auto job_store = product_store::base();
      job_store->add_product("max_parallelism",
                             concurrency::max_allowed_parallelism::active_value());
      driver.yield(job_store);
    }
  };
}

// Framework glue
DEFINE_SOURCE(send_parallelism)
DEFINE_MODULE(m, config)
{
  m.with("verify_expected",
         [expected = config.get<std::size_t>("expected_parallelism")](std::size_t actual) {
           assert(actual == expected);
         })
    .observe("max_parallelism");
}
