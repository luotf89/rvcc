#ifndef __OBJECT_H
#define __OBJECT_H

namespace rvcc {
/*
方便内存管理，参与申请和释放的类都继承自 Object类
*/
class Object {
  public:
    Object() = default;
    virtual ~Object() {}
};


} // namespace rvcc

#endif