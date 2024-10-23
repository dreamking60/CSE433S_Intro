from Crypto.Cipher import AES
from Crypto.Util.Padding import pad
import binascii

def aes_encrypt(plaintext, key, iv):
    cipher = AES.new(key, AES.MODE_CBC, iv)
    ciphertext = cipher.encrypt(pad(plaintext.encode(), AES.block_size))
    return binascii.hexlify(ciphertext).decode()

key = binascii.unhexlify('00112233445566778899aabbccddeeff')  # key
iv1 = binascii.unhexlify('31323334353637383930313233343536')  # IV1
iv2 = binascii.unhexlify('31323334353637383930313233343537')  # IV2

p1_yes = "Yes"
p1_no = "No"

c1_yes = aes_encrypt(p1_yes, key, iv1)
c1_no = aes_encrypt(p1_no, key, iv1)

p2 = "Yes"

# p2 = p2 xor iv1 xor iv2
p2_bytes = bytes(p2.encode())
# p2 add padding
p2_bytes = pad(p2_bytes, AES.block_size)
iv1_bytes = bytes(iv1)
iv2_bytes = bytes(iv2)

p2 = bytes([p2 ^ iv1 ^ iv2 for p2, iv1, iv2 in zip(p2_bytes, iv1_bytes, iv2_bytes)])

c2 = aes_encrypt(p2.decode(), key, iv2)

# print c2 and c1_yes
print(c2)
print(c1_yes)





