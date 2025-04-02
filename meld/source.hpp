#ifndef meld_source_hpp
#define meld_source_hpp

#include "boost/dll/alias.hpp"

#include "meld/configuration.hpp"
#include "meld/core/cached_product_stores.hpp"
#include "meld/core/framework_driver.hpp"
#include "meld/model/product_store.hpp"

#include <concepts>
#include <memory>

namespace meld::detail {

  // See note below.
  template <typename T>
  auto make(configuration const& config)
  {
    if constexpr (requires { T{config}; }) {
      return std::make_shared<T>(config);
    }
    else {
      return std::make_shared<T>();
    }
  }

  template <typename T>
  concept next_function_with_driver = requires(T t, framework_driver<product_store_ptr>& driver) {
    { t.next(driver) } -> std::same_as<void>;
  };

  template <typename T>
  concept next_function_without_driver = requires(T t) {
    { t.next() } -> std::same_as<void>;
  };

  template <typename T>
  std::function<void(framework_driver<product_store_ptr>&)> create_next(
    configuration const& config = {})
  {
    // N.B. Because we are initializing an std::function object with a lambda, the lambda
    //      (and therefore its captured values) must be copy-constructible.  This means
    //      that make<T>(config) must return a copy-constructible object.  Because we do not
    //      know if a user's provided source class is copyable, we create the object on
    //      the heap, and capture a shared pointer to the object.  This also ensures that
    //      the source object is created only once, thus avoiding potential errors in the
    //      implementations of the source class' copy/move constructors (e.g. if the
    //      source is caching an iterator).
    if constexpr (next_function_with_driver<T>) {
      return
        [t = make<T>(config)](framework_driver<product_store_ptr>& driver) { t->next(driver); };
    }
    else if (next_function_without_driver<T>) {
      return [t = make<T>(config)](framework_driver<product_store_ptr>&) { t->next(); };
    }
    else {
      static_assert(false, "Must have a 'next()' function that returns 'void'");
    }
  }

  // using next_store_t = std::function<product_store_ptr(cached_product_stores&)>;
  using next_store_t = std::function<void(framework_driver<product_store_ptr>&)>;
  using source_creator_t = next_store_t(configuration const&);
}

#define DEFINE_SOURCE(source) BOOST_DLL_ALIAS(meld::detail::create_next<source>, create_source)

#endif // meld_source_hpp
