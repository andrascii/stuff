#include "message1_event.h"
#include "message2_event.h"
#include "object.h"
#include "thread.h"
#include "application.h"

std::ostream& Log() {
  return std::cout << std::this_thread::get_id() << ": ";
}

class Message1EventSpammer : public eo::Object {
 public:
  explicit Message1EventSpammer(Object* parent = nullptr) : Object{parent} {}

  ~Message1EventSpammer() {
    Log() << "Message1EventSpammer destructed\n";
  }

  void Broadcast() {
    Log() << "################ broadcasting Message1Event \"################\n";
    BroadcastEvent(std::make_shared<eo::Message1Event>());
  }
};

class Message2EventSpammer : public eo::Object {
 public:
  explicit Message2EventSpammer(eo::Object* parent = nullptr) : Object{parent} {}

  ~Message2EventSpammer() {
    Log() << "Message2EventSpammer destructed\n";
  }

  void Broadcast() {
    Log() << "################ broadcasting Message2Event ################\n";
    BroadcastEvent(std::make_shared<eo::Message2Event>());
  }
};

class Receiver : public eo::Object {
 public:
  explicit Receiver(eo::Object* parent = nullptr) : eo::Object{parent} {}

 protected:
  bool OnMessage1Event(const eo::Message1Event&) override {
    Log() << "**************** received Message1Event ****************\n";
    return true;
  }

  bool OnMessage2Event(const eo::Message2Event&) override {
    Log() << "**************** received Message2Event ****************\n";
    return true;
  }
};

int main() {
  /*Receiver receiver;

  auto s1 = new Message1EventSpammer{&receiver};
  auto s2 = new Message2EventSpammer{&receiver};

  s1->Broadcast();
  s2->Broadcast();*/

  eo::Application app;
  const std::error_code error = app.Exec();

  if (error) {
    std::cerr << error.message() << std::endl;
    return EXIT_FAILURE;
  }
}
