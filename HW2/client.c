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


//    /* Finalize the decryption. Further plaintext bytes may be written at this stage. */
//    if(EVP_DecryptFinal_ex(ctx, plaintext + plaintext_len, &len) != 1) {
//         handleErrors();
//    }
//     plaintext_len += len;


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
//    if(EVP_EncryptFinal_ex(ctx, ciphertext+ciphertext_len, &len) != 1) {
//         handleErrors();
//      }  
//    ciphertext_len += len;

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

    // Generate key and iv
    if (!RAND_bytes(chacha_key, CHACHA_KEY_LENGTH)) {
        handleErrors();
    }
    if (!RAND_bytes(chacha_iv, CHACHA_IV_LENGTH)) {
        handleErrors();
    }

    // recv key
    //varread = recv(sock, rc4_key, RC4_KEY_LENGTH, 0);
    // Receive key from server and send it back for confirmation
    for (int times = 0; times < 3; times++) {
        varread = recv(sock, chacha_key, CHACHA_KEY_LENGTH, 0);
        if (varread != CHACHA_KEY_LENGTH) {
            handleErrors();
        }
        send(sock, chacha_key, CHACHA_KEY_LENGTH, 0);

        // Receive confirmation message from server
        varread = recv(sock, client_message, 1024, 0);
        client_message[varread] = '\0';
        if (strcmp(client_message, "Key not received correctly!") != 0) {
            break;
        }
    }

    // Receive IV from server and send it back for confirmation
    for (int times = 0; times < 3; times++) {
        varread = recv(sock, chacha_iv, CHACHA_IV_LENGTH, 0);
        if (varread != CHACHA_IV_LENGTH) {
            handleErrors();
        }
        send(sock, chacha_iv, CHACHA_IV_LENGTH, 0);

        // Receive confirmation message from server
        varread = recv(sock, client_message, 1024, 0);
        client_message[varread] = '\0';
        if (strcmp(client_message, "IV not received correctly!") != 0) {
            break;
        }
    }

    // Get input from the user:
    printf("Enter message sent to the server: ");
    fgets(client_message, sizeof(client_message), stdin);
    printf("ChaCha20 Plain: %x\n", client_message);

    // Encrypt the message
    int ciphertext_len = stream_encrypt(client_message, strlen(client_message), chacha_key, chacha_iv, ciphertext);

    // print the encrypted message
    printf("CHACHA20 Encrypt: %x\n",ciphertext);

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
