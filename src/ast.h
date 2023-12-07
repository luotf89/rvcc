#ifndef __AST_H
#define __AST_H

#include "lexer.h"
#include "token.h"
#include <cstdint>
#include <functional>
#include <limits>

namespace rvcc {
  enum class AstNodeType{
    NODE_ADD,             // +
    NODE_SUB,             // -
    NODE_MUL,             // *
    NODE_DIV,             // /
    NODE_NUM,             // number
    NODE_NEG,             // '-'          
    NODE_EQ,              // ==
    NODE_NE,              // !=
    NODE_LT,              // <
    NODE_LE,              // <=
    NODE_ID,              // identify
    NODE_ASSIGN,          // =
    NODE_STMT,
    Node_ILLEGAL          // illegal
  };
  class AstNode{
    public:
      AstNode() {
        type_ = AstNodeType::Node_ILLEGAL;
        val_ = INT32_MIN;
        left_ = nullptr;
        right_ = nullptr;
        next_ = nullptr;
      }
      AstNode(AstNodeType type,
              int val = INT32_MIN,
              AstNode* left = nullptr,
              AstNode* right = nullptr,
              AstNode* next = nullptr,
              char var = '\0'):
        type_(type), val_(val),left_(left),
        right_(right), next_(next), var_(var) {}
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
      char& getVar() {
        return var_;
      }
    private:
      AstNodeType type_;
      int val_;
      AstNode* left_;
      AstNode* right_;
      AstNode* next_;
      char var_;
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