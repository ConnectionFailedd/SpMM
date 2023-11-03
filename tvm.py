
import scipy.sparse as sp
import numpy as np
import argparse

parser = argparse.ArgumentParser(description="TVM 测试")

parser.add_argument("--sparse-filename", default="sparse_matrix.txt", help="稀疏矩阵的保存路径")
parser.add_argument("--dense-filename", default="dense_matrix.txt", help="密集矩阵的保存路径")

args = parser.parse_args()

with open(args.sparse_filename, "r") as sparseFile:
    lines = sparseFile.read().splitlines()
    m, n = [eval(token) for token in lines[0].split(' ')]
    nnz = eval(lines[1])
    print(m)
    print(nnz)

    row = np.array([eval(token) for token in lines[2].strip().split(' ')])
    col = np.array([eval(token) for token in lines[3].strip().split(' ')])
    data = np.array([eval(token) for token in lines[4].strip().split(' ')])

    coo_matrix = sp.coo_matrix((data, (row, col)))
    print(coo_matrix)
