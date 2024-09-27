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
#include <openssl/engine.h>

#define AES_KEY_LENGTH 32
#define AES_BLOCK_SIZE 16

#define RC4_KEY_LENGTH 16
#define CHACHA_KEY_LENGTH 32
#define CHACHA_IV_LENGTH 12

// Handle errors
void handleErrors(void)
{
    ERR_print_errors_fp(stderr);
    abort();
}

int stream_decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *key, unsigned char *iv, unsigned char *plaintext)
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
   if(EVP_DecryptInit_ex(ctx, EVP_chacha20(), NULL, key, iv) != 1) {
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

int stream_encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *key, unsigned char *iv, unsigned char *ciphertext)
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
   if(EVP_EncryptInit_ex(ctx, EVP_chacha20(), NULL, key, iv) != 1) {
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


// Define the decryption function

int main() {
    // Declare variables
	ssize_t varread;
    char server_message[1024];
    char client_message[1024];

    struct sockaddr_in server_addr;
    char server_ip[16]= "192.168.92.132";
    int server_port = 10888;

    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

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

    // Declare key
    //unsigned char key[RC4_KEY_LENGTH];

    // Generate key
    // if (!RAND_bytes(key, RC4_KEY_LENGTH)) {
    //     handleErrors();
    // }

    // Send the key to the client
    //send(client_sock, key, RC4_KEY_LENGTH, 0);

    // Declare key and iv
    unsigned char key[CHACHA_KEY_LENGTH];
    unsigned char iv[CHACHA_IV_LENGTH];

    // Generate key and iv
    if (!RAND_bytes(key, CHACHA_KEY_LENGTH)) {
        handleErrors();
    }
    if (!RAND_bytes(iv, CHACHA_IV_LENGTH)) {
        handleErrors();
    }

    // Send the key to the client

    // correct the key
    int times = 0;
    do {
        send(client_sock, key, CHACHA_KEY_LENGTH, 0);
        unsigned char keyread[CHACHA_KEY_LENGTH];
        varread = recv(client_sock, keyread, CHACHA_KEY_LENGTH, 0);
        if(keyread != key) {
            send(client_sock, "Key not received correctly!", 1024, 0);
        } else {
            break;
        }
        times++;
    } while(times < 3);

    // Send the iv to the client
    times = 0;
    do {
        send(client_sock, iv, CHACHA_IV_LENGTH, 0);
        unsigned char ivread[CHACHA_IV_LENGTH];
        varread = recv(client_sock, ivread, CHACHA_IV_LENGTH, 0);
        if(ivread != iv) {
            send(client_sock, "IV not received correctly!", 1024, 0);
        } else {
            break;
        }
        times++;
    } while(times < 3);


    // Receive client's message
    varread = recv(client_sock, client_message, 1024, 0);
    printf("CHACHA20 Encrypt: %x\n", client_message);

    // Decrypt the message
    unsigned char decrypted_message[1024];
    int decrypted_message_len = stream_decrypt(client_message, varread, key, iv, decrypted_message);

    // Print the decrypted message
    printf("CHACHA20 Decrypted message: %x\n", decrypted_message);


    // Receive OTP encrypted message
    char otp_encrypted_message[1024];
    char otp_key[varread];
    varread = recv(client_sock, otp_encrypted_message, 1024, 0);
    varread = recv(client_sock, otp_key, 1024, 0);
    printf("OTP Encrypt: %x\n", otp_encrypted_message);
    printf("OTP Key: %x\n", otp_key);

    // Decrypt the OTP message
    char otp_decrypted_message[varread];
    for (int i = 0; i < varread; i++) {
        otp_decrypted_message[i] = otp_encrypted_message[i] ^ otp_key[i];
    }

    // Print the OTP decrypted message
    printf("OTP Decrypted message: %s\n", otp_decrypted_message);

    // Respond to client
    strcpy(server_message, "Test Over!");

    // Send the message to the client
    send(client_sock, server_message, strlen(server_message), 0);

    // Close the socket
    close(client_sock);
    close(sever_sock);
    

    return 0;
}