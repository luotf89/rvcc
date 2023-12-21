#include "ast.h"
#include "logger.h"
#include "utils.h"
#include "instructions.h"
#include <cassert>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>


namespace rvcc {

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
  "NODE_RETURN",
  "NODE_COMPOUND",
  "NODE_IF",
  "NODE_FOR",
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

Expr* Expr::getNext() {
  return nullptr;
}

Expr* Expr::getLeft() {
  return nullptr;
}

Expr* Expr::getRight() {
  return nullptr;
}

Expr* Expr::getStmts() {
  return nullptr;
}

Expr* Expr::getCond() {
  return nullptr;
}

Expr* Expr::getThen() {
  return nullptr;
}

Expr* Expr::getEls() {
  return nullptr;
}

Expr* Expr::getInit() {
  return nullptr;
}

Expr* Expr::getInc() {
  return nullptr;
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

NextExpr::NextExpr(ExprType type, Expr* next):Expr(type), next_(next) {
  return_flag_ = false;
}

Expr* NextExpr::getNext() {
  return next_;
}

Expr*& NextExpr::next() {
  return next_;
}

bool NextExpr::getReturnFlag() {
  return return_flag_;
}
bool& NextExpr::returnFlag() {
  return return_flag_;
}

BinaryExpr::BinaryExpr(ExprType type, Expr* left, Expr* right):
  Expr(type), left_(left), right_(right) {
  value_ = 0;
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
  ident(oss, ident_num);
  oss << id() << " [label=\"Node " << getTypeName() << "\"," << "color=black]\n";
  ident(oss, ident_num);
  oss << id() << " -> " << getLeft()->id() << "[label=\"left\", color=black];\n";
  ident(oss, ident_num);
  oss << id() << " -> " << getRight()->id() << "[label=\"right\", color=black];\n";
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

UnaryExpr::UnaryExpr(ExprType type, Expr* left): Expr(type), left_(left) {
  value_ = 0;
}

Expr* UnaryExpr::getLeft() {
  return left_;
}

int UnaryExpr::computer() {
  switch (type()) {
  case ExprType::NODE_NEG:
    value() = -getLeft()->value();
    break;
  case ExprType::NODE_RETURN:
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
  case ExprType::NODE_RETURN:
    goto_return_label_();
    break;
  default:
    ERROR("unary expr cant support current type: %s", getTypeName());
  }
}

int& UnaryExpr::value() {
  return value_;
}

void UnaryExpr::visualize(std::ostringstream& oss, int& ident_num) {
  ident(oss, ident_num);
  oss << id() << " [label=\"Node " << getTypeName() << "\"," << "color=black]\n";
  ident(oss, ident_num);
  oss << id() << " -> " << getLeft()->id() << "[label=\"left\", color=black];\n";
}

Expr*& UnaryExpr::left() {
  return left_;
}

NumExpr::NumExpr(int value):
  Expr(ExprType::NODE_NUM) {
  value_ = value;
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
  ident(oss, ident_num);
  oss << id() << " [label=\"Node " << getTypeName() << ": " << value() << "\"," << "color=yellow]\n";
}

IdentityExpr::IdentityExpr(Var* var):
  Expr(ExprType::NODE_ID) {
  var_ = var;
}

IdentityExpr::~IdentityExpr() {
  if (var_) {
    delete var_;
    var_ = nullptr;
  }
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
  ident(oss, ident_num);
  oss << id() << " [label=\"Node " << getTypeName() << ": " << name << "\"," << "color=yellow]\n";
}

StmtExpr::StmtExpr(Expr* left):
  NextExpr(ExprType::NODE_STMT) {
  left_ = left;
  value_ = 0;
}

StmtExpr::~StmtExpr() {
  assert(!getRight());
  auto func = [](Expr* curr_node) {
    delete curr_node;
  };
  if (getLeft()){
    walkImpl<WalkOrderType::POST_ORDER>(func, getLeft());
  }
}

Expr* StmtExpr::getLeft() {
  return left_;
}

Expr*& StmtExpr::left() {
  return left_;
}

int StmtExpr::computer() {
  auto func = [](Expr* curr_node) {
    curr_node->computer();
  };
  if (getLeft()){
    walkImpl<WalkOrderType::POST_ORDER>(func, getLeft());
    value() = getLeft()->value();
    if (getLeft()->type() == ExprType::NODE_RETURN) {
      returnFlag() = true;
    }
  } else {
    WARNING("current stmt is empty!");
  }
  
  return value();
}

void StmtExpr::codegen() {
  auto func = [](Expr* curr_node) {
    curr_node->codegen();
  };
  if (getLeft()) {
    walkImpl<WalkOrderType::POST_ORDER>(func, getLeft());
  } else {
    WARNING("current stmt is empty!");
  }
}

int& StmtExpr::value() {
  return value_;
}

void StmtExpr::visualize(std::ostringstream& oss, int& ident_num) {

  ident(oss, ident_num);
  oss << id() << " [label=\"Node " << getTypeName() << "\"," << "color=red]\n";
  auto func = [&](Expr* curr_node) {
    curr_node->visualize(oss, ident_num);
  };
  if (getLeft()) {
    walkImpl<WalkOrderType::POST_ORDER>(func, getLeft());
    ident(oss, ident_num);
    oss << id() << " -> " << getLeft()->id() << "[label=\"left\", color=black];\n";
  } else {
    WARNING("current stmt is empty!");
  }  
}

CompoundStmtExpr::CompoundStmtExpr(NextExpr* stmts):
  NextExpr(ExprType::NODE_COMPOUND), stmts_(stmts) {
  value_ = 0;
}

CompoundStmtExpr::~CompoundStmtExpr() {
  Expr* curr = stmts();
  Expr* prev = nullptr;
  while (curr) {
    if (prev) {
      delete prev;
    }
    prev = curr;
    curr = curr->getNext();
  } 
}

Expr* CompoundStmtExpr::getStmts() {
  return stmts_;
}

NextExpr*& CompoundStmtExpr::stmts() {
  return stmts_;
}

int CompoundStmtExpr::computer() {
  if (!stmts()) {
    WARNING("current compound stmt is empty!");
    return 0;
  }
  NextExpr* curr = stmts();
  NextExpr* prev = nullptr;
  while (curr) {
    curr->computer();
    prev = curr;
    returnFlag() = curr->getReturnFlag();
    if (getReturnFlag()) {
      break;
    }
    curr = dynamic_cast<NextExpr*>(curr->getNext());
  }
  if (prev) {
    value() = prev->value();
  } else {
    WARNING("current compound stmt is empty!");
  }
  return value();
}

void CompoundStmtExpr::codegen() {
  if (!stmts()) {
    WARNING("current compound stmt is empty!");
    return;
  }
  Expr* curr = stmts();
  while (curr) {
    curr->codegen();
    curr = curr->getNext();
  }
}

int& CompoundStmtExpr::value() {
  return value_;
}

void CompoundStmtExpr::visualize(std::ostringstream& oss, int& ident_num) {
  ident(oss, ident_num);
  oss << id() << " [label=\"Node " << getTypeName() << "\"," << "color=red]\n";

  if (!stmts()) {
    WARNING("current compound stmt is empty!");
  } 
  Expr* curr = stmts();
  Expr* prev = nullptr;
  if (curr) {
    while (curr) {
      curr->visualize(oss, ident_num);
      if (prev) {
        ident(oss, ident_num);
        oss << prev->id() << " -> " << curr->id() << "[label=\"next\", color=black];\n";
      }
      prev = curr;
      curr = curr->getNext();
    }
    ident(oss, ident_num);
    oss << id() << " -> " << stmts()->id() << "[label=\"stmt\", color=black];\n";
  }
}

IfExpr::IfExpr(Expr* cond, Expr* then, Expr* els):
  NextExpr(ExprType::NODE_IF), cond_(cond), then_(then), els_(els) {
  value_ = 0;
}

IfExpr::~IfExpr() {
  if (cond()) {
    delete cond_;
  }
  if (then()) {
    delete then_;
  }
  if (els()) {
    delete els_;
  }
}

Expr* IfExpr::getCond() {
  return cond_;
}

Expr* IfExpr::getThen() {
  return then_;
}

Expr* IfExpr::getEls() {
  return els_;
}

int& IfExpr::value() {
  return value_;
}

Expr*& IfExpr::cond() {
  return cond_;
}

Expr*& IfExpr::then() {
  return then_;
}

Expr*& IfExpr::els() {
  return els_;
}

int IfExpr::computer() {
  auto func = [&](Expr* curr_node) {
    curr_node->computer();
  };
  if (walkImpl<WalkOrderType::POST_ORDER>(func, getCond()), 
      getCond()->value()) {
    if (!getThen()) {
      FATAL("if stmt's then is nullptr");
    }
    value() = getThen()->computer();
    returnFlag() = dynamic_cast<NextExpr*>(getThen()) ->getReturnFlag();
  } else if (!(getCond()->computer()) && getEls()) {
    value() = getEls()->computer();
    returnFlag() = dynamic_cast<NextExpr*>(getEls()) ->getReturnFlag();
  }
  return value();
}

void IfExpr::codegen() {
  auto func = [&](Expr* curr_node) {
    curr_node->codegen();
  };
  walkImpl<WalkOrderType::POST_ORDER>(func, getCond());
  pop_("a0");
  std::uint32_t unique_id = uniqueId();
  goto_else_label_("a0", unique_id);
  getThen()->codegen();
  goto_end_label_(unique_id);
  else_label_(unique_id);
  if (getEls()) {
    getEls()->codegen();
  }
  end_label_(unique_id);
}


void IfExpr::visualize(std::ostringstream& oss, int& ident_num) {
  ident(oss, ident_num);
  oss << id() << " [label=\"Node " << getTypeName() << "\"," << "color=red]\n";
  auto func = [&](Expr* curr_node) {
    curr_node->visualize(oss, ident_num);
  };
  if (getCond()) {
    walkImpl<WalkOrderType::POST_ORDER>(func, getCond());
    ident(oss, ident_num);
    oss << id() << " -> " << cond()->id() << "[label=\"cond\", color=black];\n";
  } else {
    FATAL("if stmt's cond is nullptr");
  }
  if (getThen()) {
    getThen()->visualize(oss, ident_num);
    ident(oss, ident_num);
    oss << id() << " -> " << then()->id() << "[label=\"then\", color=black];\n";
  } else {
    FATAL("if stmt's then is nullptr");
  }
  if (getEls()) {
    getEls()->visualize(oss, ident_num);
    ident(oss, ident_num);
    oss << id() << " -> " << then()->id() << "[label=\"else\", color=black];\n";
  }
}

ForExpr::ForExpr(Expr* init, Expr* cond, Expr* inc, NextExpr* stmts): 
  NextExpr(ExprType::NODE_FOR), init_(init),
  cond_(cond), inc_(inc), stmts_(stmts) {
  value_ = 0;
}

ForExpr::~ForExpr() {
  if (init()) {
    delete init_;
  }
  if (cond()) {
    delete cond_;
  }
  if (inc()) {
    delete inc_;
  }
  if (stmts()) {
    delete stmts_;
  }
}

Expr* ForExpr::getStmts() {
  return stmts_;
}

Expr* ForExpr::getInit() {
  return init_;
}

Expr* ForExpr::getCond() {
  return cond_;
}

Expr* ForExpr::getInc() {
  return inc_;
}

Expr*& ForExpr::init() {
  return init_;
}

Expr*& ForExpr::cond() {
  return cond_;
}

Expr*& ForExpr::inc() {
  return inc_;
}

NextExpr*& ForExpr::stmts() {
  return stmts_;
}

int& ForExpr::value() {
  return value_;
}

int ForExpr::computer() {
  auto func = [&](Expr* curr_node) {
    curr_node->computer();
  };
  if (getInit()) {
    walkImpl<WalkOrderType::POST_ORDER>(func, getInit());
  }
  if (getCond()) {
    while (walkImpl<WalkOrderType::POST_ORDER>(func, getCond()),
           getCond()->value()) {
      getStmts()->computer();
      returnFlag() = dynamic_cast<NextExpr*>(getStmts())->getReturnFlag();
      if (returnFlag()) {
        value() = getStmts()->value();
        return value();
      }
      if (getInc()) {
        walkImpl<WalkOrderType::POST_ORDER>(func, getInc());
      }
    }
  } else {
    while (true) {
      getStmts()->computer();
      returnFlag() = dynamic_cast<NextExpr*>(getStmts())->getReturnFlag();
      if (returnFlag()) {
        value() = getStmts()->value();
        return value();
      }
      if (getInc()) {
        walkImpl<WalkOrderType::POST_ORDER>(func, getInc());
      }
    }
  }
  return value();
}

void ForExpr::codegen() {
  auto func = [&](Expr* curr_node) {
    curr_node->codegen();
  };
  if (getInit()) {
    walkImpl<WalkOrderType::POST_ORDER>(func, getInit());
  }
  std::uint32_t unique_id = uniqueId();
  loop_begin_label_(unique_id);
  if (getCond()) {
    walkImpl<WalkOrderType::POST_ORDER>(func, getCond());
    pop_("a0");
    goto_loop_end_label_("a0", unique_id);
  }
  if (getStmts()) {
    getStmts()->codegen();
  }
  if (getInc()) {
    walkImpl<WalkOrderType::POST_ORDER>(func, getInc());
  }
  goto_loop_begin_label_(unique_id);
  end_label_(unique_id);
}

void ForExpr::visualize(std::ostringstream& oss, int& ident_num) {
  ident(oss, ident_num);
  oss << id() << " [label=\"Node " << getTypeName() << "\"," << "color=red]\n";
  auto func = [&](Expr* curr_node) {
    curr_node->visualize(oss, ident_num);
  };
  if (getInit()) {
    walkImpl<WalkOrderType::POST_ORDER>(func, getInit());
    ident(oss, ident_num);
    oss << id() << " -> " << init()->id() << "[label=\"init\", color=black];\n";
  }
  if (getCond()) {
    walkImpl<WalkOrderType::POST_ORDER>(func, getCond());
    ident(oss, ident_num);
    oss << id() << " -> " << cond()->id() << "[label=\"cond\", color=black];\n";
  }
  if (getInc()) {
    walkImpl<WalkOrderType::POST_ORDER>(func, getInc());
    ident(oss, ident_num);
    oss << id() << " -> " << inc()->id() << "[label=\"inc\", color=black];\n";
  }
  if (getStmts()) {
    getStmts()->visualize(oss, ident_num);
    ident(oss, ident_num);
    oss << id() << " -> " << stmts()->id() << "[label=\"stmts\", color=black];\n";
  }

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


Function::~Function() {
  if (body()->getNext()) {
    WARNING("curr funtion has more than one top compound stmt");
  }
  Expr* curr = body();
  Expr* prev = nullptr;
  while (curr) {
    if (prev) {
      delete prev;
    }
    prev = curr;
    curr = curr->getNext();
  }
}


void Function::getVarOffsets() {
  int idx = 0;
  for (auto& elem : var_maps_) {
    elem.second->offset() = idx;
    idx++;
  }
}

void Function::visualize(std::ostringstream& oss, int& ident_num) {
  if (!body()) {
    WARNING("curr stmt is empty, can't be computed");
    return;
  }
  if (body()->getNext()) {
    WARNING("curr funtion has more than one top compound stmt");
  }
  Expr* curr = body();
  Expr* prev = nullptr;
  while(curr) {
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
  if (!body()) {
    WARNING("curr stmt is empty, can't be computed");
    return 0;
  }
  if (body()->getNext()) {
    WARNING("curr funtion has more than one top compound stmt");
  }
  
  Expr* curr = body();
  Expr* prev = nullptr;
  while(curr) {
    curr->computer();
    prev = curr;
    curr = curr->getNext();
  };
  return prev->value();
}

void Function::codegen() {
  if (!body()) {
    WARNING("curr stmt is empty, can't be computed");
    return;
  }
  if (body()->getNext()) {
    WARNING("curr funtion has more than one top compound stmt");
  }
  
  Expr* curr = body();
  while(curr) {
    curr->codegen();
    curr = curr->getNext();
  };
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

} // end namespace rvcc
