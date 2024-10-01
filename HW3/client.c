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
    int decoded_len = EVP_DecodeBlock(output, input, length);

    // Remove padding if present
    while (decoded_len > 0 && output[decoded_len - 1] == '\0') {
        decoded_len--;
    }

    return decoded_len;
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
   if(EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv) != 1) {
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
   if(EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv) != 1) {
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

    // Ciphertext
    unsigned char ciphertext[1024];
    int ciphertext_len;

    // Base64 encode ciphertext
    unsigned char base64_encoded_ciphertext[2048];
    int base64_encoded_ciphertext_len;

    // AES key and iv
    unsigned char AES_key[AES_KEY_LENGTH];
    unsigned char AES_iv[AES_BLOCK_SIZE];


    // Base64 encoded key and iv
    unsigned char base64_encoded_key[4096];
    unsigned char base64_encoded_iv[4096];

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
    varread = recv(sock, base64_encoded_key, 4096, 0);

    // sleep to get the iv
    sleep(1);

    // Receive the iv from the server
    varread = recv(sock, base64_encoded_iv, 4096, 0);

    // print key and iv in base64
    printf("Base64 Encoded Key: %s\n", base64_encoded_key);
    printf("Base64 Encoded IV: %s\n", base64_encoded_iv);

    // Base64 decode the key and iv
    base64_decoded_key_len = base64_decode(base64_encoded_key, strlen(base64_encoded_key), AES_key);
    base64_decoded_iv_len = base64_decode(base64_encoded_iv, strlen(base64_encoded_iv), AES_iv);

    // Print the key and iv
    printf("Key: ");
    for (int i = 0; i < AES_KEY_LENGTH; i++) {
        printf("%02x", AES_key[i]);
    }
    printf("\n");

    printf("IV: ");
    for (int i = 0; i < AES_BLOCK_SIZE; i++) {
        printf("%02x", AES_iv[i]);
    }
    printf("\n");

    // Get input from the user:
    printf("Enter message sent to the server: ");
    fgets(client_message, sizeof(client_message), stdin);

    // Encrypt the message
    ciphertext_len = block_encrypt(client_message, strlen(client_message), AES_key, AES_iv, ciphertext);

    // print the ciphertext as hex
    printf("Ciphertext: ");
    for (int i = 0; i < ciphertext_len; i++) {
        printf("%02x", ciphertext[i]);
    }
    printf("\n");

    // Base64 encode the ciphertext
    base64_encoded_ciphertext_len = base64_encode(ciphertext, ciphertext_len, base64_encoded_ciphertext);

    // Send the message to server:
    send(sock, base64_encoded_ciphertext, base64_encoded_ciphertext_len, 0);

    // Receive the server's response:
    varread = read(sock, server_message, 1024);

    printf("Server's response: %s\n",server_message);

    // Close the socket:
    close(sock);

    return 0;

}
