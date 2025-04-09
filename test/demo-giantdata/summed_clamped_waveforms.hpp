// This is the data product created by our fold node.
#ifndef test_demo_giantdata_summed_clamped_waveforms_hpp
#define test_demo_giantdata_summed_clamped_waveforms_hpp

#include <cstddef>

namespace demo {

  struct SummedClampedWaveforms {
    std::size_t size = 0;
    double sum = 0.0;
  };

} // namespace demo

#endif // test_demo_giantdata_summed_clamped_waveforms_hpp
