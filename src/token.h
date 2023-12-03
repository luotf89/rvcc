#ifndef __TOKEN_H
#define __TOKEN_H

#include <map>
#include <string>
#include <iostream>

namespace rvcc {
  enum class TokenType{
    TOKEN_PUNCT,
    TOKEN_NUM,
    TOKEN_EOF,
    TOKEN_ILLEGAL,
  };

  class Token {
    public:
      Token() {
        type_ = TokenType::TOKEN_ILLEGAL;
        val_ = 0;
        loc_ = nullptr;
      }
      Token(TokenType type, int val, char* loc): type_(type),val_(val), loc_(loc) {}
      TokenType& getType() {
        return type_;
      }
      int& getValue() {
        return val_;
      }
      char*& getLoc() {
        return loc_;
      }
      void printTypeName() const {
        std::cout<< type_names.at(type_) << std::endl;
      }
    private:
      TokenType type_;
      int val_;
      char* loc_;
      static const std::map<TokenType, std::string> type_names;
  };
}
#endif
