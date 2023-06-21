#pragma once

namespace mdo {

class IMessage;

class MessageQueue {
 public:
  MessageQueue();

  void Push(std::shared_ptr<IMessage> message);

  std::error_code Poll(
    std::shared_ptr<IMessage>& message,
    const std::chrono::seconds& timeout = 0s) noexcept;

  void SetInterruptFlag(bool value) noexcept;

 private:
  mutable std::mutex mutex_;
  std::condition_variable condition_;
  std::queue<std::shared_ptr<IMessage>> messages_;
  bool interrupt_;
};

}// namespace mdo