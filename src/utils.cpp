#include <atomic>
#include <cstdint>
#include "ast.h"
#include "logger.h"
#include "hash.h"
#include "utils.h"

namespace rvcc {

std::size_t getstrHash(const char* str, int len) {
  std::size_t seed = 0;
  for(int i = 0; i < len; i++) {
    hash_combine(seed, *(str+i));
  }
  return seed;
}

void ident(std::ostringstream& oss, int& ident_num) {
  for (int i = 0; i < ident_num; i++) {
    oss << " ";
  }
}

uint32_t uniqueId() {
  static std::atomic_uint32_t unique_id{0};
  unique_id += 1;
  return unique_id;
}

void printErrorInof(const char* type_name, const char* expect, Lexer& lexer) {
  int pos = lexer.getCurrToken().getLoc() - lexer.getBuf() + 1;
  FATAL("parser %s failed  expect current token is "
        "'%s'\n %s\n%*s", type_name, expect, lexer.getBuf(), pos, "^");
}

bool startWithStr(const char* str, Lexer& lexer) {
  if ((lexer.getCurrToken().getType() == TokenType::TOKEN_PUNCT || 
       lexer.getCurrToken().getType() == TokenType::TOKEN_KEYWORD) &&
      Lexer::startWith(lexer.getCurrToken().getLoc(), str) &&
      lexer.getCurrToken().getLen() == strlen(str)) {
    return true;
  }
  return false;
}

bool startWithStr(const char* str, Expr* expr, Lexer& lexer) {
  if (!(lexer.getCurrToken().getType() == TokenType::TOKEN_PUNCT &&
        Lexer::startWith(lexer.getCurrToken().getLoc(), str) &&
        lexer.getCurrToken().getLen() == strlen(str))) {
    assert(expr);
    const char* type_name = expr->getTypeName();
    delete expr;
    printErrorInof(type_name, str, lexer);
  }
  return true;
}

bool startWithStr(const char* str, const char* type_name, Lexer& lexer) {
  if (!(lexer.getCurrToken().getType() == TokenType::TOKEN_PUNCT &&
        Lexer::startWith(lexer.getCurrToken().getLoc(), str) &&
        lexer.getCurrToken().getLen() == strlen(str))) {
    printErrorInof(type_name, str, lexer);
  }
  return true;
}


void walkLeftImpl(
  Expr* curr_node,
  Func prev_func,
  Func mid_func,
  Func post_func) {
  if (!curr_node) {
    return;
  }
  if (prev_func) {
    if(!prev_func(curr_node)) {
      return;
    }
  }
  walkLeftImpl(curr_node->getLeft(), prev_func, mid_func, post_func);
  if (mid_func) {
    mid_func(curr_node);
  }
  walkLeftImpl(curr_node->getRight(), prev_func, mid_func, post_func);
  if (post_func) {
    post_func(curr_node);
  }
}

void walkRightImpl(
  Expr* curr_node,
  Func prev_func,
  Func mid_func,
  Func post_func) {
  if (!curr_node) {
    return;
  }
  if (prev_func) {
    if (!prev_func(curr_node)) {
      return;
    }
  }
  walkRightImpl(curr_node->getRight(), prev_func, mid_func, post_func);
  if (mid_func) {
    mid_func(curr_node);
  }
  walkRightImpl(curr_node->getLeft(), prev_func, mid_func, post_func);
  if (post_func) {
    post_func(curr_node);
  }
}


} // end namespace rvcc