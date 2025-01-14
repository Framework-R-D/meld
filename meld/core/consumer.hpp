#ifndef meld_core_consumer_hpp
#define meld_core_consumer_hpp

#include "meld/model/algorithm_name.hpp"

#include <string>
#include <vector>

namespace meld {
  class consumer {
  public:
    consumer(algorithm_name name, std::vector<std::string> predicates);

    std::string full_name() const;
    std::string const& plugin() const noexcept;
    std::string const& algorithm() const noexcept;
    std::vector<std::string> const& when() const noexcept;

  private:
    algorithm_name name_;
    std::vector<std::string> predicates_;
  };
}

#endif // meld_core_consumer_hpp
