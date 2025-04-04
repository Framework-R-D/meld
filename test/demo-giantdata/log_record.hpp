#ifndef DEMO_LOG_RECORD_H
#define DEMO_LOG_RECORD_H

#include "oneapi/tbb/concurrent_queue.h"
#include "oneapi/tbb/task_arena.h"
#include <chrono>
#include <cstring>
#include <fstream>
#include <iomanip>

namespace demo {

  std::size_t const EVENT_NAME_SIZE = 16;
  struct record {
    record(char const* event,
           std::size_t run_id,
           std::size_t subrun_id,
           std::size_t spill_id,
           std::size_t apa_id,
           void const* active,
           std::size_t data,
           void const* orig) :
      run_id(run_id),
      subrun_id(subrun_id),
      spill_id(spill_id),
      apa_id(apa_id),
      active(active),
      data(data),
      orig(orig),
      timestamp(),
      thread_index(tbb::this_task_arena::current_thread_index())
    {
      auto const t = std::chrono::system_clock::now().time_since_epoch();
      double const ts =
        static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(t).count());
      timestamp = ts * 1.0e-6;
      std::strncpy(this->event, event, EVENT_NAME_SIZE);
      this->event[EVENT_NAME_SIZE - 1] = '\0'; // ensure null-termination
    }

    std::size_t run_id;
    std::size_t subrun_id;
    std::size_t spill_id;
    std::size_t apa_id;
    void const* active;
    std::size_t data;
    void const* orig;
    double timestamp;
    char event[EVENT_NAME_SIZE];
    int thread_index;
  };

  inline std::ostream& operator<<(std::ostream& os, record const& r)
  {
    os << std::scientific << std::setprecision(std::numeric_limits<double>::max_digits10)
       << r.timestamp << '\t' << r.thread_index << '\t' << r.event << '\t' << r.run_id << '\t'
       << r.subrun_id << '\t' << r.spill_id << '\t' << r.apa_id << '\t' << r.active << '\t'
       << r.data << '\t' << r.orig;
    return os;
  }

  // Define a global queue for logging records
  inline oneapi::tbb::concurrent_queue<record> global_queue;

  inline void log_record(char const* event,
                         std::size_t run_id,
                         std::size_t subrun_id,
                         std::size_t spill_id,
                         std::size_t apa_id,
                         void const* active,
                         std::size_t data,
                         void const* orig)
  {
    record r(event, run_id, subrun_id, spill_id, apa_id, active, data, orig);
    //memcpy(r.event, event, EVENT_NAME_SIZE);

    global_queue.push(r);
  }

  inline void write_log(std::string const& filename)
  {
    std::ofstream log_file(filename, std::ios_base::out | std::ios_base::trunc);
    log_file << "time\tthread\tevent\tspill\tapa\tactive\tdata\torig\n";
    for (auto it = global_queue.unsafe_cbegin(), e = global_queue.unsafe_cend(); it != e; ++it) {
      log_file << *it << '\n';
    }
  }
} // namespace demo

#endif