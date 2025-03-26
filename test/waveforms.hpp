// waveforms.hpp
#ifndef SRC_MELD_TEST_WAVEFORMS_HPP
#define SRC_MELD_TEST_WAVEFORMS_HPP

#include <array>
#include <vector>

namespace demo {

  struct Waveform {
    // We should be set to the number of samples on a wire.
    std::array<double, 3 * 1024> samples;
  };

  struct Waveforms {
    std::vector<Waveform> waveforms;
    int spill_id;
    int apa_id;

    std::size_t size() const;

    Waveforms(std::size_t n, double val, int spill_id, int apa_id);
    Waveforms(Waveforms const& other);
    Waveforms(Waveforms&& other);

    // instrument copy assignment and move assignment
    Waveforms& operator=(Waveforms const& other);
    Waveforms& operator=(Waveforms&& other);

    ~Waveforms();
  };

} // namespace demo
#endif // SRC_MELD_TEST_WAVEFORMS_HPP
