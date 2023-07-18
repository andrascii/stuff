#pragma once

#include <chrono>
#include <condition_variable>
#include <csignal>
#include <cstdlib>
#include <future>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <set>
#include <sstream>
#include <thread>
#include <utility>
#include <functional>
#include <filesystem>
#include <fstream>

//
// Spdlog
//

#pragma warning(push)
#pragma warning(disable : 4005)

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#include <spdlog/fmt/ostr.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#pragma warning(pop)

#if !defined(_WIN32)
#include <sys/event.h>
#include <sys/types.h>
#include <unistd.h>
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

#include "logger.h"

using namespace std::literals;