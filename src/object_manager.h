#ifndef __OBJECT_MAMAGER_H
#define __OBJECT_MAMAGER_H
#include "object.h"
#include <utility>
#include <vector>

namespace rvcc {

class ObjectManager {
  public:
    static ObjectManager& getInst() {
      static ObjectManager inst;
      return inst;
    }
    template<typename T, typename ...Args>
    T* alloc_type(Args... args) {
      T* object = new T(std::forward<Args>(args)...);
      objects_.push_back(object);
      return object;
    }

    ~ObjectManager() {
      for(auto object: objects_) {
        delete object;
      }
    }

  private:
    ObjectManager() = default;
    ObjectManager(const ObjectManager&) = delete;
    ObjectManager& operator=(const ObjectManager&) = delete;
    ObjectManager(ObjectManager&&) = delete;
    ObjectManager& operator=(ObjectManager&&) = delete;
    std::vector<Object*> objects_;
};

} // namespace rvcc

#endif