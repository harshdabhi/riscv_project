#!/bin/bash

#<<<================================================================================================================================================================>>#

echo "accelerated SHA encryption test begins"
/opt/riscv/bin/riscv64-unknown-linux-gnu-gcc -march=rv32i_zbb_zbc_zbkb_zbkc_zbkx_zknd_zkne_zknh_zksed_zksh -mabi=ilp32 -D USE_RISCV_CRYPTO_EXT sha_dir/sha_filesv2/testv2.c -static -o sha_acc
/usr/local/bin/qemu-riscv32 ./sha_acc

#<<<================================================================================================================================================================>>#

echo "Moving files from source to SHA result directory"
mv sha256_accelerated_results.csv sha_result
mv sha_acc sha_result


#<<<================================================================================================================================================================>>#

# echo "Removing files generated during test"
# rm temp_data.bin
# rm temp_data.enc

#<<<================================================================================================================================================================>>#

echo "Standard SHA encryption test begins"
/opt/riscv/bin/riscv64-unknown-linux-gnu-gcc -march=rv32i -mabi=ilp32 sha_dir/sha_filesv2/testv2.c -static -o sha
/usr/local/bin/qemu-riscv32 ./sha

#<<<================================================================================================================================================================>>#

# echo "Removing files generated during test"
# rm temp_data.bin
# rm temp_data.enc

#<<<================================================================================================================================================================>>#

echo "Moving files from source to SHA result directory"
mv sha256_standard_results.csv sha_result
mv sha sha_result

#<<<================================================================================================================================================================>>#
