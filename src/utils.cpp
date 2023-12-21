#include <atomic>
#include <cstdint>
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

} // end namespace rvcc