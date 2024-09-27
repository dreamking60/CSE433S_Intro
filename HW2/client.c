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
#define RC4_KEY_LENGTH 16

#define CHACHA_KEY_LENGTH 32
#define CHACHA_IV_LENGTH 12


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

// One Time Pad encryption
void otp_encrypt(char *plaintext, char *key, char *ciphertext) {
    for (int i = 0; i < strlen(plaintext); i++) {
        ciphertext[i] = plaintext[i] ^ key[i];
    }
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
    unsigned char rc4_key[RC4_KEY_LENGTH];

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

    // Perform Diffie-Hellman key exchange and decrypt AES key and IV

    // Define the chacha20 key and iv
    unsigned char chacha_key[CHACHA_KEY_LENGTH];
    unsigned char chacha_iv[CHACHA_IV_LENGTH];

    // recv key
    //varread = recv(sock, rc4_key, RC4_KEY_LENGTH, 0);
    // Receive key from server and send it back for confirmation
    varread = recv(sock, chacha_key, CHACHA_KEY_LENGTH, 0);

    // Receive IV from server and send it back for confirmation
    varread = recv(sock, chacha_iv, CHACHA_IV_LENGTH, 0);

    // Print the key and iv
    printf("Key: ");
    for (int i = 0; i < CHACHA_KEY_LENGTH; i++) {
        printf("%02x", chacha_key[i]);
    }
    printf("\n");

    printf("IV: ");
    for (int i = 0; i < CHACHA_IV_LENGTH; i++) {
        printf("%02x", chacha_iv[i]);
    }
    printf("\n");

    // Get input from the user:
    //printf("Enter message sent to the server: ");
    //fgets(client_message, sizeof(client_message), stdin);
    strcpy(client_message, "Hello, this is a test message!");


    // Encrypt the message
    int ciphertext_len = stream_encrypt(client_message, strlen(client_message), chacha_key, chacha_iv, ciphertext);

    // print the encrypted message
    printf("CHACHA20 Encrypt: %s\n",ciphertext);

    // Decrypt the message
    unsigned char decrypted_message[1024];
    int decrypted_message_len = stream_decrypt(ciphertext, ciphertext_len, chacha_key, chacha_iv, decrypted_message);

    // Print the decrypted message
    printf("CHACHA20 Decrypt: %s\n", decrypted_message);

    // Send the message to server:
    send(sock, ciphertext, ciphertext_len, 0);

    // Init OTP as length of the client_message
    char otp_key[strlen(client_message)];
    for (int i = 0; i < strlen(client_message); i++) {
        otp_key[i] = rand() % 256;
    }

    // print
    printf("OTP Plain: %s\n", client_message);

    // Encrypt the message with OTP
    char otp_ciphertext[strlen(client_message)];
    otp_encrypt(client_message, otp_key, otp_ciphertext);
    
    // print the OTP encrypted message
    printf("OTP Encrypt: %x\n",otp_ciphertext);
    printf("OTP Key: %x\n",otp_key);

    // send the OTP ciphertext to server
    send(sock, otp_ciphertext, strlen(otp_ciphertext), 0);

    // wait
    sleep(1);

    // send the OTP key to server
    send(sock, otp_key, strlen(otp_key), 0);
    


    // Receive the server's response:
    varread = read(sock, server_message, 1024);    
    printf("End: %s\n", server_message);

    // Close the socket:
    close(sock);

    return 0;

}
