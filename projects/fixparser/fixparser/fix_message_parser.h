#pragma once

#include <system_error>
#include <tl/expected.hpp>

#include "fix_message.h"

template <typename T, typename E = std::error_code>
using Expected = tl::expected<T, E>;

template <typename E = std::error_code>
using Unexpected = tl::unexpected<E>;

using Milliseconds = std::chrono::milliseconds;

template <typename T>
using TimePoint = std::chrono::time_point<std::chrono::system_clock, T>;

class FixMessageParser {
 public:
  /*
   * Header:
   *  MsgType => SenderCompID => TargetCompID => MsgSeqNum => SendingTime
   *
   * Logon:
   *  HeartBtInt
   *
   * Logout:
   *  Just parse header
   *
   * Heartbeat/TestRequest
   *  TestReqID (required in heartbeat message if it's a response on to TestRequest)
   *
   * MarketDataRequest:
   *  MDReqID => SubscriptionRequestType => NoRelatedSym => Instrument
   *    Instrument:
   *      Symbol => SecurityType
   *
   * MarketDataRequestReject:
   *  MDReqID
   *
   * MarketDataSnapshotFullRefresh:
   *  MDReqID => NoMDEntries => MDEntryType => MDEntryPx => Currency => MDEntrySize
   * */

  static Expected<FixMessage> Parse(std::string_view fix_message);
  static Expected<FixMessage> Parse(const std::string& fix_message);

 private:
  static Expected<FixMessageHeader> ParseFixHeader(hffix::message_reader& reader, hffix::message_reader::const_iterator& hint);

  static Expected<Logon> OnLogon(hffix::message_reader& reader);
  static Expected<Logout> OnLogout(hffix::message_reader& reader);
  static Expected<Heartbeat> OnHeartbeat(hffix::message_reader& reader);
  static Expected<TestRequest> OnTestRequest(hffix::message_reader& reader);
  static Expected<Reject> OnReject(hffix::message_reader& reader);
  static Expected<MarketDataRequest> OnMarketDataRequest(hffix::message_reader& reader);
  static Expected<MarketDataRequestReject> OnMarketDataRequestReject(hffix::message_reader& reader);
  static Expected<MarketDataSnapshotFullRefresh> OnMarketDataSnapshotFullRefresh(hffix::message_reader& reader);

  static Expected<std::string> Symbol(hffix::message_reader& reader, hffix::message_reader::const_iterator& hint);
  static Expected<std::string> Account(hffix::message_reader& reader, hffix::message_reader::const_iterator& hint);
  static Expected<std::string> Sender(hffix::message_reader& reader, hffix::message_reader::const_iterator& hint);
  static Expected<std::string> Receiver(hffix::message_reader& reader, hffix::message_reader::const_iterator& hint);
  static Expected<uint64_t> HeartbeatInterval(hffix::message_reader& reader, hffix::message_reader::const_iterator& hint);
  static Expected<std::string> SecurityType(hffix::message_reader& reader, hffix::message_reader::const_iterator& hint);
  static Expected<std::string> SecurityGroup(hffix::message_reader& reader, hffix::message_reader::const_iterator& hint);
  static Expected<uint64_t> MsgSeqNum(hffix::message_reader& reader, hffix::message_reader::const_iterator& hint);
  static Expected<TimePoint<Milliseconds>> SendingTime(hffix::message_reader& reader, hffix::message_reader::const_iterator& hint);
  static Expected<TimePoint<Milliseconds>> LastUpdateTime(hffix::message_reader& reader, hffix::message_reader::const_iterator& hint);
  static Expected<uint64_t> NoMdEntries(hffix::message_reader& reader, hffix::message_reader::const_iterator& hint);
  static Expected<std::string> Tenor(hffix::message_reader& reader, hffix::message_reader::const_iterator& hint);
  static Expected<std::string> SettlementDate(hffix::message_reader& reader, hffix::message_reader::const_iterator& hint);
  static Expected<std::string> MdReqId(hffix::message_reader& reader, hffix::message_reader::const_iterator& hint);
  static Expected<char> MdEntryType(hffix::message_reader& reader, hffix::message_reader::const_iterator& hint);
  static Expected<std::string> MdEntryPx(hffix::message_reader& reader, hffix::message_reader::const_iterator& hint);
  static Expected<uint64_t> MdEntrySize(hffix::message_reader& reader, hffix::message_reader::const_iterator& hint);
  static Expected<std::string> SubscriptionRequestType(hffix::message_reader& reader, hffix::message_reader::const_iterator& hint);
  static Expected<uint64_t> NoRelatedSym(hffix::message_reader& reader, hffix::message_reader::const_iterator& hint);
};