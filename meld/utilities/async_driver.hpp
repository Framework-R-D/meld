#ifndef meld_utilities_async_driver_hpp
#define meld_utilities_async_driver_hpp

#include "spdlog/spdlog.h"
#include "tbb/task.h"
#include "tbb/task_group.h"

#include <atomic>
#include <functional>
#include <optional>

namespace meld {

  template <typename RT>
  class async_driver {
    enum class states { off, drive, park };

  public:
    template <typename FT>
    async_driver(FT ft) : driver_{std::move(ft)}
    {
    }
    async_driver(void (*ft)(async_driver<RT>&)) : driver_{ft} {}

    ~async_driver() { group_.wait(); }

    std::optional<RT> operator()()
    {
      if (gear_ == states::off) {
        group_.run([&] {
          driver_(*this);
          gear_ = states::park;
          if (call_sp_ != tbb::task::suspend_point{}) {
            tbb::task::resume(call_sp_.exchange({}));
          }
        });
        gear_ = states::drive;
      }

      tbb::task::suspend([this](tbb::task::suspend_point call_sp) {
        call_sp_ = call_sp;
        if (yield_sp_ != tbb::task::suspend_point{}) {
          tbb::task::resume(yield_sp_.exchange({}));
        }
      });

      if (gear_ == states::park) {
        return std::nullopt;
      }

      return std::exchange(current_, std::nullopt);
    }

    void yield(RT rt)
    {
      tbb::task::suspend([this, current = std::move(rt)](tbb::task::suspend_point yield_sp) {
        yield_sp_ = yield_sp;
        current_ = std::make_optional(std::move(current));
        if (call_sp_ != tbb::task::suspend_point{}) {
          tbb::task::resume(call_sp_.exchange({}));
        }
      });
    }

  private:
    std::function<void(async_driver&)> driver_;
    std::optional<RT> current_;
    std::atomic<states> gear_ = states::off;
    tbb::task_group group_;
    std::atomic<tbb::task::suspend_point> yield_sp_;
    std::atomic<tbb::task::suspend_point> call_sp_;
  };

}

#endif // meld_utilities_async_driver_hpp
