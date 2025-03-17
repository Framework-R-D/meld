#ifndef DEMO_LOG_RECORD_H
#define DEMO_LOG_RECORD_H

namespace demo {
  inline void log_record(char const* event,
                         std::size_t spill_id,
                         std::size_t apa_id,
                         void const* active,
                         std::size_t data,
                         void const* orig)
  {
    spdlog::info("{}\t{}\t{}\t{}\t{}\t{}", event, spill_id, apa_id, active, data, orig);
  }
}

#endif