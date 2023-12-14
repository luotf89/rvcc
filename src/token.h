#ifndef __TOKEN_H
#define __TOKEN_H

#include <map>
#include <string>
#include <iostream>

namespace rvcc {
  enum class TokenType:int{
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
      Token(TokenType type, int val, char* loc, int len);
      TokenType& getType() {
        return type_;
      }
      int& getValue() {
        return val_;
      }
      char*& getLoc() {
        return loc_;
      }
      int& getLen() {
        return len_;
      }
      const char* getTypeName() const;
    private:
      TokenType type_;
      int val_;
      char* loc_;
      int len_;
      static const char* type_names[static_cast<int>(TokenType::TOKEN_COUNT)];
  };
}
#endif
