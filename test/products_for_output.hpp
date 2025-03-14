#ifndef test_products_for_output_hpp
#define test_products_for_output_hpp

#include "meld/model/product_store.hpp"

#include "spdlog/spdlog.h"

#include <sstream>

namespace meld::test {
  struct products_for_output {
    void save(product_store const& store) const
    {
      std::ostringstream oss;
      oss << "Saving data for store id: " << store.id() << " from source: " << store.source();
      for (auto const& [product_name, _] : store) {
        oss << "\n -> Product name: " << product_name;
      }
      spdlog::debug(oss.str());
    }
  };

}

#endif // test_products_for_output_hpp
