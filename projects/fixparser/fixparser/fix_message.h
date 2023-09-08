#pragma once

using Milliseconds = std::chrono::milliseconds;

template <typename T>
using TimePoint = std::chrono::time_point<std::chrono::system_clock, T>;

enum class OrderBookLevelType {
  kBid,
  kOffer
};

struct OrderBookLevel {
  OrderBookLevelType type;
  uint64_t volume;
  std::string price;
};

struct FixMessageHeader {
  std::string sender;
  std::string receiver;
  TimePoint<Milliseconds> sending_time;
  uint64_t seq_num{1};
};

struct Logon : FixMessageHeader {
  uint64_t heartbeat_interval{};
};

struct Logout : FixMessageHeader {};
struct Heartbeat : FixMessageHeader {};
struct TestRequest : FixMessageHeader {};
struct Reject : FixMessageHeader {};

struct MarketDataRequest : FixMessageHeader {
  struct Instrument {
    std::string symbol;
    std::string security_type;
    std::string security_group;
    std::string tenor;
    std::string settlement_date;
    std::string account;
  };

  std::string md_req_id;
  std::string subscription_request_type;
  std::vector<Instrument> instruments;
};

struct MarketDataRequestReject : FixMessageHeader {
  std::string md_req_id;
};

struct MarketDataSnapshotFullRefresh : FixMessageHeader {
  std::string symbol;
  std::string security_type;
  std::string security_group;
  std::string account;
  std::string tenor;
  std::string settlement_date;
  std::string md_req_id;
  TimePoint<Milliseconds> last_update_time;
  std::vector<OrderBookLevel> levels;
};

using FixMessage = std::variant<
  Logon,
  Logout,
  Heartbeat,
  TestRequest,
  Reject,
  MarketDataRequest,
  MarketDataRequestReject,
  MarketDataSnapshotFullRefresh
>;