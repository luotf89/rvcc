#include "type.h"
#include "object_manager.h"
#include <cstddef>
#include <vector>

namespace rvcc {

const char* Type::kind_names[static_cast<int>(TypeKind::TYPE_COUNT)] {
  "TYPE_INT",
  "TYPE_PTR",
  "TYPE_FUNC",
  "TYPE_ARRAY",
  "TYPE_ILLEGAL"
};

Type* Type::typeInt = ObjectManager::getInst().alloc_type<Type>(TypeKind::TYPE_INT, 8);

Type::Type(TypeKind kind, std::size_t size):kind_(kind), size_(size) {}

TypeKind& Type::kind() {
  return kind_;
}

std::size_t& Type::size() {
  return size_;
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
  Type(TypeKind::TYPE_PTR, 8),
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
  } else if (other->kind() == TypeKind::TYPE_ARRAY) {
    Type* base1 = base_type();
    Type* tmp = other;
    while (dynamic_cast<ArrayType*>(tmp)->base_type()->kind() == TypeKind::TYPE_ARRAY) {
      tmp = dynamic_cast<ArrayType*>(tmp)->base_type();
    }
    Type* base2 = dynamic_cast<ArrayType*>(tmp)->base_type();
    if(base1 && base2) {
      return base1->equal(base2);
    }
    return false;
  }
  return false;
}

PtrType::~PtrType() {}

FuncType::FuncType(const char* name, std::size_t name_len):
  Type(TypeKind::TYPE_FUNC, 8),
  name_(name), 
  name_len_(name_len) {}

FuncType::~FuncType() {}

Type*& FuncType::ret_type() {
  return ret_type_;
}

std::vector<Type*>& FuncType::parameter_types() {
  return parameter_types_;
}

const char*& FuncType::name() {
  return name_;
}

std::size_t& FuncType::name_len() {
  return name_len_;
}

bool FuncType::equal(Type* other) {
  if (other->kind() == TypeKind::TYPE_FUNC) {
    FuncType* tmp = dynamic_cast<FuncType*>(other);
    if (tmp->name_ != name_ || tmp->name_len_ != name_len_) {
      return false;
    }
    bool flag = ret_type_->equal(tmp->ret_type());
    if (!flag) {
      return false;
    }
    if (parameter_types().size() == tmp->parameter_types().size()) {
      for (std::size_t i = 0; i < parameter_types().size(); i++) {
        flag = parameter_types_[i]->equal(tmp->parameter_types_[i]);
        if (!flag) {
          return false;
        }
      }
      return true;
    }
    return false;
  }
  return false;
}

ArrayType::ArrayType(Type* base_type, std::size_t len):
  Type(TypeKind::TYPE_ARRAY, base_type->size()*len),
  base_type_(base_type),
  len_(len){}

ArrayType::~ArrayType() {}

Type*& ArrayType::base_type() {
  return base_type_;
}

std::size_t& ArrayType::len() {
  return len_;
}

bool ArrayType::equal(Type* other) {
  if (this == other) {
    return true;
  }
  if (other->kind() == kind() &&
      dynamic_cast<ArrayType*>(other)->len() == len()) {
    return true;
  }
  return false;
}
} // namespace rvcc