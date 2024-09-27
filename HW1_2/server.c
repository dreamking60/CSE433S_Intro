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

void chacha20_encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key, unsigned char *nonce, unsigned char *ciphertext) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) handleErrors();

    if (EVP_EncryptInit_ex(ctx, EVP_chacha20(), NULL, key, nonce) != 1) {
        handleErrors();
    }

    int len;
    if (EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len != 1)) {
        handleErrors();
    }

    EVP_CIPHER_CTX_free(ctx);
}

int main() {
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    unsigned char key[KEY_LENGTH];
    unsigned char nonce[NONCE_LENGTH];
    unsigned char buffer[1024];
    unsigned char ciphertext[1024];

    // Create socket
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("Socket creation failed!");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Binding failed!");
        return 1;
    }

    // Listen
    if (listen(server_sock, 1) < 0) {
        perror("Listening failed!");
        return 1;
    }

    // Accept client connection
    int client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_addr_len);
    if (client_sock < 0) {
        perror("Accepting failed!");
        return 1;
    }

    // Generate random key and nonce
    if (RAND_bytes(key, sizeof(key)) != 1) handleErrors();
    if (RAND_bytes(nonce, sizeof(nonce)) != 1) handleErrors();

    // Send key and nonce to client
    send(client_sock, key, KEY_LENGTH, 0);
    send(client_sock, nonce, NONCE_LENGTH, 0);

    // Receive message from client
    int len = recv(client_sock, buffer, sizeof(buffer), 0);

    // Encrypt message
    chacha20_encrypt(buffer, len, key, nonce, ciphertext);

    // Send ciphertext back to client
    send(client_sock, ciphertext, len, 0);

    // Close sockets
    close(client_sock);
    close(server_sock);

    return 0;
}
