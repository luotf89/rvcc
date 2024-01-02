#include "token.h"

namespace rvcc {


const char* Token::kind_names[static_cast<int>(TokenKind::TOKEN_COUNT)] {
  "TOKEN_ID",
  "TOKEN_EOF",
  "TOKEN_NUM",
  "TOKEN_PUNCT",
  "TOKEN_KEYWORD",
  "TOKEN_ILLEGAL"
};

Token::Token() {
  kind_ = TokenKind::TOKEN_ILLEGAL;
  val_ = 0;
  loc_ = nullptr;
}

Token::Token(TokenKind kind, int val, char* loc, int len): 
  kind_(kind),val_(val), loc_(loc), len_(len) {}

const char* Token::kindName() const {
  return kind_names[static_cast<int>(kind_)];
}

} // end namespace rvcc
