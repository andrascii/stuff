#include "abstract_object.h"
#include "message1_event.h"
#include "message2_event.h"

std::ostream& Log() {
  return std::cout << std::this_thread::get_id() << ": ";
}

class Message1EventSpammer : public AbstractObject {
 public:
  void Start() {
    for (int i = 0; i < 10; ++i) {
      Log() << "################ broadcasting Message1Event \"################\n";
      BroadcastEvent(std::make_shared<Message1Event>());
      std::this_thread::sleep_for(1s);
    }
  }
};

class Message2EventSpammer : public AbstractObject {
 public:
  void Start() {
    for (int i = 0; i < 10; ++i) {
      Log() << "################ broadcasting Message2Event ################\n";
      BroadcastEvent(std::make_shared<Message2Event>());
      std::this_thread::sleep_for(900ms);
    }
  }
};

class Receiver : public AbstractObject {
 protected:
  bool OnMessage1Event(const Message1Event&) override {
    Log() << "**************** received Message1Event ****************\n";
    return true;
  }

  bool OnMessage2Event(const Message2Event&) override {
    Log() << "**************** received Message2Event ****************\n";
    return true;
  }
};

int main() {
  Receiver receiver;

  auto t1 = std::thread([&]{
    Message1EventSpammer spammer1;
    spammer1.ConnectEventReceiver(&receiver, IEvent::kMessage1);
    spammer1.Start();
  });

  auto t2 = std::thread([&]{
    Message2EventSpammer spammer2;
    spammer2.ConnectEventReceiver(&receiver, IEvent::kMessage2);
    spammer2.Start();
  });

  receiver.Exec();

  t1.join();
  t2.join();
}
