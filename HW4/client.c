#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>

#define PORT 10888
#define KEY_LENGTH 32
#define NONCE_LENGTH 12
#define AAD_LENGTH 16

void handleErrors() {
    ERR_print_errors_fp(stderr);
    abort();
}

int gcm_encrypt(unsigned char *plaintext, int plaintext_len,
                unsigned char *aad, int aad_len,
                unsigned char *key,
                unsigned char *iv, int iv_len,
                unsigned char *ciphertext,
                unsigned char *tag)
{
    EVP_CIPHER_CTX *ctx;
    int len;
    int ciphertext_len;

    /* Create and initialize the context */
    if(!(ctx = EVP_CIPHER_CTX_new())) handleErrors();

    /* Initialize the encryption operation. */
    if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL))
        handleErrors();

    /* Set IV length if default 12 bytes (96 bits) is not appropriate */
    if(1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, iv_len, NULL))
        handleErrors();

    /* Initialize key and IV */
    if(1 != EVP_EncryptInit_ex(ctx, NULL, NULL, key, iv)) handleErrors();

    /* Provide any AAD data. This can be called zero or more times as required */
    if(aad && aad_len > 0) {
        if(1 != EVP_EncryptUpdate(ctx, NULL, &len, aad, aad_len))
            handleErrors();
    }

    /* Provide the message to be encrypted, and obtain the encrypted output */
    if(1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len))
        handleErrors();
    ciphertext_len = len;

    /* Finalize the encryption */
    if(1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len)) handleErrors();
    ciphertext_len += len;

    /* Get the tag */
    if(1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16, tag))
        handleErrors();

    /* Clean up */
    EVP_CIPHER_CTX_free(ctx);

    return ciphertext_len;
}

int main() {
    struct sockaddr_in server_addr;
    unsigned char key[KEY_LENGTH];
    unsigned char nonce[NONCE_LENGTH];
    unsigned char aad[16] = "0123456789abcdef"; // Hardcoded AAD
    unsigned char plaintext[1024] = "Test Message.";
    unsigned char ciphertext[1024];
    unsigned char tag[16];
    int ciphertext_len;

    // Generate random key and nonce
    if (!RAND_bytes(key, KEY_LENGTH) || !RAND_bytes(nonce, NONCE_LENGTH)) {
        handleErrors();
    }

    // Encrypt the plaintext
    ciphertext_len = gcm_encrypt(plaintext, strlen((char *)plaintext), aad, AAD_LENGTH, key, nonce, NONCE_LENGTH, ciphertext, tag);

    // Print key and nonce
    printf("Key: ");
    for (int i = 0; i < KEY_LENGTH; i++) {
        printf("%02x", key[i]);
    }
    printf("\n");

    printf("Nonce: ");
    for (int i = 0; i < NONCE_LENGTH; i++) {
        printf("%02x", nonce[i]);
    }
    printf("\n");

    // Print ciphertext and tag
    printf("Ciphertext: ");
    for (int i = 0; i < ciphertext_len; i++) {
        printf("%02x", ciphertext[i]);
    }
    printf("\n");

    printf("Tag: ");
    for (int i = 0; i < 16; i++) {
        printf("%02x", tag[i]);
    }
    printf("\n");

    // Create socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed!");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("192.168.92.132");  // Change this to the server IP if needed

    // Connect to server
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed!");
        close(sock);
        return 1;
    }

    // Send key, nonce, aad, ciphertext, and tag to server
    send(sock, key, KEY_LENGTH, 0);
    send(sock, nonce, NONCE_LENGTH, 0);
    send(sock, ciphertext, ciphertext_len, 0);
    send(sock, tag, sizeof(tag), 0);

    close(sock);

    return 0;
}