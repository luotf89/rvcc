#include "lexer.h"

using namespace rvcc;

Token Lexer::getNextToken() {
  Token new_token;
  if (curr_pos_) {
    skipSpace();
    new_token.getLoc() = curr_pos_;
    if (*curr_pos_ == '\0') {
        new_token.getType() = TokenType::TOKEN_EOF;
    } else if (std::isdigit(*curr_pos_)) {
        new_token.getType() = TokenType::TOKEN_NUM;
        new_token.getValue() = std::strtoul(curr_pos_, &curr_pos_, 10);
    } else if (new_token.getLen() = readPunct(curr_pos_),
               new_token.getLen()) {
        new_token.getType() = TokenType::TOKEN_PUNCT;
        curr_pos_ += new_token.getLen();
    } else {
        new_token.getType() = TokenType::TOKEN_ILLEGAL;
        curr_pos_++;
    }
  }
  return new_token;
}

void Lexer::init() {
  if (curr_pos_ == nullptr) {
    return;
  }
  curr_ = getNextToken();
}