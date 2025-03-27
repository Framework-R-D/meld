#ifndef MELD_TEST_UNFOLD_TRANSFORM_FOLD_SUPPORT_HPP
#define MELD_TEST_UNFOLD_TRANSFORM_FOLD_SUPPORT_HPP

#include <array>
#include <utility>
#include <vector>

#include "test/demo-giantdata/log_record.hpp"
#include "test/demo-giantdata/summed_clamped_waveforms.hpp"
#include "test/demo-giantdata/waveform_generator.hpp"
#include "test/demo-giantdata/waveform_generator_input.hpp"
#include "test/demo-giantdata/waveforms.hpp"

namespace demo {

  // This function is used to transform an input Waveforms object into an
  // output Waveforms object. The output is a clamped version of the input.
  inline Waveforms clampWaveforms(Waveforms const& input)
  {
    log_record("start_clamp", input.spill_id, input.apa_id, &input, input.size(), nullptr);
    Waveforms result(input);
    for (Waveform& wf : result.waveforms) {
      for (double& x : wf.samples) {
        x = std::clamp(x, -10.0, 10.0);
      }
    }
    log_record("end_clamp", input.spill_id, input.apa_id, &input, input.size(), &result);
    return result;
  }

  // This is the fold operator that will accumulate a SummedClampedWaveforms object.
  inline void accumulateSCW(SummedClampedWaveforms& scw, Waveforms const& wf)
  {
    log_record("start_accSCW", wf.spill_id, wf.apa_id, &scw, wf.size(), &wf);
    scw.size += wf.size();
    for (auto const& w : wf.waveforms) {
      for (double x : w.samples) {
        scw.sum += x;
      }
    }
    log_record("end_accSCW", wf.spill_id, wf.apa_id, &scw, wf.size(), &wf);
  }

} // namespace demo

#endif