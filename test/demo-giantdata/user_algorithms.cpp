#include "user_algorithms.hpp"
#include "log_record.hpp"
#include "summed_clamped_waveforms.hpp"
#include "waveforms.hpp"

// This function is used to transform an input Waveforms object into an
// output Waveforms object. The output is a clamped version of the input.
demo::Waveforms demo::clampWaveforms(demo::Waveforms const& input)
{
  demo::log_record("start_clamp", input.spill_id, input.apa_id, &input, input.size(), nullptr);
  demo::Waveforms result(input);
  for (demo::Waveform& wf : result.waveforms) {
    for (double& x : wf.samples) {
      x = std::clamp(x, -10.0, 10.0);
    }
  }
  demo::log_record("end_clamp", input.spill_id, input.apa_id, &input, input.size(), &result);
  return result;
}

// This is the fold operator that will accumulate a SummedClampedWaveforms object.
void demo::accumulateSCW(demo::SummedClampedWaveforms& scw, demo::Waveforms const& wf)
{
  demo::log_record("start_accSCW", wf.spill_id, wf.apa_id, &scw, wf.size(), &wf);
  scw.size += wf.size();
  for (auto const& w : wf.waveforms) {
    for (double x : w.samples) {
      scw.sum += x;
    }
  }
  demo::log_record("end_accSCW", wf.spill_id, wf.apa_id, &scw, wf.size(), &wf);
}
