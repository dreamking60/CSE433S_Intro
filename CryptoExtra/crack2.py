import math
import random

def gcd(a, b):
    while b:
        a, b = b, a % b
    return a

def pollards_rho(n):
    if n % 2 == 0:
        return 2
    x = random.randint(2, n-1)
    y = x
    c = random.randint(1, n-1)
    g = 1

    while g == 1:
        x = (x * x + c) % n
        y = (y * y + c) % n
        y = (y * y + c) % n
        g = gcd(abs(x - y), n)
    
    return g

n = 510143758735509025530880200653196460532653147
factor = pollards_rho(n)
print(f"{factor}")