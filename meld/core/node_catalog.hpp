#ifndef meld_core_node_catalog_hpp
#define meld_core_node_catalog_hpp

#include "meld/core/declared_fold.hpp"
#include "meld/core/declared_observer.hpp"
#include "meld/core/declared_output.hpp"
#include "meld/core/declared_predicate.hpp"
#include "meld/core/declared_transform.hpp"
#include "meld/core/declared_unfold.hpp"
#include "meld/core/registrar.hpp"

namespace meld {
  struct node_catalog {
    auto register_predicate(std::vector<std::string>& errors)
    {
      return registrar{predicates_, errors};
    }
    auto register_observer(std::vector<std::string>& errors)
    {
      return registrar{observers_, errors};
    }
    auto register_output(std::vector<std::string>& errors) { return registrar{outputs_, errors}; }
    auto register_fold(std::vector<std::string>& errors) { return registrar{folds_, errors}; }
    auto register_unfold(std::vector<std::string>& errors) { return registrar{unfolds_, errors}; }
    auto register_transform(std::vector<std::string>& errors)
    {
      return registrar{transforms_, errors};
    }

    declared_predicates predicates_{};
    declared_observers observers_{};
    declared_outputs outputs_{};
    declared_folds folds_{};
    declared_unfolds unfolds_{};
    declared_transforms transforms_{};
  };
}

#endif // meld_core_node_catalog_hpp
