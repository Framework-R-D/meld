// This is the data product created by our fold node.
#ifndef SUMMED_CLAMPED_WAVEFORMS_HPP
#define SUMMED_CLAMPED_WAVEFORMS_HPP

#include <cstddef>

namespace demo {

  struct SummedClampedWaveforms {
    std::size_t size = 0;
    double sum = 0.0;
  };

} // namespace demo

#endif // SUMMED_CLAMPED_WAVEFORMS_HPP
