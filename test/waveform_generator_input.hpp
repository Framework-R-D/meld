#ifndef SRC_MELD_TEST_WAVEFORM_GENERATOR_INPUT_HPP
#define SRC_MELD_TEST_WAVEFORM_GENERATOR_INPUT_HPP

#include <cstddef>

namespace demo {
  // This is the data product that our unfold node will receive from each spill.
  struct WaveformGeneratorInput {

    explicit WaveformGeneratorInput(std::size_t size = 0, int spill_id = 0);
    WaveformGeneratorInput(WaveformGeneratorInput const& other);
    WaveformGeneratorInput(WaveformGeneratorInput&& other);

    WaveformGeneratorInput& operator=(WaveformGeneratorInput const& other);
    WaveformGeneratorInput& operator=(WaveformGeneratorInput&& other);

    ~WaveformGeneratorInput();
    std::size_t size;
    int spill_id;
  };

  using WGI = WaveformGeneratorInput;
  inline std::size_t constexpr mysize(WaveformGeneratorInput const&) { return sizeof(WGI); };
} // namespace demo

#endif