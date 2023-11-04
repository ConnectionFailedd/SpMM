import argparse
import scipy.sparse as sp
import numpy as np
import os

def generate_test_data(m, n, p, density, sparse_filename, dense_filename):
    coo_matrix = sp.random(m, n, density, format="coo", dtype=float)
    dense_matrix = np.random.rand(n, p)

    np.savetxt(sparse_filename, np.c_[coo_matrix.row, coo_matrix.col, coo_matrix.data], fmt="%d %d %f", header=f"{m} {n} {coo_matrix.row.size}", comments='')
    np.savetxt(dense_filename, dense_matrix, header=f"{n} {p}", comments='')

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Testcase generator.")

    parser.add_argument("-m", type=int, default=1024, help="Rows of CSR matrix.")
    parser.add_argument("-n", type=int, default=1024, help="Columns of CSR matrix.")
    parser.add_argument("-p", type=int, default=1024, help="Columns of dense matrix.")
    parser.add_argument("--density", type=float, default=0.01, help="Density of CSR matrix.")

    args = parser.parse_args()

    os.system('mkdir -p testcases')
    generate_test_data(args.m, args.n, args.p, args.density, f'testcases/csr_{args.m}x{args.n}_{args.density}', f'testcases/dense_{args.n}x{args.p}_{args.density}')