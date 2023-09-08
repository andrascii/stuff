#pragma once

//
// C/C++
//
#include <atomic>
#include <cassert>
#include <chrono>
#include <condition_variable>
#include <csignal>
#include <cstdlib>
#include <deque>
#include <functional>
#include <future>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <set>
#include <string>
#include <thread>
#include <type_traits>
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

//
// Boost
//
#include <boost/algorithm/string.hpp>
#include <boost/asio.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/bind/placeholders.hpp>
#include <boost/config.hpp>
#include <boost/program_options.hpp>
#include <boost/program_options/cmdline.hpp>
#include <boost/program_options/option.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/positional_options.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/thread.hpp>

//
// Spdlog
//
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#include <spdlog/fmt/ostr.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

//
// Expected
//
#include <tl/expected.hpp>

//
// CppKafka
//
#if defined(__GNUC__)
#pragma GCC diagnostic push
#elif _MSC_VER
#pragma warning(push, 1)
#pragma warning(disable:4251)
#pragma warning(disable:4275)
#endif

#include <cppkafka/cppkafka.h>
#include <cppkafka/configuration.h>
#include <cppkafka/consumer.h>
#include <cppkafka/producer.h>

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#elif _MSC_VER
#pragma warning(pop)
#endif

//
// nlohmann
//
#include <nlohmann/json.hpp>

//
// mio/mmap
//
#include <mio/mmap.hpp>

//
// cxxopts
//
#include <cxxopts.hpp>

using namespace std::chrono_literals;
using namespace std::string_literals;
