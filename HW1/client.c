#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>

int main() {
    // Declare Variables
    struct sockaddr_in server_addr;

    char server_ip[16] = "192.168.92.132";
    int server_port = 10888;
    char server_message[1024];
    char client_message[1024];

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

    // Send the message to server:
    send(sock, client_message, strlen(client_message), 0);

    // Receive the server's response:
    varread = recv(sock, server_message, 1024, 0);

    printf("Server's response: %s\n",server_message);

    // Close the socket:
    close(sock);

    return 0;

}
