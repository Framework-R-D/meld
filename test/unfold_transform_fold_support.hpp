#ifndef MELD_TEST_UNFOLD_TRANSFORM_FOLD_SUPPORT_HPP
#define MELD_TEST_UNFOLD_TRANSFORM_FOLD_SUPPORT_HPP

#include <array>
#include <utility>
#include <vector>

#include "test/log_record.hpp"
#include "test/waveform_generator_input.hpp"
#include "test/waveforms.hpp"

namespace demo {

  // This is the data product created by our fold node.
  struct SummedClampedWaveforms {
    std::size_t size = 0;
    double sum = 0.0;
  };

  // This is a class that provides unfold operation and a predicate to control
  // when the unfold is stopped.
  // Note that this class knows nothing about "spills" or "APAs". It does not
  // know where the values of maxsize and chunksize come from.
  class WaveformGenerator {
  public:
    // Create a WaveformGenerator that will generate waveforms exactly maxsize
    // waveforms. They will be spread across vectors each of size no more than
    //  chunksize.
    explicit WaveformGenerator(WGI const& maxsize) : maxsize_{maxsize.size}
    {
      log_record("wgc_ctor", maxsize.spill_id, 0, this, 0, nullptr);
    }

    // The initial value of the count of how many waveforms we have made so far.
    std::size_t initial_value() const { return 0; }

    // When we have made at least as many waveforms as we have been asked to make
    // for this unfold, we are done. The predicate answers the question "should
    // we continue unfolding?"
    bool predicate(std::size_t made_so_far) const
    {
      log_record("start_pred", 0, 0, this, made_so_far, nullptr);
      bool const result = made_so_far < maxsize_;
      log_record("end_pred", 0, 0, this, made_so_far, nullptr);
      return result;
    }

    // Generate the next chunk of waveforms, and update the count of how many
    // waveforms we have made so far.
    auto op(std::size_t made_so_far, std::size_t chunksize) const
    {
      // How many waveforms should go into this chunk?
      log_record("start_op", 0, 0, this, chunksize, nullptr);
      std::size_t const newsize = std::min(chunksize, maxsize_ - made_so_far);
      auto result =
        std::make_pair(made_so_far + newsize, Waveforms{newsize, 1.0 * made_so_far, 0, 0});
      log_record("end_op", 0, 0, this, newsize, nullptr);
      return result;
    }

  private:
    std::size_t const maxsize_; // total number of waveforms to make for the unfold
  }; // class WaveformGenerator

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