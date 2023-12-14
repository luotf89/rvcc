#include "ast.h"
#include "logger.h"
#include "utils.h"
#include "instructions.h"
#include <cassert>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>


using namespace rvcc;

std::atomic_int Expr::g_id = 0;

const char* Expr::type_names[static_cast<int>(ExprType::NODE_COUNT)] {
  "NODE_ADD",
  "NODE_SUB",
  "NODE_MUL",
  "NODE_DIV",
  "NODE_NUM",
  "NODE_NEG",
  "NODE_EQ",
  "NODE_NE",
  "NODE_LT",
  "NODE_LE",
  "NODE_ID",
  "NODE_ASSIGN",
  "NODE_STMT",
  "Node_ILLEGAL"
};

Var::Var() {
  name_ = nullptr;
  len_ = 0;
  value_ = 0;
}

Var::Var(char* name, int len, int value): 
  name_(name), len_(len), value_(value) {}

int& Var::len() {
  return len_;
}

const char* Var::getName() {
  return name_;
}

int& Var::value() {
  return value_;
}

int& Var::offset() {
  return offset_;
}

Expr::Expr() {
  type_ = ExprType::NODE_ILLEGAL;
  id_ = g_id;
  g_id++;
}

Expr::Expr(ExprType type):type_(type) {
  id_ = g_id;
  g_id++;
}

int& Expr::id() {
  return id_;
}

ExprType& Expr::type() {
  return type_;
}

const char* Expr::getTypeName() const {
  return type_names[static_cast<int>(type_)];
}

BinaryExpr::BinaryExpr():Expr() {
  value_ = 0;
  left_ = nullptr;
  right_ = nullptr;
}

BinaryExpr::BinaryExpr(
  ExprType type, int value, 
  Expr* left, Expr* right): Expr(type) {
  value_ = value;
  left_ = left;
  right_ = right;
}

Expr* BinaryExpr::getNext() {
  return nullptr;
}

Expr* BinaryExpr::getLeft() {
  return left_;
}

Expr* BinaryExpr::getRight() {
  return right_;
}

int BinaryExpr::computer() {
  switch (type()) {
  case ExprType::NODE_ADD:
    value() = getLeft()->value() + getRight()->value();
    break;
  case ExprType::NODE_SUB:
    value() = getLeft()->value() - getRight()->value();
    break;
  case ExprType::NODE_MUL:
    value() = getLeft()->value() * getRight()->value();
    break;
  case ExprType::NODE_DIV:
    value() = getLeft()->value() / getRight()->value();
    break;
  case ExprType::NODE_EQ:
    value() = getLeft()->value() == getRight()->value();
    break;
  case ExprType::NODE_NE:
    value() = getLeft()->value() != getRight()->value();
    break;
  case ExprType::NODE_LT:
    value() = getLeft()->value() < getRight()->value();
    break;
  case ExprType::NODE_LE:
    value() = getLeft()->value() <= getRight()->value();
    break;
  case ExprType::NODE_ASSIGN:
    getLeft()->value() = getRight()->value();
    value() = getRight()->value();
    break;
  default:
    FATAL("binary expr cant support current type: %s", getTypeName());
  }
  return value();
}

void BinaryExpr::codegen() {
  int offset = 0;
  switch (type()) {
  case ExprType::NODE_ADD:
    pop_("a1");
    pop_("a0");
    add_("a0", "a0", "a1");
    push_("a0");
    break;
  case ExprType::NODE_SUB:
    pop_("a1");
    pop_("a0");
    sub_("a0", "a0", "a1");
    push_("a0");
    break;
  case ExprType::NODE_MUL:
    pop_("a1");
    pop_("a0");
    mul_("a0", "a0", "a1");
    push_("a0");
    break;
  case ExprType::NODE_DIV:
    pop_("a1");
    pop_("a0");
    div_("a0", "a0", "a1");
    push_("a0");
    break;
  case ExprType::NODE_EQ:
    pop_("a1");
    pop_("a0");
    xor_("a0", "a0", "a1");
    seqz_("a0", "a0");
    push_("a0");
    break;
  case ExprType::NODE_NE:
    pop_("a1");
    pop_("a0");
    xor_("a0", "a0", "a1");
    snez_("a0", "a0");
    push_("a0");
    break;
  case ExprType::NODE_LT:
    pop_("a1");
    pop_("a0");
    slt_("a0", "a0", "a1");
    push_("a0");
    break;
  case ExprType::NODE_LE:
    pop_("a1");
    pop_("a0");
    slt_("a0", "a1", "a0");
    xori_("a0", "a0", 1);
    push_("a0");
    break;
  case ExprType::NODE_ASSIGN:
    assert(getLeft()->type() == ExprType::NODE_ID);
    offset = -(dynamic_cast<IdentityExpr*>(getLeft())->var()->offset() + 1) * 8;
    pop_("a0");
    pop_("a1");
    sd_("a0", "fp", offset);
    push_("a0");
    break;
  default:
    FATAL("binary expr cant support current type: %s", getTypeName());
  }
}

int& BinaryExpr::value() {
  return value_;
}

void BinaryExpr::visualize(std::ostringstream& oss, int&ident_num) {
  ::ident(oss, ident_num);
  oss << id() << " [label=\"Node " << getTypeName() << "\"," << "color=black]\n";
  ::ident(oss, ident_num);
  oss << id() << " -> " << getLeft()->id() << "[color=black];\n";
  ::ident(oss, ident_num);
  oss << id() << " -> " << getRight()->id() << "[color=black];\n";
}

Expr*& BinaryExpr::left() {
  return left_;
}

Expr*& BinaryExpr::right() {
  return right_;
}

UnaryExpr::UnaryExpr():Expr() {
  value_ = 0;
  left_ = nullptr;
}

UnaryExpr::UnaryExpr(
  ExprType type, int value, 
  Expr* left): Expr(type) {
  value_ = value;
  left_ = left;
}

Expr* UnaryExpr::getNext() {
  return nullptr;
}

Expr* UnaryExpr::getLeft() {
  return left_;
}

Expr* UnaryExpr::getRight() {
  return nullptr;
}

int UnaryExpr::computer() {
  switch (type()) {
  case ExprType::NODE_NEG:
    value() = getLeft()->value();
    break;
  default:
    ERROR("unary expr cant support current type: %s", getTypeName());
  }
  return value();
}

void UnaryExpr::codegen() {
  switch (type()) {
  case ExprType::NODE_NEG:
    pop_("a0");
    neg_("a0", "a0");
    push_("a0");
    break;
  default:
    ERROR("unary expr cant support current type: %s", getTypeName());
  }
}

int& UnaryExpr::value() {
  return value_;
}

void UnaryExpr::visualize(std::ostringstream& oss, int& ident_num) {
  ::ident(oss, ident_num);
  oss << id() << " [label=\"Node " << getTypeName() << "\"," << "color=black]\n";
  ::ident(oss, ident_num);
  oss << id() << " -> " << getLeft()->id() << "[color=black];\n";
}

Expr*& UnaryExpr::left() {
  return left_;
}

NumExpr::NumExpr(int value):
  Expr(ExprType::NODE_NUM) {
  value_ = value;
}

Expr* NumExpr::getNext() {
  return nullptr;
}

Expr* NumExpr::getLeft() {
  return nullptr;
}

Expr* NumExpr::getRight() {
  return nullptr;
}

int NumExpr::computer() {
  return value();
}

void NumExpr::codegen() {
  li_("a0", value());
  push_("a0");
}

int& NumExpr::value() {
  return value_;
}

void NumExpr::visualize(std::ostringstream& oss, int& ident_num) {
  ::ident(oss, ident_num);
  oss << id() << " [label=\"Node " << getTypeName() << ": " << value() << "\"," << "color=yellow]\n";
}

IdentityExpr::IdentityExpr(Var* var):
  Expr(ExprType::NODE_ID) {
  var_ = var;
}

Expr* IdentityExpr::getNext() {
  return nullptr;
}

Expr* IdentityExpr::getLeft() {
  return nullptr;
}

Expr* IdentityExpr::getRight() {
  return nullptr;
}

Var*& IdentityExpr::var() {
  return var_;
}

int IdentityExpr::computer() {
  return value();
}

void IdentityExpr::codegen() {
  int  offset = -(var()->offset() + 1) * 8;
  ld_("a0", "fp", offset);
  push_("a0");
}

int& IdentityExpr::value() {
  return var_->value();
}

void IdentityExpr::visualize(std::ostringstream& oss, int& ident_num) {
  std::string name;
  for (int i = 0; i < var()->len(); i++) {
    name += *(var()->getName() + i);
  } 
  ::ident(oss, ident_num);
  oss << id() << " [label=\"Node " << getTypeName() << ": " << name << "\"," << "color=yellow]\n";
}

StmtExpr::StmtExpr(Expr* next, Expr* left):
  Expr(ExprType::NODE_STMT) {
  next_ = next;
  left_ = left;
}

Expr* StmtExpr::getNext() {
  return next_;
}

Expr* StmtExpr::getLeft() {
  return left_;
}

Expr* StmtExpr::getRight() {
  return nullptr;
}

Expr*& StmtExpr::next() {
  return next_;
}

Expr*& StmtExpr::left() {
  return left_;
}

int StmtExpr::computer() {
  return 0;
}

void StmtExpr::codegen() {}

int& StmtExpr::value() {
  return id();
}

void StmtExpr::visualize(std::ostringstream& oss, int& ident_num) {
  ::ident(oss, ident_num);
  oss << id() << " [label=\"Node " << getTypeName() << "\"," << "color=red]\n";
  ::ident(oss, ident_num);
  oss << id() << " -> " << getLeft()->id() << "[color=black];\n";
}

Function::Function() {
  body_ = nullptr;
  var_maps_.clear();
}

Expr*& Function::body() {
  return body_;
}

std::map<std::size_t, Var*>& Function::var_maps() {
  return var_maps_;
}

Function::Function(Expr* body, std::map<std::size_t,
  Var*>&& var_maps):body_(body), var_maps_(std::move(var_maps)) {}

void Function::freeNode(Expr* curr) {
  if (!curr) {
    return;
  }
  assert(typeid(Expr) == typeid(StmtExpr));
  if (curr->getNext()) {
    freeNode(curr->getNext());
  }
  walkImpl<WalkOrderType::POST_ORDER>([](Expr* curr_node) {
    delete curr_node;
  }, curr->getLeft());
  delete curr;
}

Function::~Function() {
  freeNode(body_);
}


void Function::getVarOffsets() {
  int idx = 0;
  for (auto& elem : var_maps_) {
    elem.second->offset() = idx;
    idx++;
  }
}

void Function::visualize(std::ostringstream& oss, int& ident_num) {
  auto func = [&](Expr* curr_node) {
    curr_node->visualize(oss, ident_num);
  };
  // reverse 遍历 显示的图是反着的

  // auto walkPost = [&](Expr* curr) {
  //   std::function<void(Expr*)> walkPostImpl;
  //   walkPostImpl = [&](Expr* curr_node) {
  //     if (!curr_node) {
  //       return;
  //     }
  //     walkPostImpl(curr_node->getNext());
  //     assert(curr_node->type() == ExprType::NODE_STMT);
  //     walkImpl<WalkOrderType::POST_ORDER>(func, curr_node->getLeft());
  //     curr_node->visualize(oss, ident_num);
  //     if (curr_node->getNext() != nullptr) {
  //       ident(oss, ident_num);
  //       oss << curr_node->id() << " -> " << curr_node->getNext()->id() << "[color=red];\n";
  //     }
  //   };
  //   walkPostImpl(curr);
  // };
  // walkPost(body_);

  Expr* curr = body();
  Expr* prev = nullptr;
  while(curr) {
    walkImpl<WalkOrderType::POST_ORDER>(func, curr->getLeft());
    curr->visualize(oss, ident_num);
    if (prev) {
      ident(oss, ident_num);
      oss << prev->id() << " -> " << curr->id() << "[color=red];\n";
    }
    prev = curr;
    curr = curr->getNext();
  };
}

int Function::computer() {
  auto func = [](Expr* curr_node) {
    curr_node->computer();
  };
  
  Expr* curr = body();
  Expr* prev = nullptr;
  while(curr) {
    walkImpl<WalkOrderType::POST_ORDER>(func, curr->getLeft());
    prev = curr;
    curr = curr->getNext();
  };
  if (!prev) {
    FATAL("curr stmt is empty, can't be computed");
  }
  return prev ? prev->getLeft()->value() : 0;
}

void Function::codegen() {
  auto func = [](Expr* curr_node) {
    curr_node->codegen();
  };
  Expr* curr = body();
  while (curr) {
    walkImpl<WalkOrderType::POST_ORDER>(func, curr->getLeft());
    curr = curr->getNext();
  }
}

Ast::Ast() {
  root_ = nullptr;
}

Ast::Ast(Function* root): root_(root){}

Ast::~Ast() {
  delete root_;
}

Function*& Ast::root() {
  return root_;
}

int Ast::computer() {
  return root_->computer();
}

void Ast::codegen() {
  root_->codegen();
}

int Ast::visualization(std::string filename) {
  std::ofstream fs(filename);
  std::ostringstream oss;
  int ident_num = 0;
  if (!fs.is_open()) {
    std::cout << "open file " << filename << " failed!" << std::endl;
    return -1;
  }

  ident(oss, ident_num);
  oss << "digraph ExampleGraph {\n";
  ident_num += 4;
  root_->visualize(oss, ident_num);
  ident_num -= 4;
  ident(oss, ident_num);
  oss << "}\n";
  fs << oss.str();
  return 0;
}
