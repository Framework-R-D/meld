#include "sand/core/data_levels.hpp"
#include "sand/core/module.hpp"

#include <iostream>

namespace sand::test {
  class multiple_levels {
  public:
    explicit multiple_levels(boost::json::object const&) {}

    void
    process(run const& r) const
    {
      std::cout << "Processing run " << r << " in multiple-levels module.\n";
    }

    void
    process(subrun const& sr) const
    {
      std::cout << "Processing subrun " << sr << " in multiple-levels module.\n";
    }
  };

  SAND_REGISTER_MODULE(multiple_levels, run, subrun)
}
