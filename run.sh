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
assert 0 '{ return 0; }'
assert 42 '{ return 42; }'

# [2] 支持+ -运算符
assert 34 '{ return 12-34+56; }'

# [3] 支持空格
assert 41 '{ return  12 + 34 - 5 ; }'

# [5] 支持* / ()运算符
assert 47 '{ return 5+6*7; }'
assert 15 '{ return 5*(9-6); }'
assert 17 '{ return 1-8/(2*2)+3*6; }'

# [6] 支持一元运算的+ -
assert 10 '{ return -10+20; }'
assert 10 '{ return - -10; }'
assert 10 '{ return - - +10; }'
assert 48 '{ return ------12*+++++----++++++++++4; }'

# [7] 支持条件运算符
assert 0 '{ return 0==1; }'
assert 1 '{ return 42==42; }'
assert 1 '{ return 0!=1; }'
assert 0 '{ return 42!=42; }'
assert 1 '{ return 0<1; }'
assert 0 '{ return 1<1; }'
assert 0 '{ return 2<1; }'
assert 1 '{ return 0<=1; }'
assert 1 '{ return 1<=1; }'
assert 0 '{ return 2<=1; }'
assert 1 '{ return 1>0; }'
assert 0 '{ return 1>1; }'
assert 0 '{ return 1>2; }'
assert 1 '{ return 1>=0; }'
assert 1 '{ return 1>=1; }'
assert 0 '{ return 1>=2; }'
assert 1 '{ return 5==2+3; }'
assert 0 '{ return 6==4+3; }'
assert 1 '{ return 0*9+5*2==4+4*(6/3)-2; }'

# [9] 支持;分割语句
assert 3 '{ 1; 2;return 3; }'
assert 12 '{ 12+23;12+99/3;return 78-66; }'

# [10] 支持单字母变量
assert 3 '{ int a=3;return a; }'
assert 8 '{ int a=3,z=5;return a+z; }'
assert 6 '{ int a,b; a=b=3;return a+b; }'
assert 5 '{ int a=3,b=4; a=1;return a+b; }'

# [11] 支持多字母变量
assert 3 '{ int foo=3;return foo; }'
assert 74 '{ int foo2=70; int bar4=4;return foo2+bar4; }'

# [12] 支持return
assert 1 '{ return 1; 2; 3; }'
assert 2 '{ 1; return 2; 3; }'
assert 3 '{ 1; 2; return 3; }'

# [13] 支持{...}
assert 3 '{ {1; {2;} return 3;} }'

# [14] 支持空语句
assert 5 '{ ;;; return 5; }'

# [15] 支持if语句
assert 3 '{ if (0) return 2; return 3; }'
assert 3 '{ if (1-1) return 2; return 3; }'
assert 2 '{ if (1) return 2; return 3; }'
assert 2 '{ if (2-1) return 2; return 3; }'
assert 4 '{ if (0) { 1; 2; return 3; } else { return 4; } }'
assert 3 '{ if (1) { 1; 2; return 3; } else { return 4; } }'

# [16] 支持for语句
assert 55 '{ int i=0; int j=0; for (i=0; i<=10; i=i+1) j=i+j; return j; }'
assert 3 '{ for (;;) {return 3;} return 5; }'

# [17] 支持while语句
assert 10 '{ int i=0; while(i<10) { i=i+1; } return i; }'

# [20] 支持一元& *运算符
assert 3 '{ int x=3; return *&x; }'
assert 3 '{ int x=3; int *y=&x; int **z=&y; return **z; }'
assert 5 '{ int x=3; int *y=&x; *y=5; return x; }'

# [21] 支持指针的算术运算
assert 3 '{ int x=3; int y=5; return *(&y+1); }'
assert 5 '{ int x=3; int y=5; return *(&x-1); }'
assert 7 '{ int x=3; int y=5; *(&y+1)=7; return x; }'
assert 7 '{ int x=3; int y=5; *(&x-1)=7; return y; }'

# [22] 支持int关键字
assert 8 '{ int x, y; x=3; y=5; return x+y; }'
assert 8 '{ int x=3, y=5; return x+y; }'

# 如果运行正常未提前退出，程序将显示OK
echo OK