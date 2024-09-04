#include <date/date.h>
#include <date/tz.h>

#include <chrono>
#include <iostream>

template <typename T>
using TimePoint = std::chrono::time_point<std::chrono::system_clock, T>;

using Days = std::chrono::duration<uint64_t, std::ratio<86400>>;
using Seconds = std::chrono::seconds;
using Milliseconds = std::chrono::milliseconds;
using Microseconds = std::chrono::microseconds;
using ZonedTime = date::zoned_time<Seconds>;

TimePoint<Milliseconds> GmtStartOfTheCurrentDay() {
  const auto now = std::chrono::system_clock::now();
  const auto days = std::chrono::time_point_cast<Days>(now);
  return std::chrono::time_point_cast<Milliseconds>(days);
}

TimePoint<Milliseconds>
ConvertToTimeZone(
  const TimePoint<Milliseconds>& time_point,
  const std::string& time_zone) {
  const auto zoned_time_point = date::make_zoned(time_zone, time_point);
  return time_point + zoned_time_point.get_info().offset;
}

TimePoint<Milliseconds>
ZonedStartOfTheCurrentDay(const std::string& time_zone) {
  const auto gmt_start_of_the_current_day = GmtStartOfTheCurrentDay();
  const auto zoned_time_point =
    date::make_zoned(time_zone, gmt_start_of_the_current_day);
  return gmt_start_of_the_current_day - zoned_time_point.get_info().offset;
}

template <typename T>
static TimePoint<T> GmtToMsk(const TimePoint<T>& time_point) {
  return ConvertToTimeZone(time_point, "Europe/Moscow");
}

int main() {
  using namespace std::chrono;
  using namespace std::literals;

  std::string d = "20240731-21:00:00.000";

  TimePoint<Milliseconds> tp;
  std::istringstream ss{d};
  ss >> date::parse("%Y%m%d-%T", tp);

  const auto msk_tp = GmtToMsk(tp);

  std::cout
    << "original string: " << d << std::endl
    << "tp: " << tp << std::endl
    << "msk_tp: " << msk_tp << std::endl
    << std::endl;

  std::strstr("Hello, World!", "Hello");

  const auto start_of_the_day_current_day = GmtStartOfTheCurrentDay();

  const auto start_of_the_day_current_day_gmt_to_msk_converted =
    ConvertToTimeZone(start_of_the_day_current_day, "Europe/Moscow");

  const auto start_of_the_day_current_day_by_msk =
    ZonedStartOfTheCurrentDay("Europe/Moscow");

  const auto start_of_the_day_current_day_gmt_to_arg_converted =
    ConvertToTimeZone(start_of_the_day_current_day, "America/Argentina/ComodRivadavia");

  const auto start_of_the_day_current_day_by_arg =
    ZonedStartOfTheCurrentDay("America/Argentina/ComodRivadavia");

  std::cout
    << "start of the day by GMT: " << start_of_the_day_current_day
    << std::endl;

  std::cout
    << "start of the day by GMT (MSK time at this moment): "
    << start_of_the_day_current_day_gmt_to_msk_converted << std::endl;

  std::cout
    << "start of the day by MSK (MSK to GMT corrected): "
    << start_of_the_day_current_day_by_msk << std::endl;

  std::cout
    << "start of the day by MSK: "
    << ConvertToTimeZone(start_of_the_day_current_day_by_msk, "Europe/Moscow")
    << std::endl
    << std::endl;

  std::cout
    << "start of the day by GMT (Argentina time at this moment): "
    << start_of_the_day_current_day_gmt_to_arg_converted << std::endl;

  std::cout
    << "start of the day by GMT (Argentina to GMT corrected): "
    << start_of_the_day_current_day_by_arg << std::endl;

  std::cout
    << "start of the day GMT to Argentina corrected (GMT) zoned: "
    << ConvertToTimeZone(start_of_the_day_current_day_by_arg, "America/Argentina/ComodRivadavia")
    << std::endl;
}
