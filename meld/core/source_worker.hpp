#ifndef meld_core_source_worker_hpp
#define meld_core_source_worker_hpp

#include "meld/graph/node.hpp"

#include "boost/json.hpp"

#include <memory>
#include <vector>

namespace meld {
  class source_worker {
  public:
    virtual ~source_worker() = default;

    // Replace with unique_ptr, once I figure out how to handle
    // std::function<void()> copyability issues.
    transition_messages
    next()
    {
      return next_transitions();
    }

  private:
    virtual transition_messages next_transitions() = 0;
  };

  using source_creator_t = std::unique_ptr<source_worker>(boost::json::value const&);
}

#endif /* meld_core_source_worker_hpp */
