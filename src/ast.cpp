#include "ast.h"
#include "codegen.h"
#include "logger.h"
#include "type.h"
#include "utils.h"
#include "instructions.h"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>


namespace rvcc {

std::atomic_int Expr::g_id = 0;

const char* global_func_name = nullptr;

const char* Expr::kind_names[static_cast<int>(ExprKind::NODE_COUNT)] {
  // 叶子节点
  "NODE_NUM",
  "NODE_ID",
  // unary op
  "NODE_NEG",
  "NODE_ADDR",
  "NODE_DEREF",
  "NODE_CALL",
  "NODE_RETURN",
  // binary op
  "NODE_ADD",
  "NODE_SUB",
  "NODE_MUL",
  "NODE_DIV",
  "NODE_EQ",
  "NODE_NE",
  "NODE_LT",
  "NODE_LE",
  "NODE_ASSIGN",
  // 语句
  "NODE_STMT",
  "NODE_COMPOUND",
  "NODE_IF",
  "NODE_FOR",
  "NODE_WHILE",
  "Node_ILLEGAL"
};

Var::Var() {
  name_ = nullptr;
  name_len_ = 0;
  value_ = 0;
}

Var::Var(char* name, int len, int value, Type* type): 
  name_(name), name_len_(len), value_(value), type_(type) {}

std::size_t& Var::name_len() {
  return name_len_;
}

const char* Var::getName() {
  return name_;
}

int& Var::value() {
  return value_;
}

int& Var::index() {
  return index_;
}

int& Var::offset() {
  return offset_;
}

Type*& Var::type() {
  return type_;
}

Expr::Expr() {
  kind_ = ExprKind::NODE_ILLEGAL;
  id_ = g_id;
  g_id++;
}

Expr::Expr(ExprKind kind):kind_(kind) {
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

Type* Expr::getType() {
  return nullptr;
}

int& Expr::id() {
  return id_;
}

ExprKind& Expr::kind() {
  return kind_;
}

const char* Expr::kindName() const {
  return kind_names[static_cast<int>(kind_)];
}

NextExpr::NextExpr(ExprKind kind, Expr* next):Expr(kind), next_(next) {
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

BinaryExpr::BinaryExpr(ExprKind kind, Expr* left, Expr* right):
  Expr(kind), left_(left), right_(right) {
  value_ = 0;
}

Expr* BinaryExpr::getLeft() {
  return left_;
}

Expr* BinaryExpr::getRight() {
  return right_;
}

Type* BinaryExpr::getType() {
  return type();
}

Type*& BinaryExpr::type() {
  return type_;
}

void BinaryExpr::codegen() {
  int offset = 0;
  switch (kind()) {
  case ExprKind::NODE_ADD:
    add_("a0", "a0", "a1");
    break;
  case ExprKind::NODE_SUB:
    sub_("a0", "a0", "a1");
    break;
  case ExprKind::NODE_MUL:
    mul_("a0", "a0", "a1");
    break;
  case ExprKind::NODE_DIV:
    div_("a0", "a0", "a1");
    break;
  case ExprKind::NODE_EQ:
    xor_("a0", "a0", "a1");
    seqz_("a0", "a0");
    break;
  case ExprKind::NODE_NE:
    xor_("a0", "a0", "a1");
    snez_("a0", "a0");
    break;
  case ExprKind::NODE_LT:
    slt_("a0", "a0", "a1");
    break;
  case ExprKind::NODE_LE:
    slt_("a0", "a1", "a0");
    xori_("a0", "a0", 1);
    break;
  case ExprKind::NODE_ASSIGN:
    if (getLeft()->kind() == ExprKind::NODE_ID) {
      walkRightImpl(getRight(), codegen_prev_func, codegen_mid_func, codegen_post_func);
      offset = -(dynamic_cast<IdentityExpr*>(getLeft())->var()->offset() + type()->size());
      sd_("a0", "fp", offset);
    } else if (getLeft()->kind() == ExprKind::NODE_DEREF) {
      genAddr(getLeft());
      push_("a0");
      walkRightImpl(getRight(), codegen_prev_func, codegen_mid_func, codegen_post_func);
      pop_("a1");
      sd_("a0", "a1", 0);
    }
    break;
  default:
    FATAL("binary expr cant support current kind: %s", kindName());
  }
}

int& BinaryExpr::value() {
  return value_;
}

void BinaryExpr::visualize(std::ostringstream& oss, int&ident_num) {
  ident(oss, ident_num);
  oss << id() << " [label=\"Node " << kindName()
              << " type: " << type()->kindName() << "\","  
              << "color=black]\n";
  ident(oss, ident_num);
  oss << id() << " -> " << getLeft()->id()
      << "[label=\"left\", color=black];\n";
  ident(oss, ident_num);
  oss << id() << " -> " << getRight()->id()
      << "[label=\"right\", color=black];\n";
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

UnaryExpr::UnaryExpr(ExprKind kind, Expr* left): Expr(kind), left_(left) {
  value_ = 0;
}

Expr* UnaryExpr::getLeft() {
  return left_;
}

Type* UnaryExpr::getType() {
  return type();
}

Type*& UnaryExpr::type() {
  return type_;
}

void UnaryExpr::codegen() {
  switch (kind()) {
  case ExprKind::NODE_NEG:
    walkRightImpl(getLeft(), codegen_prev_func, codegen_mid_func, codegen_post_func);
    neg_("a0", "a0");
    break;
  case ExprKind::NODE_RETURN:
    walkRightImpl(getLeft(), codegen_prev_func, codegen_mid_func, codegen_post_func);
    goto_return_label_(global_func_name);
    break;
  case ExprKind::NODE_ADDR:
    genAddr(getLeft());
    break;
  case ExprKind::NODE_DEREF:
    walkRightImpl(getLeft(), codegen_prev_func, codegen_mid_func, codegen_post_func);
    load(type());
    break;
  default:
    ERROR("unary expr cant support current kind: %s", kindName());
  }
}

int& UnaryExpr::value() {
  return value_;
}

void UnaryExpr::visualize(std::ostringstream& oss, int& ident_num) {
  ident(oss, ident_num);
  oss << id() << " [label=\"Node " << kindName()
      << " type: " << type()->kindName() << "\","
      << "color=black]\n";
  ident(oss, ident_num);
  oss << id() << " -> " << getLeft()->id() 
      << "[label=\"left\", color=black];\n";
}

Expr*& UnaryExpr::left() {
  return left_;
}

NumExpr::NumExpr(int value):
  Expr(ExprKind::NODE_NUM),
  type_(Type::typeInt) {
  value_ = value;
}

Type* NumExpr::getType() {
  return type();
}

Type*& NumExpr::type() {
  return type_;
}

void NumExpr::codegen() {
  li_("a0", value());
}

int& NumExpr::value() {
  return value_;
}

void NumExpr::visualize(std::ostringstream& oss, int& ident_num) {
  ident(oss, ident_num);
  oss << id() << " [label=\"Node " << kindName() << ": " << value()
      << " type: " << type()->kindName()
      << "\"," << "color=yellow]\n";
}

IdentityExpr::IdentityExpr(Var* var):
  Expr(ExprKind::NODE_ID),
  type_(Type::typeInt) {
  var_ = var;
}

IdentityExpr::~IdentityExpr() {}

Var*& IdentityExpr::var() {
  return var_;
}

Type* IdentityExpr::getType() {
  return type();
}

Type*& IdentityExpr::type() {
  return type_;
}

void IdentityExpr::codegen() {
  genAddr(this);
  if (type()->kind() == TypeKind::TYPE_ARRAY) {
    return;
  }
  load(type());
}

int& IdentityExpr::value() {
  return var_->value();
}

void IdentityExpr::visualize(std::ostringstream& oss,
                             int& ident_num) {
  std::string name(var()->getName(), var()->name_len()); 
  ident(oss, ident_num);
  oss << id() << " [label=\"Node " << kindName() << ": " << name
      << " type: " << type()->kindName()
      << " offset: " << var()->offset()
      << "\"," << "color=yellow]\n";
}



CallExpr::CallExpr(const char* func_name, std::size_t name_len): 
  Expr(ExprKind::NODE_CALL),
  type_(Type::typeInt),
  func_name_(func_name),
  name_len_(name_len) {}

Type* CallExpr::getType() {
  return type();
}

Type*& CallExpr::type() {
  return type_;
}

std::vector<Expr*>& CallExpr::args() {
  return args_;
}

const std::string CallExpr::getFuncName() {
  return std::string(func_name_, name_len_);
}

void CallExpr::codegen() {
  for (int i = args_.size() - 1; i >= 0; --i) {
    walkRightImpl(args_[i], codegen_prev_func, codegen_mid_func, codegen_post_func);
    push_("a0");
  }
  for (std::size_t i = 0; i < args_.size(); i++) {
    std::string str("a");
    str = str + std::to_string(i);
    pop_(str.c_str());
  }
  std::string func_name(func_name_, name_len_);
  call_(func_name.c_str());
}

int& CallExpr::value() {
  return value_;
}

void CallExpr::visualize(std::ostringstream& oss,
                             int& ident_num) {

  ident(oss, ident_num);
  oss << id() << " [label=\"Node " << kindName() << ": " << getFuncName()
      << " type: " << type()->kindName() 
      << "\"," << "color=yellow]\n";

  auto func = [&](Expr* curr_node) {
    curr_node->visualize(oss, ident_num);
    return true;
  };
  for (std::size_t i = 0; i < args_.size(); ++i) {
    walkLeftImpl(args_[i], nullptr, nullptr, func);
    ident(oss, ident_num);
    oss << id() << " -> " << args_[i]->id()
        << "[label=\"left\", color=black];\n";
  }
}

StmtExpr::StmtExpr(Expr* left):
  NextExpr(ExprKind::NODE_STMT) {
  left_ = left;
  value_ = 0;
}

StmtExpr::~StmtExpr() {}

Expr* StmtExpr::getLeft() {
  return left_;
}

Expr*& StmtExpr::left() {
  return left_;
}

void StmtExpr::codegen() {
  if (getLeft()) {
    walkRightImpl(getLeft(), codegen_prev_func, codegen_mid_func, codegen_post_func);
  }
}

int& StmtExpr::value() {
  return value_;
}

void StmtExpr::visualize(std::ostringstream& oss, int& ident_num) {

  ident(oss, ident_num);
  oss << id() << " [label=\"Node " << kindName() 
              << "\"," << "color=red]\n";
  auto func = [&](Expr* curr_node) {
    curr_node->visualize(oss, ident_num);
    return true;
  };
  if (getLeft()) {
    walkLeftImpl(getLeft(), nullptr, nullptr, func);
    ident(oss, ident_num);
    oss << id() << " -> " << getLeft()->id() << "[label=\"left\", color=black];\n";
  }
}

CompoundStmtExpr::CompoundStmtExpr(Expr* stmts):
  NextExpr(ExprKind::NODE_COMPOUND), stmts_(stmts) {
  value_ = 0;
}

CompoundStmtExpr::~CompoundStmtExpr() {}

Expr* CompoundStmtExpr::getStmts() {
  return stmts_;
}

Expr*& CompoundStmtExpr::stmts() {
  return stmts_;
}

void CompoundStmtExpr::codegen() {
  if (!stmts()) {
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
  oss << id() << " [label=\"Node " << kindName() << "\"," << "color=red]\n";
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
  NextExpr(ExprKind::NODE_IF), cond_(cond), then_(then), els_(els) {
  value_ = 0;
}

IfExpr::~IfExpr() {}

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

void IfExpr::codegen() {
  std::uint32_t unique_id = uniqueId();
  printf("\n# =====分支语句%d==============\n", unique_id);
    // 生成条件内语句
  printf("\n# Cond表达式%d\n", unique_id);
  walkRightImpl(getCond(), codegen_prev_func, codegen_mid_func, codegen_post_func);
  goto_else_label_("a0", unique_id);
  printf("\n# Then语句%d\n", unique_id);
  getThen()->codegen();
  goto_end_label_(unique_id);
  else_label_(unique_id);
  if (getEls()) {
    getEls()->codegen();
  }
  branch_end_label_(unique_id);
}


void IfExpr::visualize(std::ostringstream& oss, int& ident_num) {
  ident(oss, ident_num);
  oss << id() << " [label=\"Node " << kindName() << "\"," << "color=red]\n";
  auto func = [&](Expr* curr_node) {
    curr_node->visualize(oss, ident_num);
    return true;
  };
  if (getCond()) {
    walkLeftImpl(getCond(), nullptr, nullptr, func);
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

ForExpr::ForExpr(Expr* init, Expr* cond, Expr* inc, Expr* stmts): 
  NextExpr(ExprKind::NODE_FOR), init_(init),
  cond_(cond), inc_(inc), stmts_(stmts) {
  value_ = 0;
}

ForExpr::~ForExpr() {}

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

Expr*& ForExpr::stmts() {
  return stmts_;
}

int& ForExpr::value() {
  return value_;
}

void ForExpr::codegen() {
  std::uint32_t unique_id = uniqueId();
  printf("\n# =====循环语句%d===============\n", unique_id);
  if (getInit()) {
    printf("\n# Init语句%d\n", unique_id);
    walkRightImpl( getInit(), codegen_prev_func, codegen_mid_func, codegen_post_func);
  }
  loop_begin_label_(unique_id);
  if (getCond()) {
    printf("# Cond表达式%d\n", unique_id);
    walkRightImpl(getCond(), codegen_prev_func, codegen_mid_func, codegen_post_func);
    goto_loop_end_label_("a0", unique_id);
  }
  if (getStmts()) {
    printf("\n# 循环 body 语句%d\n", unique_id);
    getStmts()->codegen();
  }
  if (getInc()) {
    printf("\n# Inc语句%d\n", unique_id);
    walkRightImpl(getInc(), codegen_prev_func, codegen_mid_func, codegen_post_func);
  }
  goto_loop_begin_label_(unique_id);
  loop_end_label_(unique_id);
}

void ForExpr::visualize(std::ostringstream& oss, int& ident_num) {
  ident(oss, ident_num);
  oss << id() << " [label=\"Node " << kindName() << "\"," << "color=red]\n";
  auto func = [&](Expr* curr_node) {
    curr_node->visualize(oss, ident_num);
    return true;
  };
  if (getInit()) {
    walkLeftImpl(getInit(), nullptr, nullptr, func);
    ident(oss, ident_num);
    oss << id() << " -> " << init()->id() << "[label=\"init\", color=black];\n";
  }
  if (getCond()) {
    walkLeftImpl(getCond(), nullptr, nullptr, func);
    ident(oss, ident_num);
    oss << id() << " -> " << cond()->id() << "[label=\"cond\", color=black];\n";
  }
  if (getInc()) {
    walkLeftImpl(getInc(), nullptr, nullptr, func);
    ident(oss, ident_num);
    oss << id() << " -> " << inc()->id() << "[label=\"inc\", color=black];\n";
  }
  if (getStmts()) {
    getStmts()->visualize(oss, ident_num);
    ident(oss, ident_num);
    oss << id() << " -> " << stmts()->id() << "[label=\"stmts\", color=black];\n";
  }
}

WhileExpr::WhileExpr(Expr* cond, Expr* stmts): 
  NextExpr(ExprKind::NODE_WHILE), cond_(cond), stmts_(stmts) {
  value_ = 0;
}

WhileExpr::~WhileExpr() {}

Expr* WhileExpr::getStmts() {
  return stmts_;
}

Expr* WhileExpr::getCond() {
  return cond_;
}

Expr*& WhileExpr::cond() {
  return cond_;
}

Expr*& WhileExpr::stmts() {
  return stmts_;
}

int& WhileExpr::value() {
  return value_;
}

void WhileExpr::codegen() {
  std::uint32_t unique_id = uniqueId();
  printf("\n# =====循环语句%d===============\n", unique_id);
  loop_begin_label_(unique_id);
  if (getCond()) {
    printf("# Cond表达式%d\n", unique_id);
    walkRightImpl( getCond(), codegen_prev_func, codegen_mid_func, codegen_post_func);
    goto_loop_end_label_("a0", unique_id);
  }
  if (getStmts()) {
    printf("\n# 循环 body 语句%d\n", unique_id);
    getStmts()->codegen();
  }
  goto_loop_begin_label_(unique_id);
  loop_end_label_(unique_id);
}

void WhileExpr::visualize(std::ostringstream& oss, int& ident_num) {
  ident(oss, ident_num);
  oss << id() << " [label=\"Node " << kindName() << "\"," << "color=red]\n";
  auto func = [&](Expr* curr_node) {
    curr_node->visualize(oss, ident_num);
    return true;
  };
  if (getCond()) {
    walkLeftImpl(getCond(), nullptr, nullptr, func);
    ident(oss, ident_num);
    oss << id() << " -> " << cond()->id() << "[label=\"cond\", color=black];\n";
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

Type*& Function::type() {
  return type_;
}

const char*& Function::name() {
  return name_;
}

std::size_t& Function::name_len() {
  return name_len_;
}

std::map<std::size_t, Var*>& Function::var_maps() {
  return var_maps_;
}

Function::Function(Expr* body, std::map<std::size_t,
  Var*>&& var_maps):body_(body), var_maps_(std::move(var_maps)) {}


Function::~Function() {}

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

std::map<std::size_t, Var*>& Function::parameters() {
  return parameters_;
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

Ast::Ast() {}

void Ast::insert(std::pair<std::size_t, Function*> elem) {
  CHECK(functions_.insert(elem).second);
}

Ast::~Ast() {}

Function* Ast::entry_function() {
  CHECK(functions_.count(entry_function_) != 0);
  return functions_[entry_function_];
}

const std::map<std::size_t, Function*>& Ast::functions() {
  return functions_;
}

void Ast::set_entry_point(std::size_t entry_function) {
  entry_function_ = entry_function;
}

void Ast::codegen() {
  for (auto& elem: functions_) {
    elem.second->codegen();
  }
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
  for (auto& elem : functions_) {
    elem.second->visualize(oss, ident_num);
  }
  ident_num -= 4;
  ident(oss, ident_num);
  oss << "}\n";
  fs << oss.str();
  return 0;
}

} // end namespace rvcc
