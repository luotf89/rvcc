#include <stdio.h>
#include <stdlib.h>
#include <map>
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

  [&](){
    auto push_ = [](const char* reg) {
      printf("  addi sp, sp, -8\n");
      printf("  sd %s, 0(sp)\n", reg);
    }; 
    auto pop_ = [](const char* reg) {
      printf("  ld %s, 0(sp)\n", reg);
      printf("  addi sp, sp, 8\n");
    };
    auto mv_ = [](const char* reg1, const char*reg2) {
      printf("  mv %s, %s\n", reg1, reg2);
    };
    auto add_ = [](const char* reg1, const char* reg2, const char* reg3) {
      printf("  add %s, %s, %s\n", reg1, reg2, reg3);
    };
    auto sub_ = [](const char* reg1, const char* reg2, const char* reg3) {
      printf("  sub %s, %s, %s\n", reg1, reg2, reg3);
    };
    auto addi_ = [](const char* reg1, const char* reg2, int val) {
      printf("  addi %s, %s, %d\n", reg1, reg2, val);
    };
    auto mul_ = [](const char* reg1, const char* reg2, const char* reg3) {
      printf("  mul %s, %s, %s\n", reg1, reg2, reg3);
    };
    auto div_ = [](const char* reg1, const char* reg2, const char* reg3) {
      printf("  div %s, %s, %s\n", reg1, reg2, reg3);
    };
    auto xor_ = [](const char* reg1, const char* reg2, const char* reg3) {
      printf("  xor %s, %s, %s\n", reg1, reg2, reg3);
    };
    auto xori_ = [](const char* reg1, const char* reg2, int val) {
      printf("  xori %s, %s, %d\n", reg1, reg2, val);
    };
    auto seqz_ = [](const char* reg1, const char* reg2) {
      printf("  seqz %s, %s\n", reg1, reg2);
    };
    auto snez_ = [](const char* reg1, const char* reg2) {
      printf("  snez %s, %s\n", reg1, reg2);
    };
    auto slt_ = [](const char* reg1, const char* reg2, const char* reg3) {
      printf("  slt %s, %s, %s\n", reg1, reg2, reg3);
    };
    auto li_ = [](const char* reg, int val) {
      printf("  li %s, %d\n", reg, val);
    };
    auto sd_ = [](const char* reg1, const char* reg2, int offset) {
      printf("  sd %s, %d(%s)\n", reg1, offset, reg2);
    };
    auto ld_ = [](const char* reg1, const char* reg2, int offset) {
      printf("  ld %s, %d(%s)\n", reg1, offset, reg2);
    };
    auto neg_ = [](const char* reg1, const char* reg2) {
      printf("  neg %s, %s\n", reg1, reg2);
    };
    auto start_ = []() {
      printf(".globl main\n");
      printf("main:\n");
    };
    auto ret_ = []() {
      printf("  ret\n");
    };
    
    start_();
    push_("fp");
    mv_("fp", "sp");
    addi_("sp", "sp", -8*26);
    ast->computer([&](AstNode* curr_node) {
      if (curr_node->getType() == AstNodeType::NODE_ADD) {
        pop_("a1");
        pop_("a0");
        add_("a0", "a0", "a1");
        push_("a0");
      } else if (curr_node->getType() == AstNodeType::NODE_SUB) {
        pop_("a1");
        pop_("a0");
        sub_("a0", "a0", "a1");
        push_("a0");
      } else if (curr_node->getType() == AstNodeType::NODE_MUL) {
        pop_("a1");
        pop_("a0");
        mul_("a0", "a0", "a1");
        push_("a0");
      } else if (curr_node->getType() == AstNodeType::NODE_DIV) {
        pop_("a1");
        pop_("a0");
        div_("a0", "a0", "a1");
        push_("a0");
      } else if (curr_node->getType() == AstNodeType::NODE_NUM) {
        li_("a0", curr_node->getValue());
        push_("a0");
      } else if (curr_node->getType() == AstNodeType::NODE_NEG) {
        pop_("a0");
        neg_("a0", "a0");
        push_("a0");
      } else if (curr_node->getType() == AstNodeType::NODE_EQ) {
        pop_("a1");
        pop_("a0");
        xor_("a0", "a0", "a1");
        seqz_("a0", "a0");
        push_("a0");
      } else if (curr_node->getType() == AstNodeType::NODE_NE) {
        pop_("a1");
        pop_("a0");
        xor_("a0", "a0", "a1");
        snez_("a0", "a0");
        push_("a0");
      } else if (curr_node->getType() == AstNodeType::NODE_LT) {
        pop_("a1");
        pop_("a0");
        slt_("a0", "a0", "a1");
        push_("a0");
      } else if (curr_node->getType() == AstNodeType::NODE_LE) {
        pop_("a1");
        pop_("a0");
        slt_("a0", "a1", "a0");
        xori_("a0", "a0", 1);
        push_("a0");
      } else if (curr_node->getType() == AstNodeType::NODE_ID) {
        int offset = -(curr_node->getVar() - 'a' + 1) * 8;
        ld_("a0", "fp", offset);
        push_("a0");
      } else if (curr_node->getType() == AstNodeType::NODE_ASSIGN) {
        int offset = -(curr_node->getLeft()->getVar() - 'a' + 1) * 8;
        pop_("a0");
        pop_("a1");
        sd_("a0", "fp", offset);
        push_("a0");
      } else {
        std::cout << "can't process current type: " << static_cast<int>(curr_node->getType()) << std::endl;
      }
    });
    pop_("a0");
    mv_("sp", "fp");
    pop_("fp");
    ret_();
  }();

  // [&]() {
  //   std::map<char, int> var_maps;
  //   ast->computer([&](AstNode* curr_node) {
  //     auto getChildValue = [&] (AstNode* child_node) {
  //       int val;
  //       if (child_node->getType() == AstNodeType::NODE_ID) {
  //         if(var_maps.count(child_node->getVar()) == 0) {
  //           std::cout << "current id is used before assign: " << child_node->getVar() << std::endl;
  //           exit(1);
  //         }
  //         val =  var_maps[child_node->getVar()];
  //       } else {
  //         val = child_node->getValue();
  //       }
  //       return val;
  //     };

  //     if (curr_node->getType() == AstNodeType::NODE_ADD) {
  //       curr_node->getValue() = getChildValue(curr_node->getLeft()) + getChildValue(curr_node->getRight());
  //     } else if (curr_node->getType() == AstNodeType::NODE_SUB) {
  //       curr_node->getValue() = getChildValue(curr_node->getLeft()) - getChildValue(curr_node->getRight());
  //     } else if (curr_node->getType() == AstNodeType::NODE_MUL) {
  //       curr_node->getValue() = getChildValue(curr_node->getLeft()) * getChildValue(curr_node->getRight());
  //     } else if (curr_node->getType() == AstNodeType::NODE_DIV) {
  //       curr_node->getValue() = getChildValue(curr_node->getLeft()) / getChildValue(curr_node->getRight());
  //     } else if (curr_node->getType() == AstNodeType::NODE_ASSIGN) {
  //       var_maps[curr_node->getLeft()->getVar()] = curr_node->getRight()->getValue();
  //       curr_node->getVar() = curr_node->getLeft()->getVar();
  //       curr_node->getValue() = curr_node->getRight()->getValue();
  //       curr_node->getLeft()->getValue() = curr_node->getValue();
  //     } else if (curr_node->getType() == AstNodeType::NODE_NUM) {
  //     } else if (curr_node->getType() == AstNodeType::NODE_ID) {
  //       if (var_maps.count(curr_node->getVar()) != 0) {
  //         curr_node->getValue() = var_maps[curr_node->getVar()];
  //       }
  //     } else if (curr_node->getType() == AstNodeType::NODE_NEG) {
  //       curr_node->getValue() = getChildValue(curr_node->getRight()) * (-1);
  //     } else if (curr_node->getType() == AstNodeType::NODE_EQ) {
  //       curr_node->getValue() = getChildValue(curr_node->getLeft()) == getChildValue(curr_node->getRight());
  //     } else if (curr_node->getType() == AstNodeType::NODE_NE) {
  //       curr_node->getValue() = getChildValue(curr_node->getLeft()) != getChildValue(curr_node->getRight());
  //     } else if (curr_node->getType() == AstNodeType::NODE_LT) {
  //       curr_node->getValue() = getChildValue(curr_node->getLeft()) < getChildValue(curr_node->getRight());
  //     } else if (curr_node->getType() == AstNodeType::NODE_LE) {
  //       curr_node->getValue() = getChildValue(curr_node->getLeft()) <= getChildValue(curr_node->getRight());
  //     } else {
  //       std::cout << "can't process current type: " << static_cast<int>(curr_node->getType()) << std::endl;
  //     }
  //   });
  //   AstNode* prev = ast->getRoot();
  //   AstNode* curr = ast->getRoot()->getNext();
  //   while(curr != nullptr) {
  //     curr = curr->getNext();
  //     prev = prev->getNext();
  //   }
  //   std::cout << "result: " << prev->getLeft()->getValue() << std::endl;
  // }();

  return 0;
}