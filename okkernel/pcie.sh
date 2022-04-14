export CROSS_TOOLCHAINS=/home/zlx/tpucontest/toolchains_dir/
export LD_LIBRARY_PATH=/home/zlx/tpucontest/okkernel/lib/pcie/:$LD_LIBRARY_PATH
make okk
cd build/pcie
./load_firmware
cd -
