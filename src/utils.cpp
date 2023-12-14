#include "utils.h"
#include "hash.h"

std::size_t getstrHash(const char* str, int len) {
  std::size_t seed = 0;
  for(int i = 0; i < len; i++) {
    hash_combine(seed, *(str+i));
  }
  return seed;
}

void ident(std::ostringstream& oss, int& ident_num) {
  for (int i = 0; i < ident_num; i++) {
    oss << " ";
  }
}