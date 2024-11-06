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

int gcm_decrypt(unsigned char *ciphertext, int ciphertext_len,
               unsigned char *aad, int aad_len,
               unsigned char *tag,
               unsigned char *key,
               unsigned char *iv, int iv_len,
               unsigned char *plaintext)
{
   EVP_CIPHER_CTX *ctx;
   int len, plaintext_len, ret;


   /* Create and initialize the context */
   if(!(ctx = EVP_CIPHER_CTX_new())){
        handleErrors();
   };
   


   /* Initialize the decryption operation. */
   if(!EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL)){
        handleErrors();
   };


   /* Set IV length. Not necessary if this is 12 bytes (96 bits) */
   if(!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, iv_len, NULL)){
        handleErrors();
   };


   /* Initialize key and IV */
   if(!EVP_DecryptInit_ex(ctx, NULL, NULL, key, iv)){
        handleErrors();
   };


   /*
    * Provide any AAD data. This can be called zero or more times as
    * required
    */
   if(!EVP_DecryptUpdate(ctx, NULL, &len, aad, aad_len)){
        handleErrors();
   };


   /*
    * Provide the message to be decrypted, and obtain the plaintext output.
    * EVP_DecryptUpdate can be called multiple times if necessary
    */
   if(aad && aad_len > 0){
        if(!EVP_DecryptUpdate(ctx, NULL, &len, ciphertext, ciphertext_len)){
            handleErrors();
        }
   }
   plaintext_len = len;


   /* Set expected tag value. Works in OpenSSL 1.0.1d and later */
   if(!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, 16, tag)){
        handleErrors();
   };


   /*
    * Finalize the decryption. A positive return value indicates success,
    * and anything else is a failure - the plaintext is not trustworthy.
    */
   ret = EVP_DecryptFinal_ex(ctx, plaintext + len, &len);

    //print the plaintext
    printf("Plaintext: %s\n", plaintext);

   /* Clean up */
   EVP_CIPHER_CTX_free(ctx);


   if(ret > 0) {
       /* Success */
       plaintext_len += len;
       return plaintext_len;
   } else {
       /* Verify failed */
       return -1;
   }
}

int main() {
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    unsigned char key[KEY_LENGTH];
    unsigned char nonce[NONCE_LENGTH];
    unsigned char aad[16] = "0123456789abcdef"; // Hardcoded AAD
    unsigned char buffer[1024];
    unsigned char ciphertext[1024];
    unsigned char tag[16];
    unsigned char plaintext[1024];
    int plaintext_len;


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
    // Receive key, nonce, ciphertext, and tag from client
    recv(client_sock, key, KEY_LENGTH, 0);
    recv(client_sock, nonce, NONCE_LENGTH, 0);
    int ciphertext_len = recv(client_sock, ciphertext, sizeof(ciphertext), 0);
    recv(client_sock, tag, sizeof(tag), 0);

    // Print key, nonce, ciphertext, and tag
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

    // Decrypt the ciphertext
    plaintext_len = gcm_decrypt(ciphertext, ciphertext_len, aad, AAD_LENGTH, tag, key, nonce, NONCE_LENGTH, plaintext);
    if (plaintext_len < 0) {
        fprintf(stderr, "Decryption failed!\n");
        close(client_sock);
        close(server_sock);
        return 1;
    }

    // Null-terminate the plaintext
    plaintext[plaintext_len] = '\0';

    printf("Decrypted text: %s\n", plaintext);

    close(client_sock);
    close(server_sock);

    return 0;
}
