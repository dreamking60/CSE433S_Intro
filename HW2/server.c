#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <unistd.h>

#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/aes.h>
#include <openssl/dh.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/engine.h>

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
    int len = EVP_DecodeBlock(output, input, length);
    //remove '\0' from the output
    while(output[len-1] == '\0') len--;
    return len;
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
    // Declare variables
	ssize_t varread;
    char server_message[1024];
    char client_message[4096];

    struct sockaddr_in server_addr;
    char server_ip[16]= "192.168.92.132";
    int server_port = 10888;

    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    // AES key and IV
    unsigned char key[AES_KEY_LENGTH];
    unsigned char iv[AES_BLOCK_SIZE];

    // Base64 decoded message
    unsigned char decoded_message[4096];
    int decoded_message_len;

    // plain text
    unsigned char plaintext[1024];
    int plaintext_len;

    // Create socket
    int client_sock;
  	int sever_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sever_sock < 0) {
        perror("Socket creation failed with error!");
        return 1;
    }
    if(setsockopt(sever_sock, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0) {
        perror("Socket option failed with error!");
        return 1;
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    server_addr.sin_addr.s_addr = inet_addr(server_ip);
    

    // Bind to the set port and IP
    if(bind(sever_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Binding failed with error!");
        return 1;
    }
    printf("Done with binding with IP: %s, Port: %d\n", server_ip, server_port);

    // Listen for clients:
    if(listen(sever_sock, 3) < 0) {
        perror("Listening failed with error!");
        return 1;
    }

    // Accept an incoming connection
    if((client_sock = accept(sever_sock, (struct sockaddr*)&client_addr, &client_addr_len)) < 0) {
        perror("Accepting failed with error!");
        return 1;
    }
    char * client_ip = inet_ntoa(client_addr.sin_addr);
    int client_port = ntohs(client_addr.sin_port);
    printf("Client connected at IP: %s and port: %i\n", client_ip, client_port);

    // Generate key and iv
    if (!RAND_bytes(key, AES_KEY_LENGTH)) {
        handleErrors();
    }
    if (!RAND_bytes(iv, AES_BLOCK_SIZE)) {
        handleErrors();
    }

    // print key and iv
    printf("Key: ");
    for (int i = 0; i < AES_KEY_LENGTH; i++) {
        printf("%02x", key[i]);
    }
    printf("\n");

    printf("IV: ");
    for (int i = 0; i < AES_BLOCK_SIZE; i++) {
        printf("%02x", iv[i]);
    }
    printf("\n");

    // Send the key to the client
    send(client_sock, key, AES_KEY_LENGTH, 0);

    // Send the iv to the client
    send(client_sock, iv, AES_BLOCK_SIZE, 0);

    // Receive client's message
    varread = recv(client_sock, client_message, 4096, 0);

    // Base64 decode
    decoded_message_len = base64_decode(client_message, varread, decoded_message);

    // Print the decoded message as hex
    printf("Decoded Message: ");
    for (int i = 0; i < decoded_message_len; i++) {
        printf("%02x", decoded_message[i]);
    }
    printf("\n");

    // Decrypt the message
    plaintext_len = block_decrypt(decoded_message, decoded_message_len, key, iv, plaintext);

    // Print the decrypted message
    printf("Decrypted Message: %s\n", plaintext);

    // Respond to client
    strcpy(server_message, "#Server: I got your message!");
    send(client_sock, server_message, strlen(server_message), 0);

    // Close the socket
    close(client_sock);
    close(sever_sock);
    

    return 0;
}