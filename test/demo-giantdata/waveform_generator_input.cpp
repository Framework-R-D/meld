#include "waveform_generator_input.hpp"
#include "log_record.hpp"

demo::WaveformGeneratorInput::WaveformGeneratorInput(std::size_t size, int spill_id) :
  size(size), spill_id(spill_id)
{
  log_record("wgictor", spill_id, 0, this, mysize(*this), nullptr);
}

demo::WaveformGeneratorInput::WaveformGeneratorInput(WaveformGeneratorInput const& other) :
  size(other.size), spill_id(other.spill_id)
{
  log_record("wgicopy", spill_id, 0, this, mysize(*this), &other);
}

demo::WaveformGeneratorInput::WaveformGeneratorInput(WaveformGeneratorInput&& other) :
  size(other.size), spill_id(other.spill_id)
{
  log_record("wgimove", spill_id, 0, this, mysize(*this), &other);
}

demo::WaveformGeneratorInput& demo::WaveformGeneratorInput::operator=(
  WaveformGeneratorInput const& other)
{
  log_record("wgicopy=", spill_id, 0, this, 0, &other);
  size = other.size;
  spill_id = other.spill_id;
  return *this;
}

demo::WaveformGeneratorInput& demo::WaveformGeneratorInput::operator=(
  WaveformGeneratorInput&& other)
{
  log_record("wgimove=", spill_id, 0, this, 0, &other);
  size = other.size;
  spill_id = other.spill_id;
  return *this;
}

demo::WaveformGeneratorInput::~WaveformGeneratorInput()
{
  log_record("wgidtor", spill_id, 0, this, mysize(*this), nullptr);
}
