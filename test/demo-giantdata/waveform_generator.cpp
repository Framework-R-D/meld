#include "waveforms.hpp"

#include "log_record.hpp"
#include "waveform_generator.hpp"
#include "waveform_generator_input.hpp"

#include <cstddef>

demo::WaveformGenerator::WaveformGenerator(WGI const& wgi) :
  maxsize_{wgi.size}, spill_id_(wgi.spill_id)
{
  log_record("wgctor", spill_id_, 0, this, sizeof(*this), nullptr);
}

demo::WaveformGenerator::~WaveformGenerator()
{
  log_record("wgdtor", spill_id_, 0, this, sizeof(*this), nullptr);
}

std::size_t demo::WaveformGenerator::initial_value() const { return 0; }

bool demo::WaveformGenerator::predicate(std::size_t made_so_far) const
{
  log_record("start_pred", spill_id_, 0, this, made_so_far, nullptr);
  bool const result = made_so_far < maxsize_;
  log_record("end_pred", spill_id_, 0, this, made_so_far, nullptr);
  return result;
}

std::pair<std::size_t, demo::Waveforms> demo::WaveformGenerator::op(std::size_t made_so_far,
                                                                    std::size_t chunksize) const
{
  // How many waveforms should go into this chunk?
  log_record("start_op", spill_id_, 0, this, chunksize, nullptr);
  std::size_t const newsize = std::min(chunksize, maxsize_ - made_so_far);
  auto result = std::make_pair(made_so_far + newsize, Waveforms{newsize, 1.0 * made_so_far, 0, 0});
  log_record("end_op", spill_id_, 0, this, newsize, nullptr);
  return result;
}