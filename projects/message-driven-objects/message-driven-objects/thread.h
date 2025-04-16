#pragma once

#include "object.h"
#include "signal_impl.h"
#include "thread_data.h"

namespace mdo {

//!
//! WARN: do not use current_thread_data it's for internal use only!
//! Using this variable is very easy to make a mistake in your code!
//! Instead of it use a call GetThreadData(Thread::Current()) or
//! GetThreadData(thread).
//!
extern thread_local std::shared_ptr<ThreadData> current_thread_data;

class Thread : public Object {
 public:
  friend const std::shared_ptr<ThreadData>&
  GetThreadData(const Thread* thread) noexcept;

  //!
  //! This signal is emitted from the associated thread right before it
  //! finishes executing. When this signal is emitted, the event loop has
  //! already stopped running. No more events will be processed in the thread.
  //!
  Signal<void> Finished;

  //!
  //! This signal is emitted from the associated thread when it starts
  //! executing, before the Run() function is called.
  //!
  Signal<void> Started;

  //!
  //! Returns a pointer to a Thread which manages the currently executing
  //! thread.
  //!
  static Thread* Current();

  //!
  //! Yields execution of the current thread to another runnable thread, if
  //! any. Note that the operating system decides to which thread to switch.
  //!
  static void YieldThread();

  //!
  //! Forces the current thread to sleep for ms milliseconds.
  //!
  static void Sleep(const std::chrono::milliseconds& ms);

  //!
  //! Sets the currently executing thread a name.
  //!
  static void SetCurrentThreadName(const std::string& name) noexcept;

  //!
  //! Creates a new Thread object that will execute the function f with the
  //! arguments args. The new thread is not started – it must be started by an
  //! explicit call to Start(). This allows you to connect to its signals,
  //! move Objects to the thread and so on. The function f will be called in
  //! the new thread.
  //!
  //! Returns the newly created Thread instance.
  //!
  template <typename Function, typename... Args>
  static std::unique_ptr<Thread> Create(Function&& f, Args&&... args) {
    struct NewEnabler : Thread {
      explicit NewEnabler(std::function<void()> alternative_entry_point)
          : Thread(std::move(alternative_entry_point)) {}
    };

    const auto adopted_invoke = [=] { f(std::forward<Args>(args)...); };

    auto thread = std::make_unique<NewEnabler>(adopted_invoke);

    Initialize(*thread);

    return thread;
  }

  static std::unique_ptr<Thread> Create(const char* name = "");

  ~Thread() override;

  //!
  //! Returns a name assigned to the thread assigned with Thread object.
  //! Returns an empty string if name wasn't set before call this function.
  //!
  const std::string& Name() const noexcept;

  //!
  //! Sets a name that will be assigned to the thread when it would be
  //! started.
  //!
  void SetName(const std::string& name);

  //!
  //! Begins execution of the thread by calling Run().
  //! If the thread is already running, this function does nothing.
  //!
  virtual void Start();

  //!
  //! Blocks the thread until the thread function is finished.
  //!
  void Wait() const noexcept;

  //!
  //! Blocks the thread until either of these conditions is met:
  //!  - The thread associated with this Thread object has finished execution
  //!  (i.e. when it returns from Run()).
  //!    This function will return true if the thread has finished.
  //!    It also returns true if the thread has not been started yet.
  //!
  //!  - The timeout is reached. This function will return false if the
  //!  timeout is reached.
  //!
  bool WaitFor(const std::chrono::milliseconds& ms) const noexcept;

  //!
  //! Tells the thread's event loop to exit with return code 0 (success).
  //! This function does nothing if the thread does not have an event loop.
  //!
  //! Note: This function is thread-safe.
  //!
  virtual void Stop();

  //!
  //! Returns true if the thread is running; otherwise returns false.
  //!
  //! Note: This function is thread-safe.
  //!
  bool IsRunning() const noexcept;

  //!
  //! Request the interruption of the thread.
  //! That request is advisory and it is up to code running on the thread to
  //! decide if and how it should act upon such request. This function does
  //! not stop any event loop running on the thread and does not terminate it
  //! in any way.
  //!
  //! Note: This function is thread-safe.
  //!
  void RequestInterruption() const noexcept;

  //!
  //! Return true if the task running on this thread should be stopped. An
  //! interruption can be requested by RequestInterruption(). This function
  //! can be used to make long running tasks cleanly interruptible. Never
  //! checking or acting on the value returned by this function is safe,
  //! however it is advisable do so regularly in long running functions. Take
  //! care not to call it too often, to keep the overhead low.
  //!
  bool IsInterruptionRequested() const noexcept;

 protected:
  void Run();

  void HandleMessage(Message&& message);
  void HandleMessages(std::vector<Message>& messages);

  static std::string CurrentThreadId();

  explicit Thread(std::shared_ptr<ThreadData> data = nullptr);

 private:
  void StopImpl();

  explicit Thread(std::function<void()> alternative_entry_point,
                  std::shared_ptr<ThreadData> data = nullptr);

  static void Initialize(Thread& thread);

 private:
  std::shared_ptr<ThreadData> data_;
  std::future<void> future_;
  std::string name_;
  std::function<void()> alternative_entry_point_;
};

}// namespace mdo
