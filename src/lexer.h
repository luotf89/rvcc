#ifndef __LEXER_H
#define __LEXER_H

#include "token.h"
#include <cctype>
#include <cstdlib>
#include <cstring>

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
      const char* getBuf() {
        return buffer_;
      }
      static bool startWith(const char* str, const char* sub_str);
      static int readPunct(const char* str);
    private:
      Token getNextToken();
      void skipSpace();
      Token curr_;
      char* curr_pos_;
      const char* const buffer_;
  };
}
#endif