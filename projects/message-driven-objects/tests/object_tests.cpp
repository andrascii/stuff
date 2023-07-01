#include "object.h"
#include "thread.h"
#include "dispatcher.h"
#include "timer_message.h"

using namespace mdo;

TEST(ObjectTests, TimerTest) {
  class A : public Object {
   public:
    A() : timer_id_{-1}, ticked_{} {
      Thread()->Started.Connect(this, &A::OnThreadStarted);
    }

    void OnThreadStarted() {
      timer_id_ = StartTimer(1s);
    }

    bool IsTicked() const {
      return ticked_;
    }

   protected:
    bool OnTimerMessage(TimerMessage& msg) override {
      if (timer_id_ == msg.Id()) {
        std::cout << "achieved timer tick" << std::endl;

        KillTimer(msg.Id());

        ticked_ = true;
      } else {
        std::cerr
          << "achieved TimerMessage id '"
          << msg.Id()
          << "' is not equal to the timer id we start '"
          << timer_id_
          << std::endl;
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

TEST(ObjectTests, SignalFunctionSlotTest) {
  bool slot_was_called = false;

  const auto slot = [&slot_was_called] {
    std::cout << "slot was called\n";
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

TEST(ObjectTests, SignalMethodSlotSingleThreadTest) {
  class A : public Object {
   public:
    A() : TestSignal{this} {
      Thread()->Started.Connect(this, &A::OnThreadStarted);
    }

    void OnThreadStarted() {
      TestSignal();
    }

    Signal<void> TestSignal;
  };

  class B : public Object {
   public:
    B() : slot_was_called_{} {}

    void Slot() noexcept {
      std::cout << "slot was called\n";
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
    Thread::Sleep(3s);
    Dispatcher::Quit();
  });

  Dispatcher::Instance().Exec();

  future.get();

  EXPECT_TRUE(b->SlotWasCalled());
}

TEST(ObjectTests, SignalMethodSlotSecondThreadTest) {
  class First : public Object {
   public:
    explicit First(mdo::Thread* thread) : Object{thread}, TestSignal{this} {
      Thread()->Started.Connect(this, &First::OnThreadStarted);
    }

    void OnThreadStarted() {
      TestSignal();
    }

    Signal<void> TestSignal;
  };

  class Second : public Object {
   public:
    Second() : slot_was_called_{} {}

    void Slot() noexcept {
      std::cout << "slot was called\n";
      slot_was_called_ = true;
    }

    bool SlotWasCalled() const noexcept {
      return slot_was_called_;
    }

   private:
    bool slot_was_called_;
  };

  const auto thread = std::make_shared<Thread>();
  const auto a = std::make_shared<First>(thread.get());
  const auto b = std::make_shared<Second>();

  a->TestSignal.Connect(b.get(), &Second::Slot);

  auto future = std::async(std::launch::async, [] {
    Thread::Sleep(3s);
    Dispatcher::Quit();
  });

  thread->Start();
  Dispatcher::Instance().Exec();

  future.get();

  EXPECT_TRUE(b->SlotWasCalled());
}