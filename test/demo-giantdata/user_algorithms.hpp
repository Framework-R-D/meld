#ifndef MELD_TEST_USER_ALGORITHMS_HPP
#define MELD_TEST_USER_ALGORITHMS_HPP

#include "summed_clamped_waveforms.hpp"
#include "waveforms.hpp"

namespace demo {

  // This function is used to transform an input Waveforms object into an
  // output Waveforms object. The output is a clamped version of the input.
  Waveforms clampWaveforms(Waveforms const& input,
                           std::size_t run_id,
                           std::size_t subrun_id,
                           std::size_t spill_id,
                           std::size_t apa_id);

  // This is the fold operator that will accumulate a SummedClampedWaveforms object.
  void accumulateSCW(SummedClampedWaveforms& scw, Waveforms const& wf);

} // namespace demo

#endif