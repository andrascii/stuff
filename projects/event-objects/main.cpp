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

  void SendEvent(Object* receiver) {
    Log() << "################ sending event Message1Event \"################\n";
    GetThreadData(eo::Application::Instance().ThreadAffinity())->event_loop.Push(std::make_shared<eo::Message1Event>(this, receiver));
  }
};

class Message2EventSpammer : public eo::Object {
 public:
  explicit Message2EventSpammer(eo::Object* parent = nullptr) : Object{parent} {}

  ~Message2EventSpammer() {
    Log() << "Message2EventSpammer destructed\n";
  }

  void SendEvent(Object* receiver) {
    Log() << "################ sending event Message2Event ################\n";
    receiver->Event(std::make_shared<eo::Message2Event>(this, receiver));
  }

 protected:
  bool OnMessage2Event(const eo::Message2Event&) override {
    Log() << "**************** received Message2Event ****************\n";

    return true;
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

  eo::Application& app = eo::Application::Instance();
  Message1EventSpammer* spammer1 = new Message1EventSpammer(&app);
  //Message2EventSpammer* spammer2 = new Message2EventSpammer(&app);

  //spammer2->SendEvent(receiver);

  eo::Thread* thread = new eo::Thread;
  Receiver* receiver = new Receiver;
  receiver->MoveToThread(thread);
  thread->Start();

  spammer1->SendEvent(receiver);

  const std::error_code error = app.Exec();
  thread->Stop();

  if (error) {
    std::cerr << error.message() << std::endl;
    return EXIT_FAILURE;
  }
}
