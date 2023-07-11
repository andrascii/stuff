#pragma once

#include "dispatcher.h"
#include "invoke_slot_message.h"
#include "not_null.h"
#include "object.h"
#include "utils.h"

namespace mdo {

template <typename... Args>
class Signal final {
 public:
  using FunctionSlot = void (*)(Args...);

  template <typename ObjectType>
  using MethodSlot = void (ObjectType::*)(Args...);

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
      if (Utils::CurrentThread() == object->Thread()) {
        std::invoke(slot, object, std::forward<Args>(args)...);
      } else {
        Dispatcher::Dispatch(std::make_shared<InvokeSlotMessage>([=] {
          std::invoke(slot, object, args...);// std::forward<Args>(args)...
        }, owner_, object));
      }
    };

    slots_.push_back(wrapper);
  }

  void Connect(FunctionSlot slot) {
    slots_.emplace_back(slot);
  }

  void DisconnectAll() noexcept {
    slots_.clear();
  }

 private:
  Object* owner_;
  std::vector<Slot> slots_;
};

template <>
class Signal<void> {
 public:
  template <typename ObjectType>
  using MethodSlot = void (ObjectType::*)();

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
      if (Utils::CurrentThread() == object->Thread()) {
        std::invoke(slot, object);
      } else {
        Dispatcher::Dispatch(std::make_shared<InvokeSlotMessage>([=] {
          std::invoke(slot, object);
        }, owner_, object));
      }
    };

    slots_.push_back(wrapper);
  }

  void Connect(Slot slot) {
    slots_.emplace_back(slot);
  }

  void DisconnectAll() noexcept {
    slots_.clear();
  }

 private:
  Object* owner_;
  std::vector<Slot> slots_;
};

}// namespace mdo