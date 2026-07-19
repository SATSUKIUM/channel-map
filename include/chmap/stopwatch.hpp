#ifndef CHANNELMAP_STOPWATCH_H_
#define CHANNELMAP_STOPWATCH_H_

#include <chrono>

#include "debug_print.hpp"

namespace chmap {

class Stopwatch {
public:
  using clock = std::chrono::high_resolution_clock;
  using time_point = std::chrono::time_point<clock>;
  // using nanoseconds = std::chrono::nanoseconds;
  // using milliseconds = std::chrono::milliseconds;
  // using seconds = std::chrono::seconds;

  Stopwatch() : m_running(false), m_start_time(), m_stop_time() {
    start();
  }

  ~Stopwatch() {
    stop();
  }

  void start() {
    m_start_time = clock::now();
    m_running = true;
  }

  void stop() {
    if (m_running) {
      m_stop_time = clock::now();
      m_running = false;
      elapsed();
    }
  }

  template<typename Duration=std::chrono::nanoseconds>
  double elapsed() const {
    auto duration = (m_running ? clock::now() : m_stop_time) - m_start_time;
    auto elapsed = std::chrono::duration_cast<Duration>(duration).count();
    // debug << elapsed // << " / " << Duration::period::den
    //       << std::endl;
    return elapsed;
  }

private:
  bool m_running;
  time_point m_start_time;
  time_point m_stop_time;
};

}

#endif
