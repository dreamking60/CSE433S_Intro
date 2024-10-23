#Plaintext (P1): This is a known message!
#Ciphertext (C1): a469b1c502c1cab966965e50425438e1bb1b5f9037a4c159
#Plaintext (P2): (unknown to you)
#Ciphertext (C2): bf73bcd3509299d566c35b5d450337e1bb175f903fafc159

P1 = "This is a known message!"
C1 = "a469b1c502c1cab966965e50425438e1bb1b5f9037a4c159"
C2 = "bf73bcd3509299d566c35b5d450337e1bb175f903fafc159"

C1_bytes = bytes.fromhex(C1)
C2_bytes = bytes.fromhex(C2)

P1 = P1.encode()

# R = P1 xor C1
R = bytes([p1 ^ c1 for p1, c1 in zip(P1, C1_bytes)])

# P2 = R xor C2
P2 = bytes([r ^ c2 for r, c2 in zip(R, C2_bytes)])

print(P2.decode())