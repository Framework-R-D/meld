#include "meld/core/declared_observer.hpp"

namespace meld {
  declared_observer::declared_observer(algorithm_name name, std::vector<std::string> predicates) :
    products_consumer{std::move(name), std::move(predicates)}
  {
  }

  declared_observer::~declared_observer() = default;
}
