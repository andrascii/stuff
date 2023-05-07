#pragma once

#include "object.h"
#include "not_null.h"
#include "dispatcher.h"
#include "invoke_slot_message.h"

namespace message_driven_objects {

template <typename ... Args>
class Signal final {
 public:
  using FunctionSlot = void(*)(Args...);

  template <typename ObjectType>
  using MethodSlot = void(ObjectType::*)(Args...);

  using Slot = std::function<void(Args...)>;

  explicit Signal(NotNull<Object*> owner) : owner_{owner} {}

  void operator()(Args&&... args) {
    for (const auto& slot : slots_) {
      slot(std::forward<Args>(args)...);
    }
  }

  template <typename ObjectType>
  void Connect(ObjectType* object, MethodSlot<ObjectType> slot) {
    static_assert(std::is_base_of_v<Object, ObjectType>, "ObjectType must be derived from class Object");

    Slot wrapper = [=](Args&&... args) {
      if (owner_->Thread() == object->Thread()) {
        std::invoke(slot, object, std::forward<Args>(args)...);
      } else {
        Dispatcher::Dispatch(std::make_shared<InvokeSlotMessage>([&, object, slot] {
          std::invoke(slot, object, std::forward<Args>(args)...);
        }, owner_, object));
      }
    };

    slots_.push_back(wrapper);
  }

  void Connect(FunctionSlot slot) {
    slots_.emplace_back(slot);
  }

 private:
  Object* owner_;
  std::vector<Slot> slots_;
};

template <>
class Signal<void> {
 public:
  using FunctionSlot = void(*)();

  template <typename ObjectType>
  using MethodSlot = void(ObjectType::*)();

  using Slot = std::function<void()>;

  explicit Signal(NotNull<Object*> owner) : owner_{owner} {}

  void operator()() {
    for (const auto& slot : slots_) {
      slot();
    }
  }

  template <typename ObjectType>
  void Connect(ObjectType* object, MethodSlot<ObjectType> slot) {
    static_assert(std::is_base_of_v<Object, ObjectType>, "ObjectType must be derived from class Object");

    Slot wrapper = [=]() {
      if (owner_->Thread() == object->Thread()) {
        std::invoke(slot, object);
      } else {
        Dispatcher::Dispatch(std::make_shared<InvokeSlotMessage>([=] {
          std::invoke(slot, object);
        }, owner_, object));
      }
    };

    slots_.push_back(wrapper);
  }

  void Connect(FunctionSlot slot) {
    slots_.emplace_back(slot);
  }

 private:
  Object* owner_;
  std::vector<Slot> slots_;
};

}