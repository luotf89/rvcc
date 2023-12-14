#include "token.h"

using namespace rvcc;

const char* Token::type_names[static_cast<int>(TokenType::TOKEN_COUNT)] {
  "TOKEN_ID",
  "TOKEN_EOF",
  "TOKEN_NUM",
  "TOKEN_PUNCT",
  "TOKEN_KEYWORD",
  "TOKEN_ILLEGAL"
};

Token::Token() {
  type_ = TokenType::TOKEN_ILLEGAL;
  val_ = 0;
  loc_ = nullptr;
}

Token::Token(TokenType type, int val, char* loc, int len): 
  type_(type),val_(val), loc_(loc), len_(len) {}

const char* Token::getTypeName() const {
  return type_names[static_cast<int>(type_)];
}
