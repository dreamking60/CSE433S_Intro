import math
def fermat_factor(n):
    p = math.isqrt(n)
    if p * p == n:
        return p, p
    q = n - p * p
    while not math.isqrt(q) ** 2 == q:
        p -= 1
        q = n - p * p
    q = math.isqrt(q)
    return q - p, q + p

n = 510143758735509025530880200653196460532653147
factors = fermat_factor(n)
print(factors)