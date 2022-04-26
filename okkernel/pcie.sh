export CROSS_TOOLCHAINS=/home/zlx/tpu_op_contest_s1/toolchains_dir/
export LD_LIBRARY_PATH=/home/zlx/tpu_op_contest_s1/okkernel/lib/pcie/:$LD_LIBRARY_PATH
make okk
cd build/pcie
./load_firmware
cd -
