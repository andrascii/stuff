#pragma once

#include "object.h"
#include "not_null.h"
#include "dispatcher.h"
#include "invoke_slot_message.h"

namespace message_driven_objects {

template <typename ... Args>
class Signal {
 public:
  using Slot = std::function<void(Args...)>;

  explicit Signal(NotNull<Object*> owner) : owner_{owner} {}

  /*using Method = void(Object::*)(Args...);
  using FunctionSlot = void(*)(Args...);*/

  void operator()(Args&&... args) {
    for (const auto& slot : slots_) {
      slot(std::forward<Args>(args)...);

      /*const auto visitor = [&](auto&& v){
        using T = std::decay_t<decltype(v)>;

        if constexpr (std::is_same_v<T, MethodSlot>) {
          if (owner_->Thread() == v.object->Thread()) {
            v.object->*v.method(std::forward<Args>(args)...);
          } else {
            const auto invoker = [=] {
              v.object->*v.method(std::forward<Args>(args)...);
            };

            Dispatcher::Dispatch(std::make_shared<InvokeSlotMessage>(invoker, owner_, v.object));
          }
        } else if constexpr (std::is_same_v<T, FunctionSlot>) {
          v(std::forward<Args>(args)...);
        } else {
          SPDLOG_CRITICAL("invalid state of the slot");
          std::terminate();
        }
      };*/
    }
  }

  template <typename ObjectType>
  void Connect(ObjectType* object, void(ObjectType::*method)(Args...)) {
    static_assert(std::is_base_of_v<Object, ObjectType>, "ObjectType must be derived from class Object");

    Slot slot = [=](Args&&... args) {

      if (owner_->Thread() == object->Thread()) {
        (object->*method)(std::forward<Args>(args)...);
      } else {
        Dispatcher::Dispatch(std::make_shared<InvokeSlotMessage>([=] {
          (object->*method)(args...);
        }, owner_, object));
      }
    };

    slots_.push_back(slot);
  }

  void Connect(void(*slot)(Args...)) {
    slots_.push_back(slot);
  }

 private:
  Object* owner_;
  std::vector<Slot> slots_;
};

}