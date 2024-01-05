#include "parser.h"
#include "ast.h"
#include "lexer.h"
#include "logger.h"
#include "type.h"
#include "utils.h"
#include "token.h"
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <utility>


using namespace rvcc;

/*
program = "{" compound_stmt
compound_stmt = (declaration | stmt)* "}"

declaration = declspec (declarator ("=" expr)? ("," declarator ("=" expr)?)*)? ";"
declspec = "int"
declarator = "*"* ident

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
unary = ("+" | "-" | "*" | "&")unary | primary
primary = val | "("expr")"
*/

Expr* Parser::binaryOp(Expr* left, Expr*right, ExprKind kind) {
  return new BinaryExpr(kind, left, right);
}

Expr* Parser::unaryOp(Expr*left, ExprKind kind) {
  return new UnaryExpr(kind, left);
}

void Parser::init() {
  lexer_.init();
}

Expr* Parser::parser_program() {
  rvcc::startWithStr("{", "program", lexer_);
  lexer_.consumerToken();
  Expr* program = parser_compound_stmt();
  return program;
}

Expr* Parser::parser_compound_stmt() {
  CompoundStmtExpr* compound_stmt = new CompoundStmtExpr();
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
    Type* type = parser_declarator(base_type);
    if (!(lexer_.getCurrToken().kind() == TokenKind::TOKEN_ID)) {
      printErrorInof("identify", "identify", lexer_);
    }
    std::size_t key = getstrHash(lexer_.getCurrToken().loc(), lexer_.getCurrToken().len());
    if (var_maps_.count(key) != 0) {
      FATAL("var %s is defined more than once", lexer_.getCurrToken().content());
    }
    Var* var = new Var(lexer_.getCurrToken().loc(), lexer_.getCurrToken().len());
    var->type() = type;
    var->offset() = var_idx_++;
    CHECK(var_maps_.insert({key, var}).second);
    lexer_.consumerToken();
    if (startWithStr("=", lexer_)) {
      lexer_.consumerToken();
      Expr* left = new IdentityExpr(var);
      static_cast<IdentityExpr*>(left)->type() = var->type();
      Expr* right = parser_assign();
      StmtExpr* tmp = new StmtExpr();
      CHECK(*(left->getType()) == *(right->getType()));
      tmp->left() = binaryOp(left, right, ExprKind::NODE_ASSIGN);
      dynamic_cast<BinaryExpr*>(tmp->getLeft())->type() = left->getType();
      curr->next() = tmp;
      curr = dynamic_cast<NextExpr*>(curr->getNext());
    }
  }
  CompoundStmtExpr* declare = new CompoundStmtExpr(head->getNext());
  head->next() = nullptr;
  delete head;
  return declare;
}

// 返回定义指针变量的类型 例如 int **a
Type* Parser::parser_declarator(Type* base_type) {
  Type* curr = base_type;
  while(startWithStr("*", lexer_)) {
    lexer_.consumerToken();
    curr = new Type(TypeKind::TYPE_PTR, curr);
  }
  return curr;
}

// 返回变量定义时 基本类型 比如 int a 的类型为 int
Type* Parser::parser_declspec() {
  if (startWithStr("int", lexer_)) {
    lexer_.consumerToken();
    return Type::typeInt;
  }
  printErrorInof("parser_declspec", "kind of types", lexer_);
  return nullptr;
}

Expr* Parser::parser_stmt() {
  Expr* stmt;
  
  if (startWithStr("return", lexer_)) {
    lexer_.consumerToken();
    stmt =  new StmtExpr;
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
    IfExpr* if_stmt = new IfExpr();
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
    ForExpr* for_stmt = new ForExpr();
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
    WhileExpr* while_stmt = new WhileExpr();
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
      stmt = new StmtExpr;
      return stmt;
    }
    stmt =  new StmtExpr;
    stmt->kind() = ExprKind::NODE_STMT;
    dynamic_cast<StmtExpr*>(stmt)->left() = parser_expr();
    startWithStr(";", "stmt", lexer_);
    lexer_.consumerToken();
  }
  return stmt;
}

Expr* Parser::parser_expr() {
  Expr* expr = parser_assign();
  return expr;
}

Expr* Parser::parser_assign() {
  Expr* expr = parser_equality();
  while(startWithStr("=", lexer_) ) {
    lexer_.consumerToken();
    expr = binaryOp(expr, parser_assign(), ExprKind::NODE_ASSIGN);
    CHECK(*(expr->getLeft()->getType()) == *(expr->getRight()->getType()));
    static_cast<BinaryExpr*>(expr)->type() = expr->getLeft()->getType();
  }
  return expr;
}

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
    CHECK(*(expr->getLeft()->getType()) == *(expr->getRight()->getType()));
    static_cast<BinaryExpr*>(expr)->type() = expr->getLeft()->getType();
  }
  return expr;
}

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
    CHECK(*(expr->getLeft()->getType()) == *(expr->getRight()->getType()));
    static_cast<BinaryExpr*>(expr)->type() = expr->getLeft()->getType();
  }
  return expr;
}

Expr* Parser::parser_add() {
  Expr* expr = parser_mul();
  while(startWithStr("+", lexer_) || 
        startWithStr("-", lexer_)) {
    char punct = *(lexer_.getCurrToken().loc());
    lexer_.consumerToken();
    Expr* left = expr;
    Expr* right = parser_mul();
    auto update_ptr_offset = [&]() {
      Expr* tmpval = new NumExpr(8);
      static_cast<NumExpr*>(tmpval)->type() = Type::typeInt;
      right = binaryOp(right, tmpval, ExprKind::NODE_MUL);
      static_cast<BinaryExpr*>(right)->type() = Type::typeInt;
    };
    if (punct == '+') {
      if (left->getType()->kind()==TypeKind::TYPE_INT && right->getType()->kind() == TypeKind::TYPE_INT) {
      } else if (left->getType()->kind()==TypeKind::TYPE_PTR && right->getType()->kind() == TypeKind::TYPE_INT) {
        update_ptr_offset();
      } else if (left->getType()->kind()==TypeKind::TYPE_INT && right->getType()->kind() == TypeKind::TYPE_PTR) {
        std::swap(left, right);
        update_ptr_offset();
      } else {
        FATAL("operator add can't support %s + %s", left->getType()->kindName(), right->getType()->kindName());
      }
      expr = binaryOp(left, right, ExprKind::NODE_ADD);
      static_cast<BinaryExpr*>(expr)->type() = expr->getLeft()->getType();
    } else {
      if ((left->getType()->kind()==TypeKind::TYPE_INT && right->getType()->kind() == TypeKind::TYPE_INT)) {
        expr = binaryOp(left, right, ExprKind::NODE_SUB);
        static_cast<BinaryExpr*>(expr)->type() = expr->getLeft()->getType();
      } else if (left->getType()->kind()==TypeKind::TYPE_PTR && right->getType()->kind() == TypeKind::TYPE_INT) {
        update_ptr_offset();
        expr = binaryOp(left, right, ExprKind::NODE_SUB);
        static_cast<BinaryExpr*>(expr)->type() = expr->getLeft()->getType();
      } else if (left->getType()->kind()==TypeKind::TYPE_PTR && right->getType()->kind() == TypeKind::TYPE_PTR) {
        expr = binaryOp(left, right, ExprKind::NODE_SUB);
        static_cast<BinaryExpr*>(expr)->type() = Type::typeInt;
      } else {
        FATAL("operator add can't support %s - %s", left->getType()->kindName(), right->getType()->kindName());
      }
    }
  }
  return expr;
}

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
      if (expr->getLeft()->getType()->kind() == TypeKind::TYPE_PTR) {
        static_cast<UnaryExpr*>(expr)->type() = expr->getLeft()->getType()->base();
      } else {
        CHECK(false);
        static_cast<UnaryExpr*>(expr)->type() = Type::typeInt;
      }
    } else {
      expr = unaryOp(parser_unary(), ExprKind::NODE_ADDR);
      Type* ptr = new Type(TypeKind::TYPE_PTR, expr->getLeft()->getType());
      static_cast<UnaryExpr*>(expr)->type() = ptr;
    }
  } else {
    expr = parser_primary();
  }
  return expr;
}

Expr* Parser::parser_primary() {
  Expr* expr;
  if (lexer_.getCurrToken().kind() == TokenKind::TOKEN_NUM) {
    expr = new NumExpr(lexer_.getCurrToken().value());
    static_cast<NumExpr*>(expr)->type() = Type::typeInt;
    lexer_.consumerToken();
  } else if (lexer_.getCurrToken().kind() == TokenKind::TOKEN_ID) {
    std::size_t hash_value = getstrHash(lexer_.getCurrToken().loc(),
                                       lexer_.getCurrToken().len());
    Var* var = nullptr;
    if (var_maps_.count(hash_value) == 0) {
      FATAL("identify: %s is used before define", lexer_.getCurrToken().loc());
    } else {
      var = var_maps_[hash_value];
    }
    expr = new IdentityExpr(var);
    static_cast<IdentityExpr*>(expr)->type() = var->type();
    lexer_.consumerToken();
  } else {
    startWithStr("(", "parser_primary", lexer_);
    lexer_.consumerToken();
    expr = parser_expr();
    startWithStr(")", "parser_primary", lexer_);
    lexer_.consumerToken();
  }
  return expr;
}

Ast* Parser::parser_ast() {
  init();
  Ast* ast = new Ast;
  ast->root() = new Function();
  ast->root()->body() = parser_program();
  ast->root()->var_maps() = std::move(var_maps_);
  return ast;
}