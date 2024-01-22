#include "type.h"
#include "token.h"
#include <cstddef>
#include <iterator>
#include <vector>

namespace rvcc {

const char* Type::kind_names[static_cast<int>(TypeKind::TYPE_COUNT)] {
  "TYPE_INT",
  "TYPE_PTR",
  "TYPE_FUNC",
  "TYPE_ILLEGAL"
};

static Type typeInt_static(TypeKind::TYPE_INT);
Type* Type::typeInt = &typeInt_static;

Type::Type(TypeKind kind):kind_(kind) {}

TypeKind& Type::kind() {
  return kind_;
}

bool Type::equal(Type* other) {
  if (kind() != other->kind()) {
    return false;
  }
  return true;
}

const char* Type::kindName() {
  return kind_names[static_cast<int>(kind_)];
}

PtrType::PtrType(Type* base_type):
  Type(TypeKind::TYPE_PTR),
  base_type_(base_type) {}

Type*& PtrType::base_type() {
  return base_type_;
}

bool PtrType::equal(Type* other) {
  if (other->kind() == TypeKind::TYPE_PTR) {
    Type* base1 = base_type();
    Type* base2 = dynamic_cast<PtrType*>(other)->base_type();
    if(base1 && base2) {
      return base1->equal(base2);
    }
    return false;
  }
  return false;
}

PtrType::~PtrType() {
  freeImpl(base_type_);
}

void PtrType::freeImpl(Type* type) {
  if (type->kind() == TypeKind::TYPE_PTR) {
    freeImpl(dynamic_cast<PtrType*>(type)->base_type());
  }
  // 只有ptr type 是new 出来的  基础type 是静态的
  if (type->kind() == TypeKind::TYPE_PTR ||
      type->kind() == TypeKind::TYPE_FUNC) {
    delete type;
  } else {
    type->~Type();
  }
}

FuncType::FuncType():Type(TypeKind::TYPE_FUNC) {}

FuncType::~FuncType() {
  ret_type_->~Type();
  if (ret_type_->kind() == TypeKind::TYPE_PTR) {
    delete ret_type_;
  }
  for (std::size_t i = 0; i < parameter_types_.size(); i++) {
    parameter_types_[i]->~Type();
    if (parameter_types_[i]->kind() == TypeKind::TYPE_PTR ||
        parameter_types_[i]->kind() == TypeKind::TYPE_FUNC) {
      delete parameter_types_[i];
    }
  }
}

Type*& FuncType::ret_type() {
  return ret_type_;
}

std::vector<Type*>& FuncType::parameter_types() {
  return parameter_types_;
}

bool FuncType::equal(Type* other) {
  if (other->kind() == TypeKind::TYPE_FUNC) {
    FuncType* tmp = dynamic_cast<FuncType*>(other);
    bool flag = ret_type_->equal(tmp->ret_type());
    if (parameter_types().size() != tmp->parameter_types().size()) {
      for (std::size_t i = 0; i < parameter_types().size(); i++) {
        flag = flag && (parameter_types_[i]->equal(tmp->parameter_types_[i]));
      }
      return flag;
    }
    return false;
  }
  return false;
}
} // namespace rvcc