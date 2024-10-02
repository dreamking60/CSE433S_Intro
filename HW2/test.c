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

int base64_decode(const unsigned char *input, int length, unsigned char *output) {
    BIO *bio, *b64;
    int decoded_len;

    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new_mem_buf(input, length);
    bio = BIO_push(b64, bio);

    // Disable newlines
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);

    // Decode the input data
    decoded_len = BIO_read(bio, output, length);
    if (decoded_len < 0) {
        BIO_free_all(bio);
        return -1; // Decoding failed
    }

    // Clean up
    BIO_free_all(bio);

    return decoded_len;
}

// Base64 encode
int base64_encode(const unsigned char *input, int length, unsigned char *output) {
    BIO *bio, *b64;
    BUF_MEM *buffer_ptr;

    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);

    // Write the input data to the BIO
    BIO_write(bio, input, length);
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &buffer_ptr);

    // Copy the encoded data to the output buffer
    memcpy(output, buffer_ptr->data, buffer_ptr->length);
    output[buffer_ptr->length] = '\0'; // Null-terminate the output

    // Clean up
    BIO_free_all(bio);

    return buffer_ptr->length;
}