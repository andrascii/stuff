#pragma once

class AbstractObject;

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

  void RegisterObject(AbstractObject* object) {
    std::lock_guard _{mutex_};
    objects_.insert(object);
  }

  void UnregisterObject(AbstractObject* object) {
    std::lock_guard _{mutex_};
    objects_.erase(object);
  }

  bool HasObject(AbstractObject* object) {
    std::lock_guard _{mutex_};
    return objects_.contains(object);
  }

 private:
  ObjectsRegistry() = default;

 private:
  mutable std::mutex mutex_;
  std::set<AbstractObject*> objects_;
};