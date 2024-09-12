#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>

int main() {
    // Declare Variables
    char server_ip[16] = "192.168.92.132";
    int server_port = 10888;
    char server_message[1024];
    char client_message[1024];

    // Create socket:
    int sock;

    // Send connection request to server, be sure to set port and IP the same as server-side
    if(connect(sock, server_ip, server_port) < 0) {
        perror("Connection failed with error!");
        return 1;
    }

    // Get input from the user:
    printf("Enter message sent to the server: ");
    fgets(client_message);

    // Send the message to server:
    send(sock, client_message, strlen(client_message), 0);

    // Receive the server's response:
    recv(sock, server_message, 1024, 0);

    printf("Server's response: %s\n",server_message);

    // Close the socket:
    close(sock);
    
    return 0;

}
