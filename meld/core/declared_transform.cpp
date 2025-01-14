#include "meld/core/declared_transform.hpp"

namespace meld {
  declared_transform::declared_transform(algorithm_name name, std::vector<std::string> predicates) :
    products_consumer{std::move(name), std::move(predicates)}
  {
  }

  declared_transform::~declared_transform() = default;
}
