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

// Generate Diffie-Hellman keys and get AES key and IV
void generateDHKeys(DH **dh, BIGNUM **pub_key, BIGNUM **priv_key) {
    *dh = DH_new();
    if (*dh == NULL) handleErrors();

    if (1 != DH_generate_parameters_ex(*dh, 2048, DH_GENERATOR_2, NULL))
        handleErrors();

    if (1 != DH_generate_key(*dh)) handleErrors();

    DH_get0_key(*dh, (const BIGNUM **)pub_key, (const BIGNUM **)priv_key);
}

void computeSharedSecret(DH *dh, BIGNUM *peer_pub_key, unsigned char *shared_secret, int *shared_secret_len) {
    *shared_secret_len = DH_compute_key(shared_secret, peer_pub_key, dh);
    if (*shared_secret_len == -1) handleErrors();
}

void performDHKeyExchangeAndDecryptAESKey(int sock, unsigned char *aes_key, unsigned char *aes_iv) {
    // Diffie-Hellman key exchange
    DH *dh;
    BIGNUM *pub_key, *priv_key;
    generateDHKeys(&dh, &pub_key, &priv_key);

    // Send public key to server
    int pub_key_len = BN_num_bytes(pub_key);
    unsigned char *pub_key_buf = malloc(pub_key_len);
    BN_bn2bin(pub_key, pub_key_buf);
    send(sock, pub_key_buf, pub_key_len, 0);
    free(pub_key_buf);

    // Receive server's public key
    unsigned char server_pub_key_buf[256];
    int server_pub_key_len = recv(sock, server_pub_key_buf, sizeof(server_pub_key_buf), 0);
    BIGNUM *server_pub_key = BN_bin2bn(server_pub_key_buf, server_pub_key_len, NULL);

    // Compute shared secret
    unsigned char shared_secret[256];
    int shared_secret_len;
    computeSharedSecret(dh, server_pub_key, shared_secret, &shared_secret_len);

    // Receive encrypted AES key and IV
    unsigned char encrypted_key_iv[256];
    int encrypted_key_iv_len = recv(sock, encrypted_key_iv, sizeof(encrypted_key_iv), 0);

    // Decrypt AES key and IV
    unsigned char decrypted_key_iv[256];
    int decrypted_key_iv_len = stream_decrypt(encrypted_key_iv, encrypted_key_iv_len, shared_secret, NULL, decrypted_key_iv);

    memcpy(aes_key, decrypted_key_iv, AES_KEY_LENGTH);
    memcpy(aes_iv, decrypted_key_iv + AES_KEY_LENGTH, AES_BLOCK_SIZE);

    DH_free(dh);
    BN_free(server_pub_key);
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
    unsigned char aes_key[AES_KEY_LENGTH];
    unsigned char aes_iv[AES_BLOCK_SIZE];

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
    performDHKeyExchangeAndDecryptAESKey(sock, aes_key, aes_iv);

    // Get input from the user:
    printf("Enter message sent to the server: ");
    fgets(client_message, sizeof(client_message), stdin);

    // Encrypt the message
    int ciphertext_len = stream_encrypt(client_message, strlen(client_message), aes_key, aes_iv, ciphertext);

    // print the encrypted message
    printf("Encrypted message: ");

    // Send the message to server:
    send(sock, ciphertext, ciphertext_len, 0);

    // Receive the server's response:
    varread = read(sock, server_message, 1024);

    // print the server message
    printf("Server's message: %s\n",server_message);

    // Decrypt the server's response:
    unsigned char plaint_text[1024];
    int plaint_text_len = stream_decrypt(server_message, varread, aes_key, aes_iv, plaint_text);
    
    printf("Server's response: %s\n",plaint_text);

    // Close the socket:
    close(sock);

    return 0;

}
