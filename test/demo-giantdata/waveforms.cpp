#include "waveforms.hpp"
#include "log_record.hpp"

std::size_t demo::Waveforms::size() const { return waveforms.size(); }

demo::Waveforms::Waveforms(std::size_t n, double val, int spill_id, int apa_id) :
  waveforms(n, {val}), spill_id(spill_id), apa_id(apa_id)
{
  log_record("wsctor", spill_id, apa_id, this, n, nullptr);
}

demo::Waveforms::Waveforms(Waveforms const& other) :
  waveforms(other.waveforms), spill_id(other.spill_id), apa_id(other.apa_id)
{
  log_record("wscopy", spill_id, apa_id, this, waveforms.size(), &other);
}

demo::Waveforms::Waveforms(Waveforms&& other) :
  waveforms(std::move(other.waveforms)), spill_id(other.spill_id), apa_id(other.apa_id)
{
  log_record("wsmove", spill_id, apa_id, this, waveforms.size(), &other);
}

demo::Waveforms& demo::Waveforms::operator=(Waveforms const& other)
{
  waveforms = other.waveforms;
  spill_id = other.spill_id;
  apa_id = other.apa_id;
  log_record("wscopy=", spill_id, apa_id, this, waveforms.size(), &other);
  return *this;
}

demo::Waveforms& demo::Waveforms::operator=(Waveforms&& other)
{
  waveforms = std::move(other.waveforms);
  spill_id = other.spill_id;
  apa_id = other.apa_id;
  log_record("wsmove=", spill_id, apa_id, this, waveforms.size(), &other);
  return *this;
}

demo::Waveforms::~Waveforms()
{
  log_record("wsdtor", spill_id, apa_id, this, waveforms.size(), nullptr);
};
