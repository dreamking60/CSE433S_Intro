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
#include <openssl/bio.h>
#include <openssl/buffer.h>

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
    unsigned char base64_encoded_ciphertext[4096];
    int base64_encoded_ciphertext_len;

    // AES key and iv
    unsigned char AES_key[AES_KEY_LENGTH];
    unsigned char AES_iv[AES_BLOCK_SIZE];

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
    varread = read(sock, AES_key, AES_KEY_LENGTH);

    // sleep
    sleep(1);

    // Receive the iv from the server
    varread = read(sock, AES_iv, AES_BLOCK_SIZE);

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

    // Base64 encode the ciphertext
    base64_encoded_ciphertext_len = base64_encode(ciphertext, ciphertext_len, base64_encoded_ciphertext);

    // Send the message to server:
    send(sock, base64_encoded_ciphertext, base64_encoded_ciphertext_len-1, 0);

    // print the ciphertext as hex
    printf("Ciphertext: ");
    for (int i = 0; i < ciphertext_len; i++) {
        printf("%02x", ciphertext[i]);
    }
    printf("\n");


    // Receive the server's response:
    varread = read(sock, server_message, 1024);

    printf("Server's response: %s\n",server_message);

    // Close the socket:
    close(sock);

    return 0;

}
