#include "token.h"
#include <algorithm>
#include <cstddef>
#include <cstring>

namespace rvcc {

char Token::buffer[128]{0};

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

const char* Token::content() {
  memset(buffer, 0, 128);
  memcpy(buffer, loc_, std::min(static_cast<std::size_t>(127), len_));
  return buffer;
}

} // end namespace rvcc
