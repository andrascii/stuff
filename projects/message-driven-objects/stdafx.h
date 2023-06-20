#pragma once

#include <iostream>
#include <chrono>
#include <memory>
#include <mutex>
#include <thread>
#include <list>
#include <set>
#include <map>
#include <condition_variable>
#include <utility>
#include <queue>
#include <chrono>
#include <future>
#include <sstream>
#include <csignal>
#include <cstdlib>

//
// Spdlog
//

#pragma warning(push)
#pragma warning(disable: 4005)

  #define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
  #include <spdlog/fmt/ostr.h>
  #include <spdlog/logger.h>
  #include <spdlog/sinks/basic_file_sink.h>
  #include <spdlog/sinks/daily_file_sink.h>
  #include <spdlog/sinks/stdout_color_sinks.h>
  #include <spdlog/spdlog.h>

#pragma warning(pop)

#if !defined(_WIN32)
  #include <unistd.h>
  #include <sys/types.h>
  #include <sys/event.h>
#else
  #define USE_WINDOWS_SET_THREAD_NAME_HACK
  #include <windows.h>
#endif

template <typename T>
std::string ToString(const T& data) {
  std::stringstream ss;
  ss << data;
  return ss.str();
}

using namespace std::chrono_literals;