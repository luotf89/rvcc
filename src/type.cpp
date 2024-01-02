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

const char* Type::kindName() {
  return kind_names[static_cast<int>(kind_)];
}

}