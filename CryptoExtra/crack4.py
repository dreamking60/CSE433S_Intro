import math
import numpy as np
from sympy import primerange
from scipy.sparse import csr_matrix
from scipy.sparse.linalg import lsmr
from sympy.ntheory import factorint

def quadratic_sieve(n):
    # Step 1: 计算质数基的大小
    def calculate_factor_base_size(n):
        log_n = math.log(n)
        base_size = int(math.exp(0.5 * math.sqrt(log_n * math.log(log_n))))
        return base_size

    # Step 2: 检查一个数是否为 B-smooth 数
    def is_B_smooth(x, n, factor_base):
        y = (x * x) % n
        for p in factor_base:
            while y % p == 0:
                y //= p
        return y == 1

    # Step 3: 构建稀疏矩阵
    def build_sparse_matrix(smooth_numbers, factor_base):
        num_smooth = len(smooth_numbers)
        num_factors = len(factor_base)

        data = []
        row_ind = []
        col_ind = []

        for i, (x, y) in enumerate(smooth_numbers):
            for j, p in enumerate(factor_base):
                count = 0
                while y % p == 0:
                    y //= p
                    count += 1
                if count % 2 != 0:
                    data.append(1)
                    row_ind.append(i)
                    col_ind.append(j)

        sparse_matrix = csr_matrix((data, (row_ind, col_ind)), shape=(num_smooth, num_factors))
        return sparse_matrix

    # Step 4: 求解稀疏矩阵
    def solve_sparse_matrix(sparse_matrix):
        solution = lsmr(sparse_matrix, np.zeros(sparse_matrix.shape[0]))
        return solution

    # Step 5: 使用 GCD 找因子
    def find_factor(x, y, n):
        factor = math.gcd(x - y, n)
        if factor != 1 and factor != n:
            return factor
        return None

    print(f"开始分解 n = {n}")

    # Step 1: 计算质数基
    base_size = calculate_factor_base_size(n)
    factor_base = list(primerange(2, base_size))
    print(f"质数基大小: {base_size}, 前10个质数基: {factor_base[:10]}")

    # Step 2: 寻找 B-smooth 数
    smooth_numbers = []
    x_start = int(math.sqrt(n)) + 1
    print("开始寻找 B-smooth 数...")

    # 扩展 x 的范围以找到足够多的 B-smooth 数
    for x in range(x_start, x_start + 1000000):  # 适当增加搜索范围
        if is_B_smooth(x, n, factor_base):
            smooth_numbers.append((x, (x*x) % n))

    print(f"找到 {len(smooth_numbers)} 个 B-smooth 数")

    if len(smooth_numbers) < len(factor_base):
        print("B-smooth 数量不足，无法继续")
        return None

    # Step 3: 构建因子矩阵
    sparse_matrix = build_sparse_matrix(smooth_numbers, factor_base)
    print(f"因子矩阵大小: {sparse_matrix.shape}")

    # Step 4: 解稀疏矩阵
    solution = solve_sparse_matrix(sparse_matrix)
    print("稀疏矩阵求解完成")

    # Step 5: 通过 GCD 找因子
    for x, y in smooth_numbers:
        factor = find_factor(x, int(math.sqrt(y)), n)
        if factor:
            print(f"找到因子: {factor}")
            return factor

    print("未找到因子")
    return None

# 测试: 分解一个150位的大数
if __name__ == "__main__":
    # 生成一个150位的合数作为示例
    n = 510143758735509025530880200653196460532653147  # 注意：示例使用较小的合数，方便测试

    factor = quadratic_sieve(n)
    if factor:
        print(f"n 的因子是: {factor}")
    else:
        print("未成功分解 n")