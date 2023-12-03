#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include "lexer.h"
#include "parser.h"
#include "token.h"

using namespace rvcc;

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "args num should be two");
    return -1;
  }
  // rvcc::Lexer lexer(argv[1]);
  
  // lexer.init();
  // while(lexer.getCurrToken().getType() != rvcc::TokenType::TOKEN_EOF && 
  //       lexer.getCurrToken().getType() != rvcc::TokenType::TOKEN_ILLEGAL) {
  //   if (lexer.getCurrToken().getType() == TokenType::TOKEN_NUM) {
  //     std::cout << lexer.getCurrToken().getValue() << std::endl;
  //   } else if (lexer.getCurrToken().getType() == TokenType::TOKEN_PUNCT) {
  //     std::cout << *(lexer.getCurrToken().getLoc()) << std::endl;
  //   }
  //   lexer.consumerToken();
  // }

  Parser parser(argv[1]);
  AstTree* ast = parser.parser_ast();

  printf(".globl main\n");
  printf("main:\n");
  ast->walk<AstTree::WalkOrderType::POST_ORDER>([](AstNode* curr_node) {
    if (curr_node->getType() == AstNodeType::NODE_ADD) {
      curr_node->getValue() = curr_node->getLeft()->getValue() + curr_node->getRight()->getValue();
      printf("  ld a1, 0(sp)\n");
      printf("  addi sp, sp, 8\n");
      printf("  ld a0, 0(sp)\n");
      printf("  addi sp, sp, 8\n");
      printf("  add a0, a0, a1\n");
      printf("  addi sp, sp, -8\n");
      printf("  sd a0, 0(sp)\n");
    } else if (curr_node->getType() == AstNodeType::NODE_SUB) {
      curr_node->getValue() = curr_node->getLeft()->getValue() - curr_node->getRight()->getValue();
      printf("  ld a1, 0(sp)\n");
      printf("  addi sp, sp, 8\n");
      printf("  ld a0, 0(sp)\n");
      printf("  addi sp, sp, 8\n");
      printf("  sub a0, a0, a1\n");
      printf("  addi sp, sp, -8\n");
      printf("  sd a0, 0(sp)\n");
    } else if (curr_node->getType() == AstNodeType::NODE_MUL) {
      curr_node->getValue() = curr_node->getLeft()->getValue() * curr_node->getRight()->getValue();
      printf("  ld a1, 0(sp)\n");
      printf("  addi sp, sp, 8\n");
      printf("  ld a0, 0(sp)\n");
      printf("  addi sp, sp, 8\n");
      printf("  mul a0, a0, a1\n");
      printf("  addi sp, sp, -8\n");
      printf("  sd a0, 0(sp)\n");
    } else if (curr_node->getType() == AstNodeType::NODE_DIV) {
      curr_node->getValue() = curr_node->getLeft()->getValue() / curr_node->getRight()->getValue();
      printf("  ld a1, 0(sp)\n");
      printf("  addi sp, sp, 8\n");
      printf("  ld a0, 0(sp)\n");
      printf("  addi sp, sp, 8\n");
      printf("  div a0, a0, a1\n");
      printf("  addi sp, sp, -8\n");
      printf("  sd a0, 0(sp)\n");
    } else if (curr_node->getType() == AstNodeType::NODE_NUM) {
      printf("  li a0, %d\n", curr_node->getValue());
      printf("  addi sp, sp, -8\n");
      printf("  sd a0, 0(sp)\n");
    } else {
      std::cout << "can't process current type: " << static_cast<int>(curr_node->getType()) << std::endl;
    }
  });
  
  printf("  ld a0, 0(sp)\n");
  printf("  addi sp, sp, 8\n");
  printf("  ret\n");
  return 0;
}