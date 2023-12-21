#ifndef __UTILS_H
#define __UTILS_H

#include <sstream>
#include "lexer.h"
#include "ast.h"

namespace rvcc {

std::size_t getstrHash(const char* str, int len);
void ident(std::ostringstream& oss, int& ident_num);
uint32_t uniqueId();
bool startWithStr(const char* str, Lexer& lexer);
bool startWithStr(const char* str, Expr* expr, Lexer& lexer);
bool startWithStr(const char* str, const char* type_name, Lexer& lexer);

} // end namespace rvcc

#endif