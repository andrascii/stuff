#pragma once

namespace eo {

std::shared_ptr<spdlog::logger> Logger() noexcept;
std::error_code EnableConsoleLogging() noexcept;
std::error_code DisableConsoleLogging() noexcept;

}
