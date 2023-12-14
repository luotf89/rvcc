#include "lexer.h"
#include <cctype>
#include <cstdlib>

using namespace rvcc;

Token Lexer::getNextToken() {
  Token new_token;
  if (curr_pos_) {
    skipSpace();
    new_token.getLoc() = curr_pos_;
    if (*curr_pos_ == '\0') {
      new_token.getType() = TokenType::TOKEN_EOF;
    } else if (std::isdigit(*curr_pos_)) {
      char* tmp = curr_pos_;
      new_token.getType() = TokenType::TOKEN_NUM;
      new_token.getValue() = std::strtoul(curr_pos_, &curr_pos_, 10);
      new_token.getLen() = curr_pos_ - tmp;
    } else if (std::isalpha(*curr_pos_) || *curr_pos_ == '_') {
      curr_pos_++;
      while(std::isalpha(*curr_pos_) || *curr_pos_ == '_' || std::isdigit(*curr_pos_)) {
        curr_pos_++;
      }
      new_token.getType() = TokenType::TOKEN_ID;
      new_token.getLen() = curr_pos_ - new_token.getLoc();
    } else if (new_token.getLen() = readPunct(curr_pos_),
               new_token.getLen()) {
        new_token.getType() = TokenType::TOKEN_PUNCT;
        curr_pos_ += new_token.getLen();
    } else {
        new_token.getType() = TokenType::TOKEN_ILLEGAL;
        std::cout << "current token is illegal" << curr_pos_ << std::endl;
        exit(1);
    }
  } else {
    std::cout << "ERROR input is empty!" << std::endl;
    exit(1);
  }
  return new_token;
}

void Lexer::init() {
  if (curr_pos_ == nullptr) {
    return;
  }
  curr_ = getNextToken();
}

bool Lexer::startWith(const char* str, const char* sub_str) {
  return std::strncmp(str, sub_str, strlen(sub_str)) == 0;
}

int Lexer::readPunct(const char* str) {
  if (startWith(str, "==") || startWith(str, "!=") ||
      startWith(str, ">=") || startWith(str, "<=")) {
    return 2;
  } 
  return ispunct(*str) ? 1 : 0;
}

void Lexer::skipSpace() {
  while(std::isspace(*curr_pos_)) {
    curr_pos_++;
  }
}