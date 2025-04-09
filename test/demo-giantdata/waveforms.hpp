// waveforms.hpp
#ifndef test_demo_giantdata_waveforms_hpp
#define test_demo_giantdata_waveforms_hpp

#include <array>
#include <vector>

namespace demo {

  struct Waveform {
    // We should be set to the number of samples on a wire.
    std::array<double, 3 * 1024> samples;
  };

  struct Waveforms {
    std::vector<Waveform> waveforms;
    int run_id;
    int subrun_id;
    int spill_id;
    int apa_id;

    std::size_t size() const;

    Waveforms(std::size_t n, double val, int run_id, int subrun_id, int spill_id, int apa_id);
    Waveforms(Waveforms const& other);
    Waveforms(Waveforms&& other);

    // instrument copy assignment and move assignment
    Waveforms& operator=(Waveforms const& other);
    Waveforms& operator=(Waveforms&& other);

    ~Waveforms();
  };

} // namespace demo
#endif // test_demo_giantdata_waveforms_hpp
