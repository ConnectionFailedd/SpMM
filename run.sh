mkdir -p build
cd build
cmake ..
make
cd ..
./build/TESTCASE_GEN -m 16 -n 16 -p 16 -d 0.1
./build/TESTCASE_GEN -m 32 -n 32 -p 32 -d 0.05
./build/TESTCASE_GEN -m 64 -n 64 -p 64 -d 0.02
./build/TESTCASE_GEN -m 128 -n 128 -p 128 -d 0.01
./build/TESTCASE_GEN -m 256 -n 256 -p 256 -d 0.005
./build/TESTCASE_GEN -m 512 -n 512 -p 512 -d 0.002
./build/TESTCASE_GEN -m 1024 -n 1024 -p 1024 -d 0.001
./build/SPMM -t 1
./build/SPMM -t 2
./build/SPMM -t 4
./build/SPMM -t 8
./build/SPMM -t 12
./build/SPMM -t 15