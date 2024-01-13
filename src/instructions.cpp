#include "instructions.h"

std::atomic_int depth = 0;

void mv_(const char* dst, const char* src) {
    printf("  # 将寄存器 %s 的值赋值给寄存器 %s\n", src, dst);
    printf("  mv %s, %s\n", dst, src);
};
void add_(const char* dst, const char* src1, const char* src2) {
    printf("  # %s + %s，结果写入 %s\n", src1, src2, dst);
    printf("  add %s, %s, %s\n", dst, src1, src2);
};
void sub_(const char* dst, const char* src1, const char* src2) {
    printf("  # %s - %s，结果写入 %s\n", src1, src2, dst);
    printf("  sub %s, %s, %s\n", dst, src1, src2);
};
void addi_(const char* dst, const char* src, int val) {
    printf("  # %s + %d，结果写入 %s\n", src, val, dst);
    printf("  addi %s, %s, %d\n", dst, src, val);
};
void mul_(const char* dst, const char* src1, const char* src2) {
    printf("  # %s * %s，结果写入 %s\n", src1, src2, dst);
    printf("  mul %s, %s, %s\n", dst, src1, src2);
};
void div_(const char* dst, const char* src1, const char* src2) {
    printf("  # %s / %s，结果写入 %s\n", src1, src2, dst);
    printf("  div %s, %s, %s\n", dst, src1, src2);
};
void xor_(const char* dst, const char* src1, const char* src2) {
    printf("  # %s 异或 %s 结果写入 %s\n", src1, src2, dst);
    printf("  xor %s, %s, %s\n", dst, src1, src2);
};
void xori_(const char* dst, const char* src, int val) {
    printf("  # %s 异或 %d 结果写入 %s\n", src, val, dst);
    printf("  xori %s, %s, %d\n", dst, src, val);
};
void seqz_(const char* dst, const char* src) {
    printf("  # 寄存器 %s 和 0 相等的结果 写入寄存器 %s\n", src, dst);
    printf("  seqz %s, %s\n", dst, src);
};
void snez_(const char* dst, const char* src) {
    printf("  # 寄存器 %s 和 0 不相等的结果 写入寄存器 %s\n", src, dst);
    printf("  snez %s, %s\n", dst, src);
};
void slt_(const char* dst, const char* src1, const char* src2) {
    printf("  # 寄存器 %s 小于寄存器 %s 的结果 写入寄存器 %s\n", src1,  src2, dst);
    printf("  slt %s, %s, %s\n", dst, src1, src2);
};
void li_(const char* dst, int val) {
    printf("  # 将%d加载到 %s 中\n", val, dst);
    printf("  li %s, %d\n", dst, val);
};
void sd_(const char* reg, const char* addr, int offset) {
    printf("  # 将寄存器 %s 保存到 (%d)%s 地址中\n", reg, offset, addr);
    printf("  sd %s, %d(%s)\n", reg, offset, addr);
};
void ld_(const char* reg, const char* addr, int offset) {
    printf("  # 将地址 (%d)%s 中的值 加载到寄存器 %s 中\n", offset, addr, reg);
    printf("  ld %s, %d(%s)\n", reg, offset, addr);
};
void neg_(const char* dst, const char* src) {
    printf("  # 对寄存器 %s 值进行取反后写入 寄存器 %s\n", src, dst);
    printf("  neg %s, %s\n", dst, src);
};

void call_(const char* func_name) {
    printf("  # 调用函数%s\n", func_name);
    printf("  call %s\n", func_name);
};

void ret_() {
    printf("  # 返回a0值给系统调用\n");
    printf("  ret\n");
};


void push_(const char* reg) {
    depth++;
    printf("  # 将 %s 值压栈\n", reg);
    printf("  addi sp, sp, -8\n");
    printf("  sd %s, 0(sp)\n", reg);
}; 
void pop_(const char* reg) {
    depth--;
    printf("  # 将 %s 值弹栈\n", reg);
    printf("  ld %s, 0(sp)\n", reg);
    printf("  addi sp, sp, 8\n");
};

void start_() {
    printf("# 定义全局main段\n");
    printf(".globl main\n");
    printf("\n# =====程序开始===============\n");
    printf("# main段标签，也是程序入口段\n");
    printf("main:\n");
};

void goto_return_label_() {
    printf("# 返回语句\n");
    printf("  # 跳转到.L.return段\n");
    printf("  j .L.return\n");
}

void return_label_() {
    printf("\n# =====程序结束===============\n");
    printf("# return段标签\n");
    printf(".L.return:\n");
}

void goto_else_label_(const char* reg, std::uint32_t unique_id) {
    printf("  # 若a0为0，则跳转到分支%d的.L.else.%d段\n", unique_id, unique_id);
    printf("  beqz %s, .L.else.%d\n", reg, unique_id);
}

void else_label_(std::uint32_t unique_id) {
    printf("\n# Else语句%d\n", unique_id);
    printf("# 分支%d的.L.else.%d段标签\n", unique_id, unique_id);
    printf(".L.else.%d:\n", unique_id);
}

void branch_end_label_(std::uint32_t unique_id) {
    printf("\n# 分支%d的.L.end.%d段标签\n", unique_id, unique_id);
    printf(".L.end.%d:\n", unique_id);
}

void loop_end_label_(std::uint32_t unique_id) {
    printf("\n# 循环%d的.L.end.%d段标签\n", unique_id, unique_id);
    printf(".L.end.%d:\n", unique_id);
}

void goto_end_label_(std::uint32_t unique_id) {
    printf("  # 跳转到分支%d的.L.end.%d段\n", unique_id, unique_id);
    printf("  j .L.end.%d\n", unique_id);
}

void loop_begin_label_(std::uint32_t unique_id) {
    printf("\n# 循环%d的.L.begin.%d段标签\n", unique_id, unique_id);
    printf(".L.begin.%d:\n", unique_id);
}

void goto_loop_begin_label_(std::uint32_t unique_id) {
    printf("  # 跳转到循环%d的.L.begin.%d段\n", unique_id, unique_id);
    printf("  j .L.begin.%d\n", unique_id);
}

void goto_loop_end_label_(const char* reg, std::uint32_t unique_id) {
    printf("  # 若 %s 为0，则跳转到循环%d的.L.end.%d段\n", reg, unique_id, unique_id);
    printf("  beqz %s, .L.end.%d\n", reg, unique_id);
}
