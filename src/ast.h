#ifndef __AST_H
#define __AST_H
#include "lexer.h"
#include "token.h"
#include <functional>
namespace rvcc {
  enum class AstNodeType{
    NODE_ADD = 0,             // add
    NODE_SUB = 1,             // sub
    NODE_MUL = 2,             // mul
    NODE_DIV = 3,             // div
    NODE_NUM = 4,             // number
    NODE_NEG = 5,             // negative          
    NODE_EQ = 6,              // equal
    NODE_NE = 7,              // not equal
    NODE_LT = 8,              // less than
    NODE_LE = 9,            // less equal
    Node_ILLEGAL = 12         // illegal
  };
  class AstNode{
    public:
      AstNode() {
        type_ = AstNodeType::Node_ILLEGAL;
        val_ = 0;
        left_ = nullptr;
        right_ = nullptr;
      }
      AstNode(AstNodeType type, int val, AstNode* left, AstNode* right):
        type_(type), val_(val),left_(left),right_(right) {}
      AstNodeType& getType() {
        return type_;
      }
      int& getValue() {
        return val_;
      }
      AstNode*& getLeft() {
        return left_;
      }
      AstNode*& getRight() {
        return right_;
      }
    private:
      AstNodeType type_;
      int val_;
      AstNode* left_;
      AstNode* right_;
  };

  class AstTree{
    public:
      enum class WalkOrderType{
        PRE_ORDER = 0,
        IN_ORDER = 1,
        POST_ORDER = 2
      };
      AstNode*& getRoot() {
        return root;
      }
      using Func = std::function<void(AstNode*)>;
      template<WalkOrderType walk_order>
      void walkImpl(Func func, AstNode*curr_node) {
        if (!curr_node) {
          return;
        }
        if constexpr (walk_order == WalkOrderType::PRE_ORDER) {
          func(curr_node);
        }
        walkImpl<walk_order>(func, curr_node->getLeft());
        if constexpr(walk_order == WalkOrderType::IN_ORDER) {
          func(curr_node);
        }
        walkImpl<walk_order>(func, curr_node->getRight());
        if constexpr(walk_order == WalkOrderType::POST_ORDER) {
          func(curr_node);
        }
      }
      void computer(Func func) {
        // walkLeftImpl<walk_order>(func, root);
        walkImpl<WalkOrderType::POST_ORDER>(func, root);
      }
    private:
      AstNode* root;
  };
}
#endif