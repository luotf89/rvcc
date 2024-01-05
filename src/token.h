#ifndef __TOKEN_H
#define __TOKEN_H

#include <cstddef>
#include <map>
#include <string>
#include <iostream>

namespace rvcc {

enum class TokenKind:int{
  TOKEN_ID = 0,
  TOKEN_PUNCT,
  TOKEN_NUM,
  TOKEN_EOF,
  TOKEN_KEYWORD,
  TOKEN_ILLEGAL,
  TOKEN_COUNT
};

class Token {
  public:
    Token();
    Token(TokenKind kind, int val, char* loc, int len);
    TokenKind& kind() {
      return kind_;
    }
    int& value() {
      return val_;
    }
    char*& loc() {
      return loc_;
    }
    std::size_t& len() {
      return len_;
    }
    const char* content();
    const char* kindName() const;
  private:
    TokenKind kind_;
    int val_;
    char* loc_;
    std::size_t len_;
    static const char* kind_names[static_cast<int>(TokenKind::TOKEN_COUNT)];
    static char buffer[128];
};

}
#endif
