#include "waveform_generator_input.hpp"
#include "log_record.hpp"

demo::WaveformGeneratorInput::WaveformGeneratorInput(std::size_t size, int spill_id) :
  size(size), spill_id(spill_id)
{
  log_record("wgictor", spill_id, 0, this, mysize(*this), nullptr);
}

demo::WaveformGeneratorInput::WaveformGeneratorInput(WaveformGeneratorInput const& other) :
  spill_id(other.spill_id)
{
  log_record("wgictor_copy", spill_id, 0, this, mysize(*this), nullptr);
}

demo::WaveformGeneratorInput::WaveformGeneratorInput(WaveformGeneratorInput&& other) :
  spill_id(other.spill_id)
{
  log_record("wgictor_move", spill_id, 0, this, mysize(*this), nullptr);
}

demo::WaveformGeneratorInput& demo::WaveformGeneratorInput::operator=(
  WaveformGeneratorInput const& other)
{
  log_record("wgictor_copy_assign", spill_id, 0, this, 0, nullptr);
  spill_id = other.spill_id;
  return *this;
}

demo::WaveformGeneratorInput& demo::WaveformGeneratorInput::operator=(
  WaveformGeneratorInput&& other)
{
  log_record("wgictor_move_assign", spill_id, 0, this, 0, nullptr);
  spill_id = other.spill_id;
  return *this;
}

demo::WaveformGeneratorInput::~WaveformGeneratorInput()
{
  log_record("wgidtor", spill_id, 0, this, mysize(*this), nullptr);
}
