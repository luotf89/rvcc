#ifndef __LEXER_H
#define __LEXER_H

#include "token.h"
#include <cctype>
#include <cstdlib>
#include <string>

namespace rvcc {
  class Lexer {
    public:
      Lexer(const char* buffer):buffer_(buffer) {
        curr_pos_ = const_cast<char*>(buffer_);
      }
      void init();
      void consumerToken() {
        curr_ = getNextToken();
      }
      Token getCurrToken() {
        return curr_;
      }
    private:
      Token getNextToken();
      void skipSpace() {
        while(std::isspace(*curr_pos_)) {
          curr_pos_++;
        }
      }
      Token curr_;
      char* curr_pos_;
      const char* const buffer_;
  };
}
#endif