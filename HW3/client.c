#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <openssl/hmac.h>
#include <openssl/err.h>

void handleErrors(void) {
    ERR_print_errors_fp(stderr);
    abort();
}


void cal_hmac(unsigned char *mac, char *message) {
/* The secret key for hashing */
   const char key[] = "SECRET_KEY";


   /* Change the length accordingly with your chosen hash engine.
   * Be careful of the length of string with the chosen hash engine. For example, SHA1 needed 20 characters. */
   // sha256 needs 32 characters
   unsigned int len = 32;


   /* Create and initialize the context */
   HMAC_CTX *ctx;
   ctx = HMAC_CTX_new();
   if(ctx == NULL) {
       handleErrors();
   }


   /* Initialize the HMAC operation. */
   if(HMAC_Init_ex(ctx, key, strlen(key), EVP_sha256(), NULL) != 1) {
       handleErrors();
   }


   /* Provide the message to HMAC, and start HMAC authentication. */
   if(HMAC_Update(ctx, (unsigned char*)message, strlen(message)) != 1) {
       handleErrors();
   }
  
   /* HMAC_Final() writes the hashed values to md, which must have enough space for the hash function output. */
   if(HMAC_Final(ctx, mac, &len) != 1) {
       handleErrors();
   }

   /* Releases any associated resources and finally frees context variable */
   HMAC_CTX_free(ctx);
  
   return;
}

int main() {
    // Declare Variables
    struct sockaddr_in server_addr;

    char server_ip[16] = "192.168.92.132";
    int server_port = 10888;
    char server_message[1024];
    char client_message[1024];

    unsigned char mac[32];

    ssize_t varread;

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

    // Get input from the user:
    printf("Enter message sent to the server: ");
    fgets(client_message, sizeof(client_message), stdin);

    // print the message
    printf("MSG: %s\n", client_message);

    // Send the message to server:
    send(sock, client_message, strlen(client_message), 0);

    // Send the HMAC to the server:
    cal_hmac(mac, client_message);

    // print the HMAC
    printf("HMAC: ");
    for(int i = 0; i < 32; i++) {
        printf("%02x", mac[i]);
    }
    printf("\n");

    // Send the HMAC to the server:
    send(sock, mac, sizeof(mac), 0);

    // Receive the server's response:
    varread = read(sock, server_message, 1024);

    printf("Server's response: %s\n",server_message);

    // Close the socket:
    close(sock);

    return 0;

}
