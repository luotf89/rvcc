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
bool startWithStr(const char* str, const char* kind_name, Lexer& lexer);

// 如果返回值为ture 者认为该节点为非叶子节点，
// 后续此节点继续继续递归，
// 如果返回值为false 者认为该节点为叶子节点 
// 不参与后续递归
// 比如 NODE_NUM NODE_ID 之类的
using Func = std::function<bool(Expr*)>;
void walkLeftImpl(
  Expr* curr_node,
  Func prev_func=nullptr,
  Func mid_func=nullptr,
  Func post_func=nullptr);

void walkRightImpl(
  Expr* curr_node,
  Func prev_func=nullptr,
  Func mid_func=nullptr,
  Func post_func=nullptr);

} // end namespace rvcc

#endif