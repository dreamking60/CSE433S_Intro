void test() {
    // Decode the key and iv in new variables
    unsigned char decoded_key[AES_KEY_LENGTH];
    unsigned char decoded_iv[AES_BLOCK_SIZE];

    // Base64 decode the key and iv
    base64_decode(base64_encoded_key, base64_encoded_key_len, decoded_key);
    base64_decode(base64_encoded_iv, base64_encoded_iv_len, decoded_iv);

    // Print the decoded key and iv
    printf("Decoded Key: ");
    for (int i = 0; i < AES_KEY_LENGTH; i++) {
        printf("%02x", decoded_key[i]);
    }
    printf("\n");

    printf("Decoded IV: ");
    for (int i = 0; i < AES_BLOCK_SIZE; i++) {
        printf("%02x", decoded_iv[i]);
    }
    printf("\n");
}