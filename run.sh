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
for size in 16 32 64 128 256 512 1024
do
    for threads in 1 2 4 8
    do
        ./build/SPMM -t $threads -m $size -n $size -p $size;
    done
done
