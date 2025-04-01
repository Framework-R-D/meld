#ifndef meld_core_framework_driver_hpp
#define meld_core_framework_driver_hpp

#include "spdlog/spdlog.h"
#include "tbb/task.h"
#include "tbb/task_group.h"

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <optional>

namespace meld {

  template <typename RT>
  class framework_driver {
    enum class states { off, drive, park };

  public:
    template <typename FT>
    framework_driver(FT ft) : driver_{std::move(ft)}
    {
    }
    framework_driver(void (*ft)(framework_driver<RT>&)) : driver_{ft} {}

    ~framework_driver() { group_.wait(); }

    std::optional<RT> operator()()
    {
      if (gear_ == states::off) {
        group_.run([&] {
          driver_(*this);
          gear_ = states::park;
          cv_.notify_one();
        });
        gear_ = states::drive;
      }
      else {
        tbb::task::resume(sp_);
      }

      std::unique_lock lock{mutex_};
      cv_.wait(lock, [&] { return current_.has_value() or gear_ == states::park; });
      return std::exchange(current_, std::nullopt);
    }

    void yield(RT rt)
    {
      tbb::task::suspend([this, current = std::move(rt)](tbb::task::suspend_point sp) {
        sp_ = sp;
        current_ = std::make_optional(std::move(current));
        std::unique_lock lock{mutex_};
        cv_.notify_one();
      });
    }

  private:
    std::function<void(framework_driver&)> driver_;
    std::optional<RT> current_;
    std::atomic<states> gear_ = states::off;
    tbb::task_group group_;
    tbb::task::suspend_point sp_;
    std::mutex mutex_;
    std::condition_variable cv_;
  };

}

#endif // meld_core_framework_driver_hpp
