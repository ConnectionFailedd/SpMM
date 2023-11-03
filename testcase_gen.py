import argparse
import scipy.sparse as sp
import numpy as np

def generate_test_data(m, n, p, density, sparse_filename, dense_filename):
    coo_matrix = sp.random(m, n, density, format="coo", dtype=float)
    dense_matrix = np.random.rand(n, p)

    # # 将稀疏矩阵和密集矩阵分别保存到文件
    # with open(sparse_filename, "w") as f:
    #     f.write(f"{m} {n}\n")  # 写入矩阵的行数和列数
    #     f.write(f"{coo_matrix.row.size}\n")  # 写入非零元素的数量
    #     for row in coo_matrix.row:
    #         f.write(f"{row} ")
    #     f.write("\n")
    #     for col in coo_matrix.col:
    #         f.write(f"{col} ")
    #     f.write("\n")
    #     for data in coo_matrix.data:
    #         f.write(f"{data} ")
    #     f.write("\n")
    np.savetxt(sparse_filename, np.c_[coo_matrix.row, coo_matrix.col, coo_matrix.data], fmt="%d %d %f", header=f"{m} {n} {coo_matrix.row.size}", comments='')
    np.savetxt(dense_filename, dense_matrix, header=f"{n} {p}", comments='')

if __name__ == "__main__":
    # 添加命令行参数解析器
    parser = argparse.ArgumentParser(description="生成测试数据")

    parser.add_argument("-m", type=int, default=1000, help="矩阵的行数")
    parser.add_argument("-n", type=int, default=1000, help="矩阵的列数")
    parser.add_argument("-p", type=int, default=1000, help="密集矩阵的列数")
    parser.add_argument("--density", type=float, default=0.01, help="稀疏矩阵的密度")
    parser.add_argument("--sparse-filename", default="sparse_matrix.txt", help="稀疏矩阵的保存路径")
    parser.add_argument("--dense-filename", default="dense_matrix.txt", help="密集矩阵的保存路径")

    args = parser.parse_args()

    generate_test_data(args.m, args.n, args.p, args.density, args.sparse_filename, args.dense_filename)