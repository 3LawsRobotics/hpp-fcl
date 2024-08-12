//
// Copyright (c) 2021-2023 INRIA
//

#ifndef HPP_FCL_TIMINGS_FWD_H
#define HPP_FCL_TIMINGS_FWD_H

#include "hpp/fcl/fwd.hh"

#ifdef HPP_FCL_WITH_CXX11_SUPPORT
#include <chrono>
#endif

namespace hpp {
namespace fcl {

struct CPUTimes {
  FCL_REAL wall;
  FCL_REAL user;
  FCL_REAL system;

  CPUTimes() : wall(0), user(0), system(0) {}

  void clear() { wall = user = system = 0; }
};

///
/// @brief This class mimics the way "boost/timer/timer.hpp" operates while
/// using the modern std::chrono library.
///        Importantly, this class will only have an effect for C++11 and more.
///
struct HPP_FCL_DLLAPI Timer {
#ifdef HPP_FCL_WITH_CXX11_SUPPORT
  typedef std::chrono::steady_clock clock_type;
  typedef clock_type::duration duration_type;
#endif

  /// \brief Default constructor for the timer
  ///
  /// \param[in] start_on_construction if true, the timer will be run just after
  /// the object is created
  Timer(const bool start_on_construction = true) : m_is_stopped(true) {
    if (start_on_construction)
      Timer::start();
  }

  CPUTimes elapsed() const {
    if (m_is_stopped)
      return m_times;

    CPUTimes current(m_times);
#ifdef HPP_FCL_WITH_CXX11_SUPPORT
    std::chrono::time_point<std::chrono::steady_clock> current_clock =
        std::chrono::steady_clock::now();
    current.user += static_cast<FCL_REAL>(
                        std::chrono::duration_cast<std::chrono::nanoseconds>(
                            current_clock - m_start)
                            .count()) *
                    FCL_REAL(1e-3);
#endif
    return current;
  }

#ifdef HPP_FCL_WITH_CXX11_SUPPORT
  duration_type duration() const { return (m_end - m_start); }
#endif

  void start() {
    if (m_is_stopped) {
      m_is_stopped = false;
      m_times.clear();

#ifdef HPP_FCL_WITH_CXX11_SUPPORT
      m_start = std::chrono::steady_clock::now();
#endif
    }
  }

  void stop() {
    if (m_is_stopped)
      return;
    m_is_stopped = true;

#ifdef HPP_FCL_WITH_CXX11_SUPPORT
    m_end = std::chrono::steady_clock::now();
    m_times.user += static_cast<FCL_REAL>(
                        std::chrono::duration_cast<std::chrono::nanoseconds>(
                            m_end - m_start)
                            .count()) *
                    FCL_REAL(1e-3);
#endif
  }

  void resume() {
#ifdef HPP_FCL_WITH_CXX11_SUPPORT
    if (m_is_stopped) {
      m_start = std::chrono::steady_clock::now();
      m_is_stopped = false;
    }
#endif
  }

  bool is_stopped() const { return m_is_stopped; }

protected:
  CPUTimes m_times;
  bool m_is_stopped;

#ifdef HPP_FCL_WITH_CXX11_SUPPORT
  std::chrono::time_point<std::chrono::steady_clock> m_start, m_end;
#endif
};

} // namespace fcl
} // namespace hpp

#endif // ifndef HPP_FCL_TIMINGS_FWD_H
