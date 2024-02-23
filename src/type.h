#ifndef __TYPE_H
#define __TYPE_H

#include "object.h"
#include "token.h"
#include <cstddef>
#include <vector>

namespace rvcc {

enum class TypeKind:int{
  TYPE_INT = 0,
  TYPE_PTR,
  TYPE_FUNC,
  TYPE_ARRAY,
  TYPE_ILLEGAL,
  TYPE_COUNT
};

class Type:public Object {
  public:
    Type(TypeKind kind=TypeKind::TYPE_ILLEGAL, std::size_t size=0);
    TypeKind& kind();
    virtual ~Type() {};
    std::size_t& size();
    const char* kindName();
    static Type* typeInt;
    virtual bool equal(Type* other);
  private:
    TypeKind kind_;
    std::size_t size_;
    static const char* kind_names[static_cast<int>(TypeKind::TYPE_COUNT)];
};

class PtrType: public Type {
  public:
    ~PtrType() override;
    PtrType(Type* base_type);
    Type*& base_type();
    virtual bool equal(Type* other) override;
  private:
    Type* base_type_;
};

class FuncType: public Type {
  public:
    FuncType(const char* name, std::size_t name_len);
    ~FuncType() override;
    Type*& ret_type();
    const char*& name();
    std::size_t& name_len();
    std::vector<Type*>& parameter_types();
    virtual bool equal(Type* other) override;
  private:
    Type* ret_type_;
    const char* name_;
    std::size_t name_len_;
    std::vector<Type*> parameter_types_;
};

class ArrayType: public Type {
  public:
    ArrayType(Type* base_type, std::size_t len);
    ~ArrayType() override;
    Type*& base_type();
    std::size_t& len();
    virtual bool equal(Type* other) override;
  private:
    Type* base_type_;
    std::size_t len_;
};


} // namespace rvcc

#endif