import sys, getopt, random

if __name__ == '__main__':
    size = 1000
    density = 0.001
    try:
        opts, args = getopt.getopt(sys.argv[1 : ], 's:d:')
    except getopt.GetoptError:
        print('Usage: python3 testcase_gen.py -s <size> -d <density>')
        sys.exit(2)
    for opt, arg in opts:
        if opt == '-s':
            size = eval(arg)
        elif opt == '-d':
            density = eval(arg)

    with open('dense', 'bw') as ofile:
        ofile.write(size.to_bytes(4))
        for i in range(0, size):
            for j in range(0, size):
                ofile.write(random.randint(0, 1 << 32 - 1).to_bytes(4))

    with open('csr', 'bw') as ofile:
        coo = list[tuple[int, int, int]]()
        for i in range(0, size):
            for j in range(0, size):
                if(random.random() < density):
                    coo.append((random.randint(0, 1 << 32 - 1), i, j))
        ofile.write(len(coo).to_bytes(4))
        coo.sort(key = lambda x: x[1])
        for entry in coo:
            ofile.write(entry[0].to_bytes(4))
            ofile.write(entry[1].to_bytes(4))
            ofile.write(entry[2].to_bytes(4))
