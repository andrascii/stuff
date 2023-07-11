#include "object.h"
#include "thread.h"
#include "dispatcher.h"
#include "timer_message.h"
#include "test_message.h"

using namespace mdo;

TEST(ObjectTests, ReceiveTimerMessage) {
  class A : public Object {
   public:
    A() : timer_id_{-1}, ticked_{} {
      Thread()->Started.Connect(this, &A::OnThreadStarted);
    }

    void OnThreadStarted() {
      LOG_TRACE("[tid: {}] A object received signal about attached thread start", Thread()->Name());
      timer_id_ = StartTimer(1s);
    }

    bool IsTicked() const {
      return ticked_;
    }

   protected:
    bool OnTimerMessage(TimerMessage& msg) override {
      EXPECT_EQ(Thread(), current_thread_data->Thread());

      if (timer_id_ == msg.Id()) {
        LOG_TRACE("[tid: {}] achieved timer tick", Thread()->Name());

        KillTimer(msg.Id());

        ticked_ = true;
      } else {
        LOG_ERROR(
          "[tid: {}] achieved TimerMessage id '{}' ' is not equal to the timer id we start '{}'",
          msg.Id(),
          timer_id_,
          Thread()->Name());
      }

      return ticked_;
    }

   private:
    int timer_id_;
    bool ticked_;
  };

  std::shared_ptr<A> a = std::make_shared<A>();

  auto future = std::async(std::launch::async, [] {
    Thread::Sleep(2s);
    Dispatcher::Quit();
  });

  Dispatcher::Instance().Exec();

  future.get();

  EXPECT_TRUE(a->IsTicked());
}

TEST(ObjectTests, SignalToFunctionSlot) {
  bool slot_was_called = false;

  const auto slot = [&slot_was_called] {
    LOG_TRACE("slot was called");
    slot_was_called = true;
  };

  class A : public Object {
   public:
    A() : TestSignal{this} {}

   Signal<void> TestSignal;
  };

  std::shared_ptr<A> a = std::make_shared<A>();

  a->TestSignal.Connect(slot);
  a->TestSignal();

  EXPECT_TRUE(slot_was_called);
}

TEST(ObjectTests, SignalToMethodSlotInSingleThread) {
  class A : public Object {
   public:
    A() : TestSignal{this} {
      Thread()->Started.Connect(this, &A::OnThreadStarted);
    }

    void OnThreadStarted() {
      LOG_TRACE("[tid: {}] A object received signal about attached thread start", Thread()->Name());
      TestSignal();
    }

    Signal<void> TestSignal;
  };

  class B : public Object {
   public:
    B() : slot_was_called_{} {}

    void Slot() noexcept {
      LOG_TRACE("[tid: {}] slot was called", Thread()->Name());
      slot_was_called_ = true;
    }

    bool SlotWasCalled() const noexcept {
      return slot_was_called_;
    }

   private:
    bool slot_was_called_;
  };

  const auto a = std::make_shared<A>();
  const auto b = std::make_shared<B>();

  a->TestSignal.Connect(b.get(), &B::Slot);

  auto future = std::async(std::launch::async, [] {
    Thread::Sleep(100ms);
    Dispatcher::Quit();
  });

  Dispatcher::Instance().Exec();

  future.get();

  EXPECT_TRUE(b->SlotWasCalled());
}

TEST(ObjectTests, SignalToMethodSlotInSecondThread) {
  class A : public Object {
   public:
    A() : TestSignal{this} {
      Thread()->Started.Connect(this, &A::OnThreadStarted);
    }

    void OnThreadStarted() {
      LOG_TRACE("[tid: {}] A object received signal about attached thread start", Thread()->Name());
      TestSignal();
    }

    Signal<void> TestSignal;
  };

  class B : public Object {
   public:
    explicit B(const std::shared_ptr<mdo::Thread>& thread)
        : Object{thread},
          slot_was_called_{} {}

    void Slot() noexcept {
      LOG_TRACE("[tid: {}] slot was called", Thread()->Name());
      slot_was_called_ = true;
    }

    bool SlotWasCalled() const noexcept {
      return slot_was_called_;
    }

   private:
    bool slot_was_called_;
  };

  const auto thread = Thread::Create("background");
  const auto a = std::make_shared<A>();
  const auto b = std::make_shared<B>(thread);

  a->TestSignal.Connect(b.get(), &B::Slot);

  auto future = std::async(std::launch::async, [] {
    Thread::Sleep(100ms);
    Dispatcher::Quit();
  });

  thread->Start();
  Dispatcher::Instance().Exec();

  future.get();

  EXPECT_TRUE(b->SlotWasCalled());
}

TEST(ObjectTests, SignalToMethodSlotCallsSequence) {
  class A : public Object {
   public:
    A() : TestSignal{this} {
      Thread()->Started.Connect(this, &A::OnThreadStarted);
    }

    void OnThreadStarted() {
      LOG_TRACE("[tid: {}] A object received signal about attached thread start", Thread()->Name());

      TestSignal("Hello, ");
      TestSignal("World! ");
      TestSignal("One, ");
      TestSignal("Two, ");
      TestSignal("Three, ");
      TestSignal("Four, ");
      TestSignal("Five.");
    }

    Signal<const std::string&> TestSignal;
  };

  class B : public Object {
   public:
    void Slot(const std::string& s) {
      LOG_TRACE("[tid: {}] slot was called", Thread()->Name());
      cumulative_ += s;
    }

    const std::string& Cumulative() const noexcept {
      return cumulative_;
    }

   private:
    std::string cumulative_;
  };

  const auto a = std::make_shared<A>();
  const auto b = std::make_shared<B>();

  a->TestSignal.Connect(b.get(), &B::Slot);

  auto future = std::async(std::launch::async, [] {
    Thread::Sleep(100ms);
    Dispatcher::Quit();
  });

  Dispatcher::Instance().Exec();

  future.get();

  EXPECT_EQ(b->Cumulative(), "Hello, World! One, Two, Three, Four, Five.");
}

TEST(ObjectTests, ReceiveMessagesSequence) {
  class A : public Object {
   public:
    explicit A(Object* receiver)
        : receiver_{receiver} {
      Thread()->Started.Connect(this, &A::OnThreadStarted);
    }

    void OnThreadStarted() {
      LOG_TRACE("[tid: {}] A object received signal about attached thread start", Thread()->Name());
      Dispatcher::Dispatch(std::make_shared<TestMessage>("Hello, ", this, receiver_));
      Dispatcher::Dispatch(std::make_shared<TestMessage>("World! ", this, receiver_));
      Dispatcher::Dispatch(std::make_shared<TestMessage>("One, ", this, receiver_));
      Dispatcher::Dispatch(std::make_shared<TestMessage>("Two, ", this, receiver_));
      Dispatcher::Dispatch(std::make_shared<TestMessage>("Three, ", this, receiver_));
      Dispatcher::Dispatch(std::make_shared<TestMessage>("Four, ", this, receiver_));
      Dispatcher::Dispatch(std::make_shared<TestMessage>("Five.", this, receiver_));
    }

   private:
    Object* receiver_;
  };

  class B : public Object {
   public:
    const std::string& Cumulative() const noexcept {
      return cumulative_;
    }

   protected:
    bool OnTestMessage(TestMessage& message) override {
      LOG_TRACE("[tid: {}] OnTestMessage was called", Thread()->Name());
      cumulative_ += message.Data();
      return true;
    }

   private:
    std::string cumulative_;
  };

  const auto b = std::make_shared<B>();
  const auto a = std::make_shared<A>(b.get());

  auto future = std::async(std::launch::async, [] {
    Thread::Sleep(100ms);
    Dispatcher::Quit();
  });

  Dispatcher::Instance().Exec();

  future.get();

  EXPECT_EQ(b->Cumulative(), "Hello, World! One, Two, Three, Four, Five.");
}