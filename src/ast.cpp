#include "ast.h"

using namespace rvcc;

AstNode* AstNode::binaryOp(AstNode* left, AstNode*right, AstNodeType type) {
  AstNode* res = new AstNode;
  res->getLeft() = left;
  res->getRight() = right;
  res->getType() = type;
  return res;
}