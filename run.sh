rm build -rf
mkdir build
cd build
cmake -S ../ -G Ninja
ninja
cd src
RISCV=~/software/riscv

assert() {
    expect="$1"
    input="$2"
    ./rvcc "$input" >tmp.s || exit
    riscv64-linux-gnu-gcc -static -o tmp tmp.s
    $RISCV/bin/qemu-riscv64 -L $RISCV/sysroot ./tmp
    actual="$?"
    # 注意 shell 脚本 "[" 和 "]" 用作test 需要空格
    if [ "$actual" = "$expect" ]; then
        echo "$input => $actual"
    else
        echo "$input => $expect expect, but got $actual"
        exit 1
    fi
}



# assert 期待值 输入值
# [1] 返回指定数值
assert 0 0
assert 42 42

# [2] 支持+ -运算符
assert 34 '12-34+56'

# [3] 支持空格
assert 41 " 12 + 34 - 5 "

# [5] 支持* / ()运算符
assert 47 '5+6*7'
assert 15 '5*(9-6)'
assert 17 '1-8/(2*2)+3*6'

# [6] 支持一元运算的+ -
assert 10 '-10+20'
assert 10 '- -10'
assert 10 '- - +10'
assert 48 '------12*+++++----++++++++++4'
# 无法返回负值，赋值为byte的补码
#assert "-17" "-(1-8/(2*2)+3*6)"

# 如果运行正常未提前退出，程序将显示OK
echo OK