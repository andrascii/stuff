#include <iostream>
#include <iterator>
#include <vector>
#include <filesystem>
#include <fstream>

#include "fix_message.h"
#include "fix_message_parser.h"

using namespace std::chrono;

std::vector<std::string> ReadFile(const std::filesystem::path& path) {
  std::string line;
  std::vector<std::string> result;

  std::ifstream in{path.generic_string()};

  if (!in.is_open()) {
    std::cerr << "can't open file '" << path.generic_string() << ";\n";
    return {};
  }

  while(std::getline(in, line)) {
    result.push_back(line);
  }

  return result;
}

int main(int argc, char** argv) {
  if (argc <= 1) {
    std::cerr << "fixparser path-to-file\n";
    return EXIT_FAILURE;
  }

  const auto lines = ReadFile(argv[1]);

  for (const auto& line : lines) {
    const auto message = FixMessageParser::Parse(line);

    if (!message) {
      std::cerr << message.error().message() << std::endl;
      return EXIT_FAILURE;
    }

    std::visit([](auto&& v) {
      using T = std::decay_t<decltype(v)>;

      if constexpr (std::is_same_v<T, Logon>) {
        std::cout
          << "Logon message:" << std::endl
          << "SenderCompID  = " << v.sender << std::endl
          << "TargetCompID  = " << v.receiver << std::endl
          << "SendingTime  = " << v.sending_time << std::endl
          << "HeartBtInt  = " << v.heartbeat_interval << std::endl
          << "MsgSeqNum  = " << v.seq_num << std::endl << std::endl;
      } else if constexpr (std::is_same_v<T, Logout>) {
        std::cout
          << "Logout message:" << std::endl
          << "SenderCompID  = " << v.sender << std::endl
          << "TargetCompID  = " << v.receiver << std::endl
          << "SendingTime  = " << v.sending_time << std::endl
          << "MsgSeqNum  = " << v.seq_num << std::endl << std::endl;
      } else if constexpr (std::is_same_v<T, Heartbeat>) {
        std::cout
          << "Heartbeat message:" << std::endl
          << "SenderCompID  = " << v.sender << std::endl
          << "TargetCompID  = " << v.receiver << std::endl
          << "SendingTime  = " << v.sending_time << std::endl
          << "MsgSeqNum  = " << v.seq_num << std::endl << std::endl;
      } else if constexpr (std::is_same_v<T, TestRequest>) {
        std::cout
          << "TestRequest message:" << std::endl
          << "SenderCompID  = " << v.sender << std::endl
          << "TargetCompID  = " << v.receiver << std::endl
          << "SendingTime  = " << v.sending_time << std::endl
          << "MsgSeqNum  = " << v.seq_num << std::endl << std::endl;
      } else if constexpr (std::is_same_v<T, MarketDataRequest>) {
        std::cout
          << "MarketDataRequest message:" << std::endl
          << "SenderCompID  = " << v.sender << std::endl
          << "TargetCompID  = " << v.receiver << std::endl
          << "SendingTime  = " << v.sending_time << std::endl
          << "MsgSeqNum  = " << v.seq_num << std::endl
          << "MDReqID  = " << v.md_req_id << std::endl
          << "SubscriptionRequestType  = " << v.subscription_request_type << std::endl << std::endl;

        std::cout << "  Instruments: " << std::endl;

        for (const auto& instrument : v.instruments) {
          std::cout
            << "    Symbol  = " << instrument.symbol << std::endl
            << "    Security Type  = " << instrument.security_type << std::endl
            << "    Security Group  = " << instrument.security_group << std::endl
            << "    Accout  = " << instrument.account << std::endl
            << "    Tenor  = " << instrument.tenor << std::endl
            << "    Settlement Date  = " << instrument.settlement_date << std::endl;
        }

        std::cout << std::endl;
      } else if constexpr (std::is_same_v<T, MarketDataRequestReject>) {
        std::cout
          << "MarketDataRequestReject message:" << std::endl
          << "SenderCompID  = " << v.sender << std::endl
          << "TargetCompID  = " << v.receiver << std::endl
          << "SendingTime  = " << v.sending_time << std::endl
          << "MsgSeqNum  = " << v.seq_num << std::endl
          << "MDReqID  = " << v.md_req_id << std::endl << std::endl;
      } else if constexpr (std::is_same_v<T, MarketDataSnapshotFullRefresh>) {
        std::cout
          << "MarketDataSnapshotFullRefresh message:" << std::endl
          << "Symbol  = " << v.symbol << std::endl
          << "SenderCompID  = " << v.sender << std::endl
          << "TargetCompID  = " << v.receiver << std::endl
          << "SecurityType  = " << v.security_type << std::endl
          << "SecurityGroup  = " << v.security_group << std::endl
          << "Account  = " << v.account << std::endl
          << "Tenor  = " << v.tenor << std::endl
          << "SettlementDate  = " << v.settlement_date << std::endl
          << "MDReqID  = " << v.md_req_id << std::endl
          << "SendingTime  = " << v.sending_time << std::endl
          << "LastUpdateTime  = " << v.sending_time << std::endl
          << "MsgSeqNum  = " << v.seq_num << std::endl << std::endl;

        for (const auto& level : v.levels) {
          std::cout
            << (level.type == OrderBookLevelType::kBid ? "  Bid" : "  Offer") << std::endl
            << "    Price  = " << level.price << std::endl
            << "    Volume  = " << level.volume << std::endl << std::endl;
        }
      }
    }, *message);
  }
}
