#ifndef __TYPE_H
#define __TYPE_H

namespace rvcc {

enum class TypeKind:int{
  TYPE_INT = 0,
  TYPE_PTR,
  TYPE_ILLEGAL,
  TYPE_COUNT
};

class Type {
  public:
    Type(TypeKind kind=TypeKind::TYPE_ILLEGAL, Type* base=nullptr);
    TypeKind& kind();
    Type*& base();
    bool operator== (Type other);
    const char* kindName();
    static Type* typeInt;
  private:
    TypeKind kind_;
    Type* base_;
    static const char* kind_names[static_cast<int>(TypeKind::TYPE_COUNT)];
};


}

#endif