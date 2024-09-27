#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

#define PORT 10888
#define KEY_LENGTH 32
#define NONCE_LENGTH 12

void handleErrors() {
    ERR_print_errors_fp(stderr);
    abort();
}

void chacha20_decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key, unsigned char *nonce, unsigned char *plaintext) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) handleErrors();

    if (EVP_DecryptInit_ex(ctx, EVP_chacha20(), NULL, key, nonce) != 1) {
        handleErrors();
    }

    int len;
    if (EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len) != 1) {
        handleErrors();
    }

    plaintext[len] = '\0';  // Null-terminate the plaintext
    EVP_CIPHER_CTX_free(ctx);
}

int main() {
    struct sockaddr_in server_addr;
    unsigned char key[KEY_LENGTH];
    unsigned char nonce[NONCE_LENGTH];
    unsigned char buffer[1024];
    unsigned char ciphertext[1024];

    // Create socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed!");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");  // Change this to the server IP if needed

    // Connect to server
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed!");
        return 1;
    }

    // Receive key and nonce from server
    recv(sock, key, KEY_LENGTH, 0);
    recv(sock, nonce, NONCE_LENGTH, 0);

    // Get message from user
    printf("Enter message to send: ");
    fgets(buffer, sizeof(buffer), stdin);
    buffer[strcspn(buffer, "\n")] = '\0';  // Remove newline

    // Send plaintext message to server
    send(sock, buffer, strlen(buffer), 0);

    // Receive ciphertext from server
    int len = recv(sock, ciphertext, sizeof(ciphertext), 0);

    // Decrypt message
    unsigned char decrypted[1024];
    chacha20_decrypt(ciphertext, len, key, nonce, decrypted);

    // Print decrypted message
    printf("Decrypted message: %s\n", decrypted);

    // Close socket
    close(sock);

    return 0;
}
