# One Letter Frequency
letter_freq = ['E', 'T', 'A', 'O', 'I', 'N', 'S', 'H', 'R', 'D', 'L', 'U', 'C', 'M', 'W', 'F', 'Y', 'G', 'P', 'B', 'V', 'K', 'X', 'J', 'Q', 'Z']
letter_freq = [x.lower() for x in letter_freq]

# bigram
bigram_freq = ['TH', 'HE', 'IN', 'ER', 'AN', 'RE', 'ON', 'AT', 'EN', 'ND', 'TI', 'ES', 'OR', 'TE', 'OF', 'ED', 'IS', 'IT', 'AL', 'AR', 'ST', 'TO', 'NT', 'NG', 'SE', 'HA', 'AS', 'OU', 'IO', 'LE', 'VE', 'CO', 'ME', 'DE', 'HI', 'RI', 'RO', 'IC', 'NE', 'EA', 'RA', 'CE', 'LI', 'CH', 'LL', 'BE', 'MA', 'SI', 'OM', 'UR']
bigram_freq = [x.lower() for x in bigram_freq]

# trigram
trigram_freq = ['THE', 'AND', 'ING', 'ENT', 'ION', 'HER', 'FOR', 'THA', 'NTH', 'INT', 'ERE', 'TIO', 'TER', 'EST', 'ERS', 'ATI', 'HAT', 'ATE', 'ALL', 'ETH', 'HES', 'VER', 'HIS', 'STA', 'ITH', 'OTH', 'RES', 'ONT', 'ARE', 'EAR', 'THER', 'STH', 'SHE', 'WAS', 'ETHA', 'END', 'HAS', 'ONS', 'ITHA', 'WIT', 'DTH', 'THER', 'THI', 'HICH', 'FTH', 'SIN', 'TED', 'WIT', 'THER', 'HICH']
trigram_freq = [x.lower() for x in trigram_freq]

# exchange two letter in frequency
def exchange(letter1, letter2, one_freq):
    index1 = one_freq.index(letter1)
    index2 = one_freq.index(letter2)
    one_freq[index1], one_freq[index2] = one_freq[index2], one_freq[index1]
    
# calculate frequency
def freq(text):
    freq = {}
    for c in text:
        if c in freq:
            freq[c] += 1
        else:
            freq[c] = 1
    return freq

# calculate bigram frequency
def b_freq(text):
    freq = {}
    for i in range(len(text)-1):
        bigram = text[i:i+2]
        if bigram in freq:
            freq[bigram] += 1
        else:
            freq[bigram] = 1
    return freq

# Modify one letter frequency by bigram frequency
def modify_by_bigram(sorted_one_freq, two_freq):
    havedone = letter_freq[:4]
    for x in range(len(bigram_freq)):
        i = two_freq[x]
        index0 = sorted_one_freq.index(i[0])
        index1 = sorted_one_freq.index(i[1])
        if letter_freq[index0] not in havedone and bigram_freq[x][0] not in havedone:
            if letter_freq[index0] != bigram_freq[x][0]:
                exchange(letter_freq[index0], bigram_freq[x][0], letter_freq)
        if letter_freq[index1] not in havedone and bigram_freq[x][1] not in havedone:
            if letter_freq[index1] != bigram_freq[x][1]:
                exchange(letter_freq[index1], bigram_freq[x][1], letter_freq)
        havedone.append(letter_freq[index0])
        havedone.append(letter_freq[index1])
              

# calculate trigram frequency
def t_freq(text):
    freq = {}
    for i in range(len(text)-2):
        trigram = text[i:i+3]
        if trigram in freq:
            freq[trigram] += 1
        else:
            freq[trigram] = 1
    return freq

# Modify one letter frequency by trigram frequency
def modify_by_trigram(sorted_one_freq, three_freq):
    havedone = letter_freq[:4]
    for x in range(len(trigram_freq)):
        i = three_freq[x]
        index0 = sorted_one_freq.index(i[0])
        index1 = sorted_one_freq.index(i[1])
        index2 = sorted_one_freq.index(i[2])
        if letter_freq[index0] not in havedone and trigram_freq[x][0] not in havedone:
            if letter_freq[index0] != trigram_freq[x][0]:
                exchange(letter_freq[index0], trigram_freq[x][0], letter_freq)
        if letter_freq[index1] not in havedone and trigram_freq[x][1] not in havedone:
            if letter_freq[index1] != trigram_freq[x][1]:
                exchange(letter_freq[index1], trigram_freq[x][1], letter_freq)
        if letter_freq[index2] not in havedone and trigram_freq[x][2] not in havedone:
            if letter_freq[index2] != trigram_freq[x][2]:
                exchange(letter_freq[index2], trigram_freq[x][2], letter_freq)
        havedone.append(letter_freq[index0])
        havedone.append(letter_freq[index1])
        havedone.append(letter_freq[index2])

# sort frequency
def sort_freq(freq):
    return sorted(freq.items(), key=lambda x: x[1], reverse=True)

# from cipher text to plain text
def decrypt(sorted_freq_cipher, cipher):
    plain = ""
    for c in cipher:
        if c.isalpha():
            plain += letter_freq[sorted_freq_cipher.index(c)]
        else:
            plain += c
    return plain

with open('cipher.txt', 'r') as f:
    cipher = f.read().strip()
    
cipher_no_space = cipher.replace(" ", "")
cipher_no_space_newline = cipher_no_space.replace("\n", "")

one_freq = freq(cipher_no_space_newline)
two_freq = b_freq(cipher_no_space_newline)
three_freq = t_freq(cipher_no_space_newline)

sorted_one_freq = sort_freq(one_freq)
sorted_one_freq = [x[0] for x in sorted_one_freq]

sorted_two_freq = sort_freq(two_freq)
sorted_two_freq = [x[0] for x in sorted_two_freq]
modify_by_bigram(sorted_one_freq, sorted_two_freq)

sorted_three_freq = sort_freq(three_freq)
sorted_three_freq = [x[0] for x in sorted_three_freq]
modify_by_trigram(sorted_one_freq, sorted_three_freq)

#exchage u and c
exchange('u', 'c', letter_freq)

#exchange m and u
exchange('m', 'u', letter_freq)

#exchange f and b
exchange('f', 'b', letter_freq)

#exchange v and y
exchange('v', 'y', letter_freq)

#exchange j and q
exchange('j', 'q', letter_freq)

#exchange v and f
exchange('v', 'f', letter_freq)

plain = decrypt(sorted_one_freq, cipher)

with open('plain.txt', 'w') as f:
    f.write(plain)


