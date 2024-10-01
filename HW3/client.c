#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>

#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/dh.h>
#include <openssl/rand.h>

#define AES_KEY_LENGTH 32
#define AES_BLOCK_SIZE 16

// Handle errors
void handleErrors(void)
{
    ERR_print_errors_fp(stderr);
    abort();
}

// Base64 decode
int base64_decode(const unsigned char *input, int length, unsigned char *output) {
    return EVP_DecodeBlock(output, input, length);
}

// Base64 encode
int base64_encode(const unsigned char *input, int length, unsigned char *output) {
    return EVP_EncodeBlock(output, input, length);
}

int block_decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key, unsigned char *iv, unsigned char *plaintext)
{
   /* Declare cipher context */
   EVP_CIPHER_CTX *ctx;

   int len, plaintext_len;

   /* Create and initialize the context */
   ctx = EVP_CIPHER_CTX_new();
   if(!ctx) {
        handleErrors();
    }

   /* Initialize the decryption operation. */
   if(EVP_DecryptInit_ex(ctx, EVP_AES_256_CBC(), NULL, key, iv) != 1) {
        handleErrors();
   }

   /* Provide the message to be decrypted, and obtain the plaintext output. EVP_DecryptUpdate can be called multiple times if necessary. */
   if(EVP_DecryptUpdate(ctx, plaintext, &plaintext_len, ciphertext, ciphertext_len) != 1) {
        handleErrors();
   }


   /* Finalize the decryption. Further plaintext bytes may be written at this stage. */
   if(EVP_DecryptFinal_ex(ctx, plaintext + plaintext_len, &len) != 1) {
        handleErrors();
   }
    plaintext_len += len;

   /* Clean up */
   EVP_CIPHER_CTX_free(ctx);

   return plaintext_len;
}

int block_encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key, unsigned char *iv, unsigned char *ciphertext)
{
  /* Declare cipher context */
   EVP_CIPHER_CTX *ctx;

   int len, ciphertext_len = 0;

   /* Create and initialize the context */
   ctx = EVP_CIPHER_CTX_new();
    if(!ctx) {
        handleErrors();
    }

   /* Initialize the encryption operation. */ 
   if(EVP_EncryptInit_ex(ctx, EVP_AES_256_CBC(), NULL, key, iv) != 1) {
        handleErrors();
     }


   /* Provide the message to be encrypted, and obtain the encrypted output. EVP_EncryptUpdate can be called multiple times if necessary */
    if(EVP_EncryptUpdate(ctx, ciphertext, &ciphertext_len, plaintext, plaintext_len) != 1) {
        handleErrors();
     }


   /* Finalize the encryption. Further ciphertext bytes may be written at this stage. */
   if(EVP_EncryptFinal_ex(ctx, ciphertext+ciphertext_len, &len) != 1) {
        handleErrors();
     }  
   ciphertext_len += len;

   /* Clean up */
   EVP_CIPHER_CTX_free(ctx);


   return ciphertext_len;
}

int main() {
    // Declare Variables
    struct sockaddr_in server_addr;

    char server_ip[16] = "192.168.92.132";
    int server_port = 10888;
    char server_message[1024];
    char client_message[1024];

    ssize_t varread;
    unsigned char ciphertext[1024];
    int ciphertext_len;

    // AES key and iv
    unsigned char AES_key[AES_KEY_LENGTH];
    unsigned char AES_iv[AES_BLOCK_SIZE];


    // Base64 encoded key and iv
    unsigned char base64_encoded_key[1024];
    unsigned char base64_encoded_iv[1024];

    // Decode length
    int base64_decoded_key_len;
    int base64_decoded_iv_len;

    // Create socket:
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    server_addr.sin_addr.s_addr = inet_addr(server_ip);

    // Send connection request to server, be sure to set port and IP the same as server-side
    if(connect(sock, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed with error!");
        return 1;
    }

    // Receive the key from the server
    varread = recv(sock, base64_encoded_key, 1024, 0);

    // Receive the iv from the server
    varread = recv(sock, base64_encoded_iv, 1024, 0);

    // Base64 decode the key and iv
    base64_decoded_key_len = base64_decode(base64_encoded_key, strlen(base64_encoded_key), AES_key);
    base64_decoded_iv_len = base64_decode(base64_encoded_iv, strlen(base64_encoded_iv), AES_iv);

    // Get input from the user:
    printf("Enter message sent to the server: ");
    fgets(client_message, sizeof(client_message), stdin);

    // Encrypt the message
    ciphertext_len = block_encrypt(client_message, strlen(client_message), AES_key, AES_iv, ciphertext);

    // Send the message to server:
    send(sock, ciphertext, ciphertext_len, 0);

    // Receive the server's response:
    varread = read(sock, server_message, 1024);

    printf("Server's response: %s\n",server_message);

    // Close the socket:
    close(sock);

    return 0;

}
