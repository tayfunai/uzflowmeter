#include "include/socket.h"

void send_json_to_python(const char *json_data) {
    int sock;
    struct sockaddr_in server_addr;
    char *message = (char *)json_data;

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("Could not create socket");
        return;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connect failed");
        close(sock);
        return;
    }

    // Send the JSON data
    if (send(sock, message, strlen(message), 0) < 0) {
        perror("Send failed");
        close(sock);
        return;
    }

    printf("JSON sent: %s\n", message);

    // Receive response from the server (optional)
    char server_reply[10000];
    if (recv(sock, server_reply, sizeof(server_reply), 0) < 0) {
        perror("Receive failed");
    } else {
        printf("Server reply: %s\n", server_reply);
    }

    // Close socket
    close(sock);
}


