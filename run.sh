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
    ./rvcc $input >tmp.s || exit
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

assert 0  0
assert 42 42
assert 41 41
assert 17 "1-8/(2*2)+3*6"