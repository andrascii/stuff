#pragma once

#include "not_null.h"

namespace mdo {

class Object;

class ObjectsRegistry {
 public:
  static ObjectsRegistry& Instance() {
    struct NewOpEnabler : ObjectsRegistry {
      NewOpEnabler() : ObjectsRegistry() {}
    };

    static std::unique_ptr<ObjectsRegistry> instance = std::make_unique<NewOpEnabler>();

    return *instance;
  }

  ObjectsRegistry(ObjectsRegistry&& other) = delete;
  ObjectsRegistry(const ObjectsRegistry& other) = delete;

  virtual ~ObjectsRegistry() {
    if (!objects_.empty()) {
      //Logger()->error("ObjectsRegistry destroys but some Objects is still alive");
    }
  }

  const std::set<Object*>& Objects() const noexcept {
    std::scoped_lock _{*this};
    return objects_;
  }

  void RegisterObject(NotNull<Object*> object) {
    std::scoped_lock _{*this};
    objects_.insert(object);
  }

  void UnregisterObject(NotNull<Object*> object) {
    std::scoped_lock _{*this};
    objects_.erase(object);
  }

  bool HasObject(NotNull<Object*> object) {
    std::scoped_lock _{*this};
    return objects_.contains(object);
  }

  void lock() const noexcept {
    mutex_.lock();
  }

  void unlock() const noexcept {
    mutex_.unlock();
  }

 private:
  ObjectsRegistry() = default;

 private:
  mutable std::recursive_mutex mutex_;
  std::set<Object*> objects_;
};

}// namespace mdo