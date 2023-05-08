#pragma once

namespace mdo {

class Object;

class ObjectsRegistry final {
 public:
  static ObjectsRegistry& Instance() {
    static std::unique_ptr<ObjectsRegistry> instance = nullptr;

    if (!instance) {
      instance.reset(new ObjectsRegistry);
    }

    return *instance;
  }

  ObjectsRegistry(ObjectsRegistry&& other) = delete;
  ObjectsRegistry(const ObjectsRegistry& other) = delete;

  ~ObjectsRegistry() {
    if (!objects_.empty()) {
      std::cerr << "ObjectsRegistry destroys but some AbstractObjects is still alive" << std::endl;
    }
  }

  const std::set<Object*>& Objects() const noexcept {
    return objects_;
  }

  void RegisterObject(Object* object) {
    std::scoped_lock _{*this};
    objects_.insert(object);
  }

  void UnregisterObject(Object* object) {
    std::scoped_lock _{*this};
    objects_.erase(object);
  }

  bool HasObject(Object* object) {
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
  mutable std::mutex mutex_;
  std::set<Object*> objects_;
};

}