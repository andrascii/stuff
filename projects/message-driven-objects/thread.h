#pragma once

#include "message_queue.h"
#include "object.h"
#include "thread_data.h"
#include "signal.h"

namespace message_driven_objects {

static thread_local ThreadDataPtr current_thread_data = nullptr;
extern std::atomic<Thread*> the_main_thread;

class Thread : public Object {
 public:
  friend ThreadDataPtr GetThreadData(Thread* thread) noexcept;

  //!
  //! This signal is emitted from the associated thread right before it finishes executing.
  //! When this signal is emitted, the event loop has already stopped running.
  //! No more events will be processed in the thread.
  //!
  Signal<void> Finished;

  //!
  //! This signal is emitted from the associated thread when it starts executing, before the Run() function is called.
  //!
  Signal<void> Started;

  //!
  //! Returns a pointer to a Thread which manages the currently executing thread.
  //!
  static Thread* Current();

  //!
  //! Yields execution of the current thread to another runnable thread, if any.
  //! Note that the operating system decides to which thread to switch.
  //!
  static void YieldCurrentThread();

  //!
  //! Forces the current thread to sleep for ms milliseconds.
  //!
  static void Sleep(const std::chrono::milliseconds& ms);

  //!
  //! Sets the currently executing thread a name.
  //!
  static void SetCurrentThreadName(const std::string& name) noexcept;

  //!
  //! Creates a new Thread object that will execute the function f with the arguments args.
  //! The new thread is not started – it must be started by an explicit call to Start().
  //! This allows you to connect to its signals, move Objects to the thread and so on.
  //! The function f will be called in the new thread.
  //!
  //! Returns the newly created Thread instance.
  //!
  template <typename ... Args>
  static Thread* Create(NotNull<void(*)(Thread*, Args...)> f, Args&&... args) {
    const auto adopted_invoke = [=] {
      f(std::forward<Args>(args)...);
    };

    return new Thread{adopted_invoke};
  }

  explicit Thread(ThreadDataPtr data = nullptr, Object* parent = nullptr);

  ~Thread() override;

  //!
  //! Returns a name assigned to the thread assigned with Thread object.
  //! Returns an empty string if name wasn't set before call this function.
  //!
  const std::string& Name() const noexcept;

  //!
  //! Sets a name that will be assigned to the thread when it would be started.
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
  //!  - The thread associated with this Thread object has finished execution (i.e. when it returns from Run()).
  //!    This function will return true if the thread has finished.
  //!    It also returns true if the thread has not been started yet.
  //!
  //!  - The timeout is reached. This function will return false if the timeout is reached.
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
  //! That request is advisory and it is up to code running on the thread to decide if and how it should act upon such request.
  //! This function does not stop any event loop running on the thread and does not terminate it in any way.
  //!
  //! Note: This function is thread-safe.
  //!
  void RequestInterruption() const noexcept;

  //!
  //! Return true if the task running on this thread should be stopped. An interruption can be requested by requestInterruption().
  //! This function can be used to make long running tasks cleanly interruptible.
  //! Never checking or acting on the value returned by this function is safe, however it is advisable do so regularly in long running functions.
  //! Take care not to call it too often, to keep the overhead low.
  //!
  bool IsInterruptionRequested() const noexcept;

 protected:
  static void Run();

 private:
  void StopImpl();

  void AddChild(Object* child) noexcept override;

  explicit Thread(
    std::function<void()> alternative_entry_point,
    ThreadDataPtr data = nullptr,
    Object* parent = nullptr
  );

 private:
  ThreadDataPtr data_;
  std::future<void> future_;
  std::string name_;
  std::function<void()> alternative_entry_point_;
};

}