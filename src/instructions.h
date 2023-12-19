#ifndef __INSTRUCTION_H
#define __INSTRUCTION_H
#include <cstdint>
#include <stdio.h>

void mv_(const char* dst, const char* src);
void add_(const char* dst, const char* src1, const char* src2);
void sub_(const char* dst, const char* src1, const char* src2);
void addi_(const char* dst, const char* src, int val);
void mul_(const char* dst, const char* src1, const char* src2);
void div_(const char* dst, const char* src1, const char* src2);
void xor_(const char* dst, const char* src1, const char* src2);
void xori_(const char* dst, const char* src1, int src2);
void seqz_(const char* dst, const char* src);
void snez_(const char* dst, const char* src);
void slt_(const char* dst, const char* src1, const char* src2);
void li_(const char* dst, int val);
void sd_(const char* src, const char* dst, int offset);
void ld_(const char* dst, const char* src, int offset);
void neg_(const char* dst, const char* src);
void ret_();

void push_(const char* reg);
void pop_(const char* reg);
void start_();
void goto_return_label_();
void return_label_();

void goto_else_label_(const char* reg, std::uint32_t unique_id);
void else_label_(std::uint32_t unique_id);
void end_label_(std::uint32_t unique_id);

#endif