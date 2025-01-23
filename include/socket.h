// json_sender.h
#ifndef JSON_SENDER_H
#define JSON_SENDER_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <json-c/json.h>
#include <unistd.h>           // Include unistd.h for the close() function


#define SERVER_IP "127.0.0.1"    // Python server IP address
#define SERVER_PORT 12345        // Port where Python server is listening

// Function to send JSON data to the Python server
void send_json_to_python(const char *json_data);

#endif // JSON_SENDER_H
