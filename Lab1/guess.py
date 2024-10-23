import time
import random
from Crypto.Cipher import AES
from binascii import unhexlify

#Plaintext: 255044462d312e350a25d0d4c5d80a34
#Ciphertext: d06bf9d0dab8e8ef880660d2af65aa82
#IV: 09080706050403020100A2B2C2D2E2F2

P = bytes.fromhex("255044462d312e350a25d0d4c5d80a34")
C = bytes.fromhex("d06bf9d0dab8e8ef880660d2af65aa82")
IV = bytes.fromhex("09080706050403020100A2B2C2D2E2F2")

print(P)
random1 = 1524017329
random2 = 1524024529

def generate_key(seed):
    random.seed(seed)
    key = bytearray()
    for _ in range(16):
        key.append(random.randint(0, 255))
    return bytes(key)

def decrypt(ciphertext, key, iv):
    cipher = AES.new(key, AES.MODE_CBC, iv)
    return cipher.decrypt(ciphertext)

# read all key as bytes
with open("key.txt", "r") as f:
    keys = [bytes.fromhex(line.strip()) for line in f]

for key in keys:
    plaintext = decrypt(C, key, IV)
    if plaintext.startswith(b"%PDF"):
        print(key)
        break