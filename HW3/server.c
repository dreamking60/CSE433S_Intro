#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
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


   /* Initialize the HMAC operation. */
   HMAC_Init_ex(ctx, key, strlen(key), EVP_sha256(), NULL);


   /* Provide the message to HMAC, and start HMAC authentication. */
   HMAC_Update(ctx, (unsigned char*)message, strlen(message));
  
   /* HMAC_Final() writes the hashed values to md, which must have enough space for the hash function output. */
   HMAC_Final(ctx, mac, &len);


   /* Releases any associated resources and finally frees context variable */
   HMAC_CTX_free(ctx);
  
   return;
   
}

int main() {
    // Declare variables
	ssize_t varread;
    char server_message[1024];
    char client_message[1024];

    unsigned char mac[32];
    unsigned char server_mac[32];

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

    // Receive client's message
    varread = recv(client_sock, client_message, 1024, 0);
    printf("Msg: %s\n", client_message);

    // Receive client's HMAC
    varread = recv(client_sock, mac, 32, 0);
    printf("HMAC from client: %s\n", mac);

    // Calculate HMAC
    cal_hmac(server_mac, client_message);
    printf("HMAC from server: %s\n", server_mac);

    if(strcmp(mac, server_mac) == 0) {
        printf("Authentication successful!\n");
        // Respond to client
        strcpy(server_message, "Correct!");
        send(client_sock, server_message, strlen(server_message), 0);
    } else {
        printf("Authentication failed!\n");
    }

    // Close the socket
    close(client_sock);
    close(sever_sock);
    

    return 0;
}