echo "Checking Installation Of compiler and qemu"
/opt/riscv/bin/riscv64-unknown-linux-gnu-gcc -march=rv32i -mabi=ilp32 test_example/hello.c -static -o test_example/hello
/usr/local/bin/qemu-riscv32 ./test_example/hello
rm test_example/hello

echo "Available CPU version and Qemu version"
/opt/riscv/bin/riscv64-unknown-linux-gnu-gcc -print-multi-lib
ls /usr/local/bin | grep qemu