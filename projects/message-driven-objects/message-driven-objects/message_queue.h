#pragma once

namespace mdo {

class IMessage;

class MessageQueue {
 public:
  MessageQueue();

  void Push(std::shared_ptr<IMessage> message);

  //!
  //! Extracts a message from queue and assigns 'message' argument to extracted value.
  //!
  //! Returns error code describing the result of calling, error code could be:
  //!     - std::errc::timed_out (if reached 'timeout' value).
  //!     - std::errc::interrupted (if SetInterruptFlag function was called with first parameter is true).
  //!     - no error (if all is ok, in this case 'message' argument would contain a pointer to extracted message).
  //!
  std::error_code Poll(
    std::deque<std::shared_ptr<IMessage>>& messages,
    const std::chrono::seconds& timeout = 0s) noexcept;

  void SetInterruptFlag(bool value) noexcept;

  void Clear() noexcept;

  size_t Size() const noexcept;

 private:
  mutable std::mutex mutex_;
  std::condition_variable condition_;
  std::deque<std::shared_ptr<IMessage>> messages_;
  bool interrupt_;
};

}// namespace mdo