for size in 32 64 128 256 512 1024
do
    for density in 0.001 0.01 0.1
    do
        python3 testcase_gen.py -m $size -n $size -p $size --density $density;
        for threads in 1 2 4 8
        do
            ./build/SPMM -t $threads -m $size -n $size -p $size -d $density;
        done
    done
done
