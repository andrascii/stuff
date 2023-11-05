#pragma once

#include <string_view>
#include <system_error>
#include <tl/expected.hpp>
#include <unordered_map>

#include "fields.h"
#include "fix_message.h"

namespace my {

template <typename T, typename E = std::error_code>
using Expected = tl::expected<T, E>;

template <typename E = std::error_code>
using Unexpected = tl::unexpected<E>;

class MyFixParser {
 public:
  using MessageLabels = std::unordered_map<tag::Field, std::string_view>;

  MyFixParser();

  Expected<FixMessage> Parse(std::string_view fix_message);

 private:
  Expected<FixMessageHeader> ParseFixHeader(const MessageLabels& labels);
  Expected<Logon> OnLogon(const MessageLabels& labels);
  Expected<Logout> OnLogout(const MessageLabels& labels);
  Expected<Heartbeat> OnHeartbeat(const MessageLabels& labels);
  Expected<TestRequest> OnTestRequest(const MessageLabels& labels);
  Expected<Reject> OnReject(const MessageLabels& labels);
  Expected<MarketDataRequest>
  OnMarketDataRequest(const MessageLabels& labels);
  Expected<MarketDataRequestReject>
  OnMarketDataRequestReject(const MessageLabels& labels);
  Expected<MarketDataSnapshotFullRefresh>
  OnMarketDataSnapshotFullRefresh(const MessageLabels& labels);

 private:
  std::unordered_map<tag::Field, std::string_view> field_dictionary_;
  std::unordered_map<std::string_view, std::string_view>
    message_name_dictionary_;
};

}// namespace my