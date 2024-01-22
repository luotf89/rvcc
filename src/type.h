#ifndef __TYPE_H
#define __TYPE_H

#include <vector>

namespace rvcc {

enum class TypeKind:int{
  TYPE_INT = 0,
  TYPE_PTR,
  TYPE_FUNC,
  TYPE_ILLEGAL,
  TYPE_COUNT
};

class Type {
  public:
    Type(TypeKind kind=TypeKind::TYPE_ILLEGAL);
    TypeKind& kind();
    virtual ~Type() {};
    const char* kindName();
    static Type* typeInt;
    virtual bool equal(Type* other);
  private:
    TypeKind kind_;
    static const char* kind_names[static_cast<int>(TypeKind::TYPE_COUNT)];
};

class PtrType: public Type {
  public:
    ~PtrType() override;
    PtrType(Type* base_type);
    Type*& base_type();
    virtual bool equal(Type* other) override;
  private:
    void freeImpl(Type* type);
    Type* base_type_;
};

class FuncType: public Type {
  public:
    FuncType();
    ~FuncType() override;
    Type*& ret_type();
    std::vector<Type*>& parameter_types();
    virtual bool equal(Type* other) override;
  private:
    Type* ret_type_;
    std::vector<Type*> parameter_types_;
};


} // namespace rvcc

#endif