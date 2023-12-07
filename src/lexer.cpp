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
    } else if (std::isalpha(*curr_pos_)) {
        new_token.getType() = TokenType::TOKEN_ID;
        new_token.getLen() = 1;
        curr_pos_++;
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