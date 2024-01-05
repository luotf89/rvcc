#include "type.h"
#include "token.h"

namespace rvcc {

const char* Type::kind_names[static_cast<int>(TypeKind::TYPE_COUNT)] {
  "TYPE_INT",
  "TYPE_PTR",
  "TYPE_ILLEGAL"
};

Type* Type::typeInt = new Type(TypeKind::TYPE_INT);

Type::Type(TypeKind kind, Type* base):kind_(kind), base_(base) {}

TypeKind& Type::kind() {
  return kind_;
}

Type*& Type::base() {
  return base_;
}

bool Type::operator== (Type other) {
  if (kind_ != other.kind_) {
    return false;
  } 
  Type* base1 = base_;
  Type* base2 = other.base_;
  while (base1 || base2) {
    if (!base1 || !base2) {
      return false;
    }
    if (base1->kind_ != base2->kind_) {
      return false;
    }
    base1 = base1->base_;
    base2 = base2->base_;
  }
  return true;
}

const char* Type::kindName() {
  return kind_names[static_cast<int>(kind_)];
}

}