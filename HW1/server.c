#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>

int main() {
    // Declare variables
	char server_ip[16] = "192.168.92.132";
    int server_port = 10888;
    char server_message[1024];
    char client_message[1024];


    // Create socket
  	int sock;
    int client_sock;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock == 0) {
        perror("Socket failed with error!");
        return 1;
    }
    // Bind to the set port and IP
    printf("Done with binding with IP: %s, Port: %d\n", server_ip, server_port);
    if(bind(sock, server_ip, server_port) < 0) {
        perror("Bind failed with error!");
        return 1;
    }
    // Listen for clients:
    if(socket_listen(sock, 3) < 0) {
        perror("Listen failed with error!");
        return 1;
    }

    // Accept an incoming connection
    if(socket_accept(sock, client_sock, client_ip, client_port) < 0) {
        perror("Accept failed with error!");
        return 1;
    }
    printf("Client connected at IP: %s and port: %i\n", client_ip, client_port);

    // Receive client's message
    client_message = socket_recv(client_sock, 1024);
    printf("Msg from client: %s\n", client_message);


    // Respond to client
    strcpy(server_message, "##Hello, Bob! This is Alice.##");
    socket_send(client_sock, server_message, strlen(server_message));

    // Close the socket
    close(client_sock);
    close(sock);

    return 0;
}