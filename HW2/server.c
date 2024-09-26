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


// Handle errors
void handleErrors(void)
{
    ERR_print_errors_fp(stderr);
    abort();
}

// Define the decryption function
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

// Function to send AES key and IV to client using Diffie-Hellman
void sendAESKeyAndIV(int client_socket, unsigned char *key, unsigned char *iv) {

    // Initialize Diffie-Hellman
    DH *dh = DH_new();
    if(dh == NULL) {
        handleErrors();
    }
    
    if(DH_generate_parameters_ex(dh, 2048, DH_GENERATOR_2, NULL) != 1) {
        handleErrors();
    }

    if(DH_generate_key(dh) != 1) {
        handleErrors();
    }

    const BIGNUM *pub_key = DH_get0_pub_key(dh);
    const BIGNUM *priv_key = DH_get0_priv_key(dh);

    // Send the public key to the client
    int pub_key_len = BN_num_bytes(pub_key);
    unsigned char *pub_key_bytes = (unsigned char *)malloc(pub_key_len);
    BN_bn2bin(pub_key, pub_key_bytes);
    send(client_socket, pub_key_bytes, pub_key_len, 0);
    free(pub_key_bytes);

    // Receive the public key from the client
    unsigned char client_pub_key_bytes[256];
    int client_pub_key_len = recv(client_socket, client_pub_key_bytes, 256, 0);
    BIGNUM *client_pub_key = BN_bin2bn(client_pub_key_bytes, client_pub_key_len, NULL);

    // Compute the shared secret
    unsigned char *shared_key[256];
    int shared_key_len = DH_compute_key(shared_key, client_pub_key, dh);
    if(shared_key_len < 0) {
        handleErrors();
    }

    // Encrypt the key and iv using the shared key
    unsigned char *encrypted_key[256];
    int encrypted_key_len = stream_encrypt(key, AES_KEY_LENGTH, shared_key, iv, encrypted_key);
    if(encrypted_key_len < 0) {
        handleErrors();
    }

    // Send the encrypted key to the client
    send(client_socket, encrypted_key, encrypted_key_len, 0);

    DH_free(dh);   
    BN_free(client_pub_key);
}



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

    // Declare key and iv
    unsigned char key[AES_KEY_LENGTH];
    unsigned char iv[AES_BLOCK_SIZE];

    // Initialize key and iv
    if(RAND_bytes(key, AES_KEY_LENGTH) != 1) {
        printf("Error in generating key\n");
        return;
    }

    if(RAND_bytes(iv, AES_BLOCK_SIZE) != 1) {
        printf("Error in generating iv\n");
        return;
    }

    // Send AES key and IV to client
    sendAESKeyAndIV(client_sock, key, iv);    

    // Receive client's message
    varread = recv(client_sock, client_message, 1024, 0);
    printf("Msg from client: %s\n", client_message);

    // Decrypt the message
    unsigned char decrypted_message[1024];
    int decrypted_message_len = stream_decrypt(client_message, varread, key, iv, decrypted_message);

    // Print the decrypted message
    printf("Decrypted message: %s\n", decrypted_message);

    // Respond to client
    strcpy(server_message, "##Hello, Bob! This is Alice.##");

    // Encrypt the message
    unsigned char encrypted_message[1024];
    int encrypted_message_len = stream_encrypt(server_message, strlen(server_message), key, iv, encrypted_message);

    // Send the message to the client
    send(client_sock, encrypted_message, encrypted_message_len, 0);

    // Close the socket
    close(client_sock);
    close(sever_sock);
    

    return 0;
}