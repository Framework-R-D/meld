#ifndef MELD_TEST_DEMO_GIANTDATA_WAVEFORM_GENERATOR_HPP
#define MELD_TEST_DEMO_GIANTDATA_WAVEFORM_GENERATOR_HPP

#include "test/demo-giantdata/log_record.hpp"
#include "test/demo-giantdata/waveform_generator_input.hpp"
#include "test/demo-giantdata/waveforms.hpp"

#include <cstddef>

namespace demo {
  // This is a class that provides unfold operation and a predicate to control
  // when the unfold is stopped.
  // Note that this class knows nothing about "spills" or "APAs". It does not
  // know where the values of maxsize and chunksize come from.
  class WaveformGenerator {
  public:
    // Create a WaveformGenerator that will generate waveforms exactly maxsize
    // waveforms. They will be spread across vectors each of size no more than
    //  chunksize.
    explicit WaveformGenerator(WGI const& wgi) : maxsize_{wgi.size}, spill_id_(wgi.spill_id)
    {
      log_record("wgctor", spill_id_, 0, this, sizeof(*this), nullptr);
    }

    WaveformGenerator(WaveformGenerator const&) = delete;
    WaveformGenerator(WaveformGenerator&&) = delete;
    WaveformGenerator& operator=(WaveformGenerator const&) = delete;
    WaveformGenerator& operator=(WaveformGenerator&&) = delete;

    ~WaveformGenerator() { log_record("wgdtor", spill_id_, 0, this, sizeof(*this), nullptr); }

    // The initial value of the count of how many waveforms we have made so far.
    std::size_t initial_value() const { return 0; }

    // When we have made at least as many waveforms as we have been asked to make
    // for this unfold, we are done. The predicate answers the question "should
    // we continue unfolding?"
    bool predicate(std::size_t made_so_far) const
    {
      log_record("start_pred", spill_id_, 0, this, made_so_far, nullptr);
      bool const result = made_so_far < maxsize_;
      log_record("end_pred", spill_id_, 0, this, made_so_far, nullptr);
      return result;
    }

    // Generate the next chunk of waveforms, and update the count of how many
    // waveforms we have made so far.
    auto op(std::size_t made_so_far, std::size_t chunksize) const
    {
      // How many waveforms should go into this chunk?
      log_record("start_op", spill_id_, 0, this, chunksize, nullptr);
      std::size_t const newsize = std::min(chunksize, maxsize_ - made_so_far);
      auto result =
        std::make_pair(made_so_far + newsize, Waveforms{newsize, 1.0 * made_so_far, 0, 0});
      log_record("end_op", spill_id_, 0, this, newsize, nullptr);
      return result;
    }

  private:
    std::size_t maxsize_; // total number of waveforms to make for the unfold
    int spill_id_;        // the id of the spill this object will process
  }; // class WaveformGenerator
} // namespace demo
#endif // MELD_TEST_DEMO_GIANTDATA_WAVEFORM_GENERATOR_HPP