#ifndef __INSTRUCTION_H
#define __INSTRUCTION_H
#include <atomic>
#include <cstdint>
#include <stdio.h>

extern std::atomic_int depth;

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
void sd_(const char* reg, const char* addr, int offset);
void ld_(const char* reg, const char* addr, int offset);
void neg_(const char* dst, const char* src);
void call_(const char* func_name);
void ret_();

void push_(const char* reg);
void pop_(const char* reg);
void start_();
void goto_return_label_();
void return_label_();

void goto_else_label_(const char* reg, std::uint32_t unique_id);
void else_label_(std::uint32_t unique_id);
void branch_end_label_(std::uint32_t unique_id);
void loop_end_label_(std::uint32_t unique_id);
void goto_end_label_(std::uint32_t unique_id);
void loop_begin_label_(std::uint32_t unique_id);
void goto_loop_begin_label_(std::uint32_t unique_id);
void goto_loop_end_label_(const char* reg, std::uint32_t unique_id);

#endif