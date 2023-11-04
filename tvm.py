import tvm
from tvm import relay
import numpy as np
import argparse

parser = argparse.ArgumentParser(description="Testcase generator.")

parser.add_argument("-m", type=int, default=1024, help="Rows of CSR matrix.")
parser.add_argument("-n", type=int, default=1024, help="Columns of CSR matrix.")
parser.add_argument("-p", type=int, default=1024, help="Columns of dense matrix.")
parser.add_argument("--density", type=float, default=0.01, help="Density of CSR matrix.")

args = parser.parse_args()

os.system('mkdir -p testcases')
generate_test_data(args.m, args.n, args.p, args.density, f'testcases/csr_{args.m}x{args.n}_{args.density}', f'testcases/dense_{args.n}x{args.p}_{args.density}')

# 创建稠密矩阵输入
m = 1024  # 稠密矩阵行数
n = 1024  # 稠密矩阵列数
A = np.random.rand(m, n).astype('float32')

# 创建CSR矩阵输入
row_ptr = np.array([0, 2, 3, 6], dtype='int32')  # 行指针
col_idx = np.array([0, 2, 1, 0, 2, 3], dtype='int32')  # 列索引
data = np.random.rand(6).astype('float32')  # 数据值

# 创建TVM上下文
target = 'llvm'
ctx = tvm.context(target, 0)

# 转换稠密矩阵为relay中的Tensor类型
dense_tensor = relay.Constant(tvm.nd.array(A, ctx))

# 转换CSR矩阵为relay中的SparseTensor类型
sparse_tensor = relay.sparse.SparseTensor(
    data=relay.Constant(tvm.nd.array(data, ctx)),
    indices=relay.Constant(tvm.nd.array(col_idx, ctx)),
    indptr=relay.Constant(tvm.nd.array(row_ptr, ctx)),
    shape=(m, n)
)

# 执行CSR矩阵和稠密矩阵的乘法
result = relay.nn.sparse_dense.matmul(sparse_tensor, dense_tensor)

# 创建计算图
func = relay.Function([], result)

# 编译计算图
with relay.build_config(opt_level=3):
    graph, lib, params = relay.build(func, target=target)

# 创建输出数据
output_tvm = tvm.nd.empty((m, n), ctx)

# 调用TVM运行CSR矩阵和稠密矩阵的乘法
exe = lib.time_evaluator(lib.entry_name, ctx, number=10)
print('Execution time: %.4f ms' % (exe().mean * 1000))