#ifndef DEMO_LOG_RECORD_H
#define DEMO_LOG_RECORD_H

#include "oneapi/tbb/concurrent_queue.h"
#include <fstream>

namespace demo {

  std::size_t const EVENT_NAME_SIZE = 16;
  struct record {
    std::size_t spill_id;
    std::size_t apa_id;
    void const* active;
    std::size_t data;
    void const* orig;
    char event[EVENT_NAME_SIZE];
  };

  std::ostream& operator<<(std::ostream& os, record const& r)
  {
    os << r.event << '\t' << r.spill_id << '\t' << r.apa_id << '\t' << r.active << '\t' << r.data
       << '\t' << r.orig;
    return os;
  }

  static oneapi::tbb::concurrent_queue<record> global_queue;

  inline void log_record(char const* event,
                         std::size_t spill_id,
                         std::size_t apa_id,
                         void const* active,
                         std::size_t data,
                         void const* orig)
  {
    record r{spill_id, apa_id, active, data, orig, ""};
    memcpy(r.event, event, EVENT_NAME_SIZE);
    global_queue.push(r);
  }

  inline void write_log(std::string const& filename)
  {
    std::ofstream log_file(filename, std::ios_base::out | std::ios_base::trunc);
    log_file << "event\tspill_id\tapa_id\tactive\tdata\torig\n";
    for (auto it = global_queue.unsafe_cbegin(), e = global_queue.unsafe_cend(); it != e; ++it) {
      log_file << *it << '\n';
    }
  }
} // namespace demo

#endif