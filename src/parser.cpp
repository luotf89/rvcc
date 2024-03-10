#include "parser.h"
#include "ast.h"
#include "lexer.h"
#include "logger.h"
#include "type.h"
#include "object_manager.h"
#include "utils.h"
#include "token.h"
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <utility>


using namespace rvcc;

/*
// program = functionDefinition* | globalvar*
// functionDefinition = declspec declarator "{" compoundStmt*
// declspec = "int"
// declarator = "*"* ident typeSuffix
// typeSuffix = ("(" parameters? | "[" num "]")?
// parameters = (parameter ("," parameter)*)? ")"
// parameter = declspec declarator

compoundStmt = (declaration | stmt)* "}"
declaration = declspec (declarator ("=" expr)? ("," declarator ("=" expr)?)*)? ";"

stmt = "return" expr ";" |
       expr? ";" |
       "if" "(" expr ")" stmt ( "else" stmt )? |
       "for" "(" expr? ";" expr? ";" expr? ")" stmt |
       "while" "(" expr ")" stmt |
       "{" compound_stmt
expr = assign
assign = equality (= assign)*
equality =  relation ("==" relation | "!=" relation)*
relation = add ("<" add | ">" add | "<=" add | ">=" add)*
add = mul ("+" mul | "-" mul)*
mul = unary ("*" unary | "/" unary)*
unary = ("+" | "-" | "*" | "&")unary | postfix
postfix = primary ("[" expr "]")*
primary = num | "("expr")" | "sizeof" unary | var | funtioncall
funtioncall = ident "(" (expr (, expr)*)? ")"
*/

Expr* Parser::binaryOp(Expr* left, Expr*right, ExprKind kind) {
  return ObjectManager::getInst().alloc_type<BinaryExpr>(kind, left, right);
}

Expr* Parser::unaryOp(Expr*left, ExprKind kind) {
  return ObjectManager::getInst().alloc_type<UnaryExpr>(kind, left);
}

void Parser::updatePtrOffset(Expr*& left, Expr*& right) {
  Type* base_type = left->getType()->kind()==TypeKind::TYPE_PTR ?
    dynamic_cast<PtrType*>(left->getType())->base_type():
    dynamic_cast<ArrayType*>(left->getType())->base_type();
  Expr* tmpval = ObjectManager::getInst().alloc_type<NumExpr>(base_type->size());
  static_cast<NumExpr*>(tmpval)->type() = Type::typeInt;
  right = binaryOp(right, tmpval, ExprKind::NODE_MUL);
  static_cast<BinaryExpr*>(right)->type() = Type::typeInt;
}

Expr* Parser::newAdd(Expr *left, Expr *right) {
  Expr* expr;
  if (left->getType()->kind() == TypeKind::TYPE_INT &&
      right->getType()->kind() == TypeKind::TYPE_INT) {
  } else if ((left->getType()->kind() == TypeKind::TYPE_ARRAY ||
              left->getType()->kind() == TypeKind::TYPE_PTR) &&
             (right->getType()->kind() == TypeKind::TYPE_INT)) {
    updatePtrOffset(left, right);
  } else if ((left->getType()->kind() == TypeKind::TYPE_INT) &&
             (right->getType()->kind() == TypeKind::TYPE_ARRAY ||
              right->getType()->kind() == TypeKind::TYPE_PTR)) {
    std::swap(left, right);
    updatePtrOffset(left, right);
  } else {
    FATAL("operator add can't support %s + %s",
          left->getType()->kindName(), right->getType()->kindName());
  }
  expr = binaryOp(left, right, ExprKind::NODE_ADD);
  dynamic_cast<BinaryExpr*>(expr)->type() = left->getType();
  return expr;
}

Expr* Parser::newSub(Expr *left, Expr *right) {
  Expr* expr;
  if ((left->getType()->kind()==TypeKind::TYPE_INT && right->getType()->kind() == TypeKind::TYPE_INT)) {
    expr = binaryOp(left, right, ExprKind::NODE_SUB);
    static_cast<BinaryExpr*>(expr)->type() = expr->getLeft()->getType();
  } else if (left->getType()->kind()==TypeKind::TYPE_PTR && right->getType()->kind() == TypeKind::TYPE_INT) {
    updatePtrOffset(left, right);
    expr = binaryOp(left, right, ExprKind::NODE_SUB);
    static_cast<BinaryExpr*>(expr)->type() = expr->getLeft()->getType();
  } else if (left->getType()->kind()==TypeKind::TYPE_PTR && right->getType()->kind() == TypeKind::TYPE_PTR) {
    expr = binaryOp(left, right, ExprKind::NODE_SUB);
    static_cast<BinaryExpr*>(expr)->type() = Type::typeInt;
  } else {
    FATAL("operator add can't support %s - %s", left->getType()->kindName(), right->getType()->kindName());
  }
  return expr;
}

void Parser::init() {
  lexer_.init();
  global_var_index_ = 0;
  global_var_offset_ = 0;
}

// program = functionDefinition*
Ast* Parser::parser_program() {
  init();
  Ast* ast = ObjectManager::getInst().alloc_type<Ast>();
  while(lexer_.getCurrToken().kind() != TokenKind::TOKEN_EOF) {
    Token id;
    Type* base_type = parser_declspec();
    Type* type = parser_declarator(base_type, id);

    if (startWithStr("{", lexer_)) {
      lexer_.consumerToken();
      Function* func = parser_function(type, id);
      std::size_t hash_value = getstrHash(func->name(), func->name_len());
      ast->insert({hash_value, func});
      std::string name(func->name(), func->name_len());
      if (name == "main") {
        ast->set_entry_point(hash_value);
      }
    } else if (startWithStr(";", lexer_) ||
               startWithStr(",", lexer_)) {
      auto handle_global_var = [&]() {
        lexer_.consumerToken();
        Var* var = ObjectManager::getInst().alloc_type<Var>(id.loc(), id.len());
        std::size_t hash_value = getstrHash(var->getName(), var->name_len());
        CHECK(global_vars_.insert({hash_value, var}).second);
        var->type() = type;
        var->index() = global_var_index_;
        var->offset() = global_var_offset_;
        var->addr_kind() = AddrKind::ADDR_DATA;
        global_var_index_++;
        global_var_offset_ += type->size();
      };
      while(startWithStr(",", lexer_)) {
        handle_global_var();
        if (lexer_.getCurrToken().kind() == TokenKind::TOKEN_ID) {
          id = lexer_.getCurrToken();
          lexer_.consumerToken();
        }
      }
      CHECK(startWithStr(";", lexer_));
      handle_global_var();
    } else {
      FATAL("parser function or global var is failed");
    }

  }
  ast->global_vars().swap(global_vars_);
  return ast;
}

// functionDefinition = declspec declarator "{" compoundStmt*
// 将寄存器里保存的参数 保存在栈中， 当局部变量使用
Function* Parser::parser_function(Type* type, Token& id) {
  local_var_index_ = 0;
  local_var_offset_ = 0;
  CHECK(type->kind() == TypeKind::TYPE_FUNC);
  Function* func = ObjectManager::getInst().alloc_type<Function>();
  func->name() = id.loc();
  func->name_len() = id.len();
  func->body() = parser_compound_stmt();
  func->local_vars().swap(local_vars_);
  func->parameters().swap(parameter_maps_);
  func->type() = type;
  return func;
}

// declspec = "int"
// 返回变量定义时 基本类型 比如 int a 的类型为 int
Type* Parser::parser_declspec() {
  if (startWithStr("int", lexer_)) {
    lexer_.consumerToken();
    return Type::typeInt;
  }
  printErrorInof("parser_declspec", "kind of types", lexer_);
  return nullptr;
}

// declarator = "*"* ident typeSuffix
Type* Parser::parser_declarator(Type* base_type, Token& id) {
  Type* curr = base_type;
  while(startWithStr("*", lexer_)) {
    lexer_.consumerToken();
    curr = ObjectManager::getInst().alloc_type<PtrType>(curr);
  }
  CHECK(lexer_.getCurrToken().kind()==TokenKind::TOKEN_ID);
  id = lexer_.getCurrToken();
  lexer_.consumerToken();
  Type* type = parser_suffix(curr, id);
  return type;
}

// typeSuffix = ("(" parameters? | "[" num "]")?
Type* Parser::parser_suffix(Type* base_type, Token& id) {
  if (startWithStr("(", lexer_)) {
    lexer_.consumerToken();
    FuncType* func_type = dynamic_cast<FuncType*>(
      ObjectManager::getInst().alloc_type<FuncType>(id.loc(), id.len()));
    // FuncType* func_type = new FuncType(id.loc(), id.len());
    func_type->ret_type() = base_type;
    parser_parameters(func_type);
    return func_type;
  } else if (startWithStr("[", lexer_)) {
    lexer_.consumerToken();
    CHECK(lexer_.getCurrToken().kind()==TokenKind::TOKEN_NUM);
    std::size_t size = lexer_.getCurrToken().value();
    lexer_.consumerToken();
    CHECK(startWithStr("]", lexer_));
    lexer_.consumerToken();
    if (startWithStr("[", lexer_)) {
      base_type = parser_suffix(base_type, id);
    }
    base_type = ObjectManager::getInst().alloc_type<ArrayType>(base_type, size);
    // base_type = new ArrayType(base_type, size);
    return base_type;
  } else {
    return base_type;
  }
}

// parameters = (parameter ("," parameter)*)? ")"
void Parser::parser_parameters(FuncType* func_type) {
  if (!startWithStr(")", lexer_)) {
    parser_parameter(func_type);
  }
  while (!startWithStr(")", lexer_)) {
    CHECK(startWithStr(",", lexer_));
    lexer_.consumerToken();
    parser_parameter(func_type); 
  }
  CHECK(startWithStr(")", lexer_));
  lexer_.consumerToken();
}

// parameter = declspec declarator
void Parser::parser_parameter(FuncType* func_type) {
  Type* base_type = parser_declspec();
  Token id;
  Type* type = parser_declarator(base_type, id);
  Var* var = ObjectManager::getInst().alloc_type<Var>(id.loc(), id.len());
  std::size_t hash_value = getstrHash(var->getName(), var->name_len());
  CHECK(local_vars_.insert({hash_value, var}).second);
  var->type() = type;
  var->index() = local_var_index_;
  var->offset() = local_var_offset_;
  var->addr_kind() = AddrKind::ADDR_STACK;
  local_var_index_++;
  local_var_offset_ += type->size();
  CHECK(parameter_maps_.insert({hash_value, local_vars_[hash_value]}).second);
  func_type->parameter_types().push_back(local_vars_[hash_value]->type());
}

// compoundStmt = (declaration | stmt)* "}"
Expr* Parser::parser_compound_stmt() {
  CompoundStmtExpr* compound_stmt = ObjectManager::getInst().alloc_type<CompoundStmtExpr>();
  NextExpr* head = new StmtExpr();
  NextExpr* curr_stmt = head; 
  while(!(startWithStr("}", lexer_) ||
          lexer_.getCurrToken().kind() == TokenKind::TOKEN_ILLEGAL ||
          lexer_.getCurrToken().kind() == TokenKind::TOKEN_EOF)) {
    if (startWithStr("int", lexer_)) {
      curr_stmt->next() = parser_declaration();
    } else {
      curr_stmt->next() = parser_stmt();
    }
    curr_stmt = dynamic_cast<NextExpr*>(curr_stmt->getNext());
  }
  startWithStr("}", "compound", lexer_);
  lexer_.consumerToken();
  compound_stmt->stmts() = head->getNext();
  head->next() = nullptr;
  delete head;
  return compound_stmt;
}

// declaration = declspec (declarator ("=" expr)? ("," declarator ("=" expr)?)*)? ";"
Expr* Parser::parser_declaration() {
  Type* base_type = parser_declspec();
  int count = 0;
  NextExpr* head = new StmtExpr();
  NextExpr* curr = head;
  while (!startWithStr(";", lexer_)) {
    // 解析 int a, *b, c=5; 跳过第一个 ","
    if (count > 0) {
      if (startWithStr(",", "declaration", lexer_)) {
        lexer_.consumerToken();
      }
    }
    count++;
    Token id;
    Type* type = parser_declarator(base_type, id);
    Var* var = ObjectManager::getInst().alloc_type<Var>(id.loc(), id.len());
    std::size_t hash_value = getstrHash(var->getName(), var->name_len());
    CHECK(local_vars_.insert({hash_value, var}).second);
    var->type() = type;
    var->index() = local_var_index_;
    var->offset() = local_var_offset_;
    var->addr_kind() = AddrKind::ADDR_STACK;
    local_var_index_++;
    local_var_offset_ += type->size();
    if (startWithStr("=", lexer_)) {
      lexer_.consumerToken();
      Expr* left = ObjectManager::getInst().alloc_type<IdentityExpr>(var);
      static_cast<IdentityExpr*>(left)->type() = var->type();
      Expr* right = parser_assign();
      StmtExpr* tmp = ObjectManager::getInst().alloc_type<StmtExpr>();
      CHECK(left->getType()->equal(right->getType()));
      tmp->left() = binaryOp(left, right, ExprKind::NODE_ASSIGN);
      dynamic_cast<BinaryExpr*>(tmp->getLeft())->type() = left->getType();
      curr->next() = tmp;
      curr = dynamic_cast<NextExpr*>(curr->getNext());
    }
  }
  CompoundStmtExpr* declare = ObjectManager::getInst().alloc_type<CompoundStmtExpr>(head->getNext());
  head->next() = nullptr;
  delete head;
  return declare;
}

// stmt = "return" expr ";" |
//        expr? ";" |
//        "if" "(" expr ")" stmt ( "else" stmt )? |
//        "for" "(" expr? ";" expr? ";" expr? ")" stmt |
//        "while" "(" expr ")" stmt |
//        "{" compound_stmt
Expr* Parser::parser_stmt() {
  Expr* stmt;
  
  if (startWithStr("return", lexer_)) {
    lexer_.consumerToken();
    stmt =  ObjectManager::getInst().alloc_type<StmtExpr>();
    dynamic_cast<StmtExpr*>(stmt)->left() =
      unaryOp(parser_expr(), ExprKind::NODE_RETURN);
    static_cast<UnaryExpr*>(stmt->getLeft())->type() = stmt->getLeft()->getLeft()->getType();
    startWithStr(";", "return", lexer_);
    lexer_.consumerToken();
  } else if (startWithStr("{", lexer_)) {
    lexer_.consumerToken();
    stmt = parser_compound_stmt();
  } else if (startWithStr("if", lexer_)) {
    lexer_.consumerToken();
    IfExpr* if_stmt = ObjectManager::getInst().alloc_type<IfExpr>();
    if (startWithStr("(", "if", lexer_)) {
      lexer_.consumerToken();
      if_stmt->cond() = parser_expr();
      startWithStr(")", "if", lexer_);
      lexer_.consumerToken();
      if_stmt->then() = parser_stmt();
      if (startWithStr("else", lexer_)) {
        lexer_.consumerToken();
        if_stmt->els() = parser_stmt();
      }
    }
    stmt = if_stmt;
  } else if (startWithStr("for", lexer_)) {
    lexer_.consumerToken();
    ForExpr* for_stmt = ObjectManager::getInst().alloc_type<ForExpr>();
    if (startWithStr("(", "for", lexer_)) {
      lexer_.consumerToken();
      if (!startWithStr(";", lexer_)) {
        for_stmt->init() = parser_expr();
      }
      startWithStr(";", "for", lexer_);
      lexer_.consumerToken();
      if (!startWithStr(";", lexer_)) {
        for_stmt->cond() = parser_expr();
      }
      startWithStr(";", "for", lexer_);
      lexer_.consumerToken();
      if (!startWithStr(")", lexer_)) {
        for_stmt->inc() = parser_expr();
      }
      startWithStr(")", "for", lexer_);
      lexer_.consumerToken();
      for_stmt->stmts() = parser_stmt();
    }
    stmt = for_stmt;
  } else if (startWithStr("while", lexer_)) {
    lexer_.consumerToken();
    WhileExpr* while_stmt = ObjectManager::getInst().alloc_type<WhileExpr>();
    if (startWithStr("(", "while", lexer_)) {
      lexer_.consumerToken();
      while_stmt->cond() = parser_expr();
      startWithStr(")", "while", lexer_);
      lexer_.consumerToken();
      while_stmt->stmts() = parser_stmt();
    }
    stmt = while_stmt;
  } else {
    // 空语句的处理逻辑 ;;
    if (startWithStr(";", lexer_)) {
      lexer_.consumerToken();
      stmt = ObjectManager::getInst().alloc_type<StmtExpr>();
      return stmt;
    }
    stmt =  ObjectManager::getInst().alloc_type<StmtExpr>();
    stmt->kind() = ExprKind::NODE_STMT;
    dynamic_cast<StmtExpr*>(stmt)->left() = parser_expr();
    startWithStr(";", "stmt", lexer_);
    lexer_.consumerToken();
  }
  return stmt;
}

// expr = assign
Expr* Parser::parser_expr() {
  Expr* expr = parser_assign();
  return expr;
}

Expr* Parser::parser_assign() {
  Expr* expr = parser_equality();
  while(startWithStr("=", lexer_) ) {
    lexer_.consumerToken();
    expr = binaryOp(expr, parser_assign(), ExprKind::NODE_ASSIGN);
    CHECK(expr->getLeft()->getType()->equal(expr->getRight()->getType()));
    static_cast<BinaryExpr*>(expr)->type() = expr->getLeft()->getType();
  }
  return expr;
}

// assign = equality (= assign)*
Expr* Parser::parser_equality() {
  Expr* expr = parser_relation();
  while(startWithStr("==", lexer_) ||
        startWithStr("!=", lexer_)) {
    const char* curr_punct = lexer_.getCurrToken().loc();
    lexer_.consumerToken();
    if (Lexer::startWith(curr_punct, "==")) {
      expr = binaryOp(expr, parser_relation(), ExprKind::NODE_EQ);
    } else {
      expr = binaryOp(expr, parser_relation(), ExprKind::NODE_NE);
    }
    CHECK(expr->getLeft()->getType()->equal(expr->getRight()->getType()));
    static_cast<BinaryExpr*>(expr)->type() = expr->getLeft()->getType();
  }
  return expr;
}

// relation = add ("<" add | ">" add | "<=" add | ">=" add)*
Expr* Parser::parser_relation() {
  Expr* expr = parser_add();
  while(startWithStr("<=", lexer_) ||
        startWithStr(">=", lexer_) ||
        startWithStr("<", lexer_) ||
        startWithStr(">", lexer_)) {
    const char* curr_punct = lexer_.getCurrToken().loc();
    int curr_len = lexer_.getCurrToken().len();
    lexer_.consumerToken();
    if (curr_len == 1) {
      if (Lexer::startWith(curr_punct, ">")) {
        expr = binaryOp(parser_add(), expr, ExprKind::NODE_LT);
      } else {
        expr = binaryOp(expr, parser_add(), ExprKind::NODE_LT);
      }
    } else {
      if (Lexer::startWith(curr_punct, ">=")) {
        expr = binaryOp(parser_add(), expr, ExprKind::NODE_LE);
      } else {
        expr = binaryOp(expr, parser_add(), ExprKind::NODE_LE);
      }
    }
    CHECK(expr->getLeft()->getType()->equal(expr->getRight()->getType()));
    static_cast<BinaryExpr*>(expr)->type() = expr->getLeft()->getType();
  }
  return expr;
}

// add = mul ("+" mul | "-" mul)*
Expr* Parser::parser_add() {
  Expr* expr = parser_mul();
  while(startWithStr("+", lexer_) || 
        startWithStr("-", lexer_)) {
    char punct = *(lexer_.getCurrToken().loc());
    lexer_.consumerToken();
    Expr* left = expr;
    Expr* right = parser_mul();
    if (punct == '+') {
      expr = newAdd(left, right);
    } else {
      expr = newSub(left, right);
    }
  }
  return expr;
}

// mul = unary ("*" unary | "/" unary)*
Expr* Parser::parser_mul() {
  Expr* expr = parser_unary();
  while(startWithStr("*", lexer_) || 
        startWithStr("/", lexer_)) {
    char punct = *(lexer_.getCurrToken().loc());
    lexer_.consumerToken();
    if (punct == '*') {
      expr = binaryOp(expr, parser_unary(), ExprKind::NODE_MUL);
    } else {
      expr = binaryOp(expr, parser_unary(), ExprKind::NODE_DIV);
    }
    static_cast<BinaryExpr*>(expr)->type() = expr->getLeft()->getType();
  }
  return expr;
}

// unary = ("+" | "-" | "*" | "&")unary | postfix
Expr* Parser::parser_unary() {
  Expr* expr;
  if (startWithStr("+", lexer_) || 
      startWithStr("-", lexer_) || 
      startWithStr("*", lexer_) ||
      startWithStr("&", lexer_)) {
    char punct = *(lexer_.getCurrToken().loc());
    lexer_.consumerToken();
    if (punct == '+') {
      expr = parser_unary();
    } else if (punct == '-'){
      expr = unaryOp(parser_unary(), ExprKind::NODE_NEG);
      static_cast<UnaryExpr*>(expr)->type() = expr->getLeft()->getType();
    } else if (punct == '*'){
      expr = unaryOp(parser_unary(), ExprKind::NODE_DEREF);
      CHECK(expr->getLeft()->getType()->kind() == TypeKind::TYPE_PTR ||
            expr->getLeft()->getType()->kind() == TypeKind::TYPE_ARRAY)
      static_cast<UnaryExpr*>(expr)->type() = 
        (expr->getLeft()->getType()->kind() == TypeKind::TYPE_PTR?
         dynamic_cast<PtrType*>(expr->getLeft()->getType())->base_type():
         dynamic_cast<ArrayType*>(expr->getLeft()->getType())->base_type());
    } else {
      expr = unaryOp(parser_unary(), ExprKind::NODE_ADDR);
      Type* ptr = ObjectManager::getInst().alloc_type<PtrType>(expr->getLeft()->getType());
      // Type* ptr = new PtrType(expr->getLeft()->getType());
      static_cast<UnaryExpr*>(expr)->type() = ptr;
    }
  } else {
    expr = parser_postfix();
  }
  return expr;
}

// postfix = primary ("[" expr "]")*
Expr* Parser::parser_postfix() {
  Expr* expr = parser_primary();
  while (startWithStr("[", lexer_)) {
    lexer_.consumerToken();
    Expr* idx = parser_expr();
    expr = ObjectManager::getInst().alloc_type<UnaryExpr>(ExprKind::NODE_DEREF, newAdd(expr, idx));
    CHECK(expr->getLeft()->getType()->kind() == TypeKind::TYPE_ARRAY ||
          expr->getLeft()->getType()->kind() == TypeKind::TYPE_PTR);
    Type* base_type = expr->getLeft()->getType()->kind() == TypeKind::TYPE_PTR?
       dynamic_cast<PtrType*>(expr->getLeft()->getType())->base_type():
       dynamic_cast<ArrayType*>(expr->getLeft()->getType())->base_type();
    dynamic_cast<UnaryExpr*>(expr)->type() = base_type;
    CHECK(startWithStr("]", lexer_));
    lexer_.consumerToken();
  }
  return expr;
}

// primary = num | "("expr")" | "sizeof" unary | var | funtioncall
Expr* Parser::parser_primary() {
  Expr* expr;
  if (lexer_.getCurrToken().kind() == TokenKind::TOKEN_NUM) {
    expr = ObjectManager::getInst().alloc_type<NumExpr>(lexer_.getCurrToken().value());
    static_cast<NumExpr*>(expr)->type() = Type::typeInt;
    lexer_.consumerToken();
  } else if (lexer_.getCurrToken().kind() == TokenKind::TOKEN_ID) {
    Token id = lexer_.getCurrToken();
    lexer_.consumerToken();
    if (startWithStr("(", lexer_)) {
      lexer_.consumerToken();
      expr = parser_call(id);
    } else {
      std::size_t hash_value = getstrHash(id.loc(), id.len());
      Var* var = nullptr;
      std::string id_name(id.loc(), id.len());
      if (local_vars_.count(hash_value) == 0 &&
          global_vars_.count(hash_value) == 0 &&
          parameter_maps_.count(hash_value) == 0) {
        FATAL("identify: %s is used before define", id_name.c_str());
      } else {
        if (local_vars_.count(hash_value) != 0) {
          var = local_vars_[hash_value];
        } else if (global_vars_.count(hash_value) != 0) {
          var = global_vars_[hash_value];
        } else {
          var = parameter_maps_[hash_value];
        }
      }
      IdentityExpr* id_expr = ObjectManager::getInst().alloc_type<IdentityExpr>(var);
      id_expr->type() = var->type();
      expr = id_expr;
    }
  } else if (startWithStr("sizeof", lexer_)) {
    lexer_.consumerToken();
    expr = parser_unary();
    expr = ObjectManager::getInst().alloc_type<NumExpr>(expr->getType()->size());
    dynamic_cast<NumExpr*>(expr)->type() = Type::typeInt;
    return expr;
  } else {
    startWithStr("(", "parser_primary", lexer_);
    lexer_.consumerToken();
    expr = parser_expr();
    startWithStr(")", "parser_primary", lexer_);
    lexer_.consumerToken();
  }
  return expr;
}

// funcall = ident "(" (expr ("," expr)*)? ")"
Expr* Parser::parser_call(Token& id) {
  CallExpr* expr = ObjectManager::getInst().alloc_type<CallExpr>(id.loc(), id.len());
  if (!startWithStr(")", lexer_)) {
    expr->args().push_back(parser_expr());
  }
  while(!startWithStr(")", lexer_)) {
    CHECK(startWithStr(",", lexer_));
    lexer_.consumerToken();
    expr->args().push_back(parser_expr());
  }
  lexer_.consumerToken();
  expr->type() = Type::typeInt;
  return expr;
}
