#include "instructions.h"


void mv_(const char* dst, const char* src) {
    printf("  mv %s, %s\n", dst, src);
};
void add_(const char* dst, const char* src1, const char* src2) {
    printf("  add %s, %s, %s\n", dst, src1, src2);
};
void sub_(const char* dst, const char* src1, const char* src2) {
    printf("  sub %s, %s, %s\n", dst, src1, src2);
};
void addi_(const char* dst, const char* src, int val) {
    printf("  addi %s, %s, %d\n", dst, src, val);
};
void mul_(const char* dst, const char* src1, const char* src2) {
    printf("  mul %s, %s, %s\n", dst, src1, src2);
};
void div_(const char* dst, const char* src1, const char* src2) {
    printf("  div %s, %s, %s\n", dst, src1, src2);
};
void xor_(const char* dst, const char* src1, const char* src2) {
    printf("  xor %s, %s, %s\n", dst, src1, src2);
};
void xori_(const char* dst, const char* src1, int src2) {
    printf("  xori %s, %s, %d\n", dst, src1, src2);
};
void seqz_(const char* dst, const char* src) {
    printf("  seqz %s, %s\n", dst, src);
};
void snez_(const char* dst, const char* src) {
    printf("  snez %s, %s\n", dst, src);
};
void slt_(const char* dst, const char* src1, const char* src2) {
    printf("  slt %s, %s, %s\n", dst, src1, src2);
};
void li_(const char* dst, int val) {
    printf("  li %s, %d\n", dst, val);
};
void sd_(const char* dst, const char* src, int offset) {
    printf("  sd %s, %d(%s)\n", dst, offset, src);
};
void ld_(const char* dst, const char* src, int offset) {
    printf("  ld %s, %d(%s)\n", dst, offset, src);
};
void neg_(const char* dst, const char* src) {
    printf("  neg %s, %s\n", dst, src);
};

void ret_() {
    printf("  ret\n");
};


void push_(const char* reg) {
    printf("  addi sp, sp, -8\n");
    printf("  sd %s, 0(sp)\n", reg);
}; 
void pop_(const char* reg) {
    printf("  ld %s, 0(sp)\n", reg);
    printf("  addi sp, sp, 8\n");
};

void start_() {
    printf(".globl main\n");
    printf("main:\n");
};

void goto_return_lable_() {
    printf("  j .L.return\n");
}

void return_lable_() {
    printf(".L.return:\n");
}