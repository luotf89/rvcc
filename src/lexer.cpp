#include "lexer.h"
#include "token.h"
#include <cctype>
#include <cstdlib>

namespace rvcc {

const char* keywords[] {
  "return",
  "if",
  "else",
  "for",
  "while",
  nullptr
};

Token Lexer::getNextToken() {
  Token new_token;
  if (curr_pos_) {
    skipSpace();
    new_token.loc() = curr_pos_;
    if (*curr_pos_ == '\0') {
      new_token.kind() = TokenKind::TOKEN_EOF;
    } else if (std::isdigit(*curr_pos_)) {
      char* tmp = curr_pos_;
      new_token.kind() = TokenKind::TOKEN_NUM;
      new_token.value() = std::strtoul(curr_pos_, &curr_pos_, 10);
      new_token.len() = curr_pos_ - tmp;
    } else if (std::isalpha(*curr_pos_) || *curr_pos_ == '_') {
      curr_pos_++;
      while(std::isalpha(*curr_pos_) || *curr_pos_ == '_' || std::isdigit(*curr_pos_)) {
        curr_pos_++;
      }
      new_token.kind() = TokenKind::TOKEN_ID;
      new_token.len() = curr_pos_ - new_token.loc();
      int i = 0;
      while(keywords[i] != nullptr) {
        if (startWith(new_token.loc(), keywords[i]) && 
            new_token.len() == strlen(keywords[i])) {
          new_token.kind() = TokenKind::TOKEN_KEYWORD;
          break;
        }
        i++;
      }
    } else if (new_token.len() = readPunct(curr_pos_),
               new_token.len()) {
        new_token.kind() = TokenKind::TOKEN_PUNCT;
        curr_pos_ += new_token.len();
    } else {
        new_token.kind() = TokenKind::TOKEN_ILLEGAL;
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

} // end namespace rvcc
