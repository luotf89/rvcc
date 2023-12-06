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
    NODE_STMT = 10,
    Node_ILLEGAL = 12         // illegal
  };
  class AstNode{
    public:
      AstNode() {
        type_ = AstNodeType::Node_ILLEGAL;
        val_ = 0;
        left_ = nullptr;
        right_ = nullptr;
        next_ = nullptr;
      }
      AstNode(AstNodeType type, int val, AstNode* left, AstNode* right, AstNode* next):
        type_(type), val_(val),left_(left),right_(right), next_(next) {}
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
      AstNode*& getNext() {
        return next_;
      }
    private:
      AstNodeType type_;
      int val_;
      AstNode* left_;
      AstNode* right_;
      AstNode* next_;
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
        AstNode* curr_node = root;
        while (curr_node) {
          if(curr_node->getType() != AstNodeType::NODE_STMT) {
            std::cout << __FUNCTION__ << __LINE__ << "computer failed exepect current node is stmt" << std::endl;
          }
          walkImpl<WalkOrderType::POST_ORDER>(func, curr_node->getLeft());
          curr_node = curr_node->getNext();
        }
      }
    private:
      AstNode* root;
  };
}
#endif