#pragma once

#define LOG_TRACE(...) \
  if (spdlog::should_log(spdlog::level::trace)) { \
    Logger()->trace(__VA_ARGS__); \
  }

#define LOG_DEBUG(...) \
  if (spdlog::should_log(spdlog::level::debug)) { \
    Logger()->debug(__VA_ARGS__); \
  }

#define LOG_INFO(...) \
  if (spdlog::should_log(spdlog::level::info)) { \
    Logger()->info(__VA_ARGS__); \
  }

#define LOG_WARNING(...) \
  if (spdlog::should_log(spdlog::level::warn)) { \
    Logger()->warn(__VA_ARGS__); \
  }

#define LOG_ERROR(...) \
  if (spdlog::should_log(spdlog::level::err)) { \
    Logger()->error(__VA_ARGS__); \
  }

#define LOG_CRITICAL(...) \
  if (spdlog::should_log(spdlog::level::critical)) { \
    Logger()->critical(__VA_ARGS__); \
  }

namespace mdo {

std::shared_ptr<spdlog::logger> Logger() noexcept;
std::error_code EnableConsoleLogging() noexcept;
std::error_code DisableConsoleLogging() noexcept;

}// namespace mdo
