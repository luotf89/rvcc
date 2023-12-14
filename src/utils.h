#ifndef __UTILS_H
#define __UTILS_H

#include <cstddef>
#include <sstream>


std::size_t getstrHash(const char* str, int len);
void ident(std::ostringstream& oss, int& ident_num);




#endif