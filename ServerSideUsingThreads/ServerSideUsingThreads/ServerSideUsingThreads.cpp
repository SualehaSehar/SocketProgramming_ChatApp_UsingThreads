#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <pthread.h>
#include<iostream>
#include <windows.h>
#include<winsock.h>
#include <string.h>

#pragma comment(lib, "ws2_32.lib")
using std::cout;
using std::endl;
using std::cin;
using std::string;

#define MAX_CLIENT 200
int client_count = 0;
int client_sockets_arr[MAX_CLIENT];
pthread_mutex_t mutx;

void send_message(char *buffer, int len, int soc_nmbr) {
    int i;
    pthread_mutex_lock(&mutx);
    //send(soc_nmbr, buffer, strlen(buffer), 0);
    for (i = 0; i < client_count; i++) {
        send(client_sockets_arr[i], buffer, strlen(buffer), 0);
    }
    pthread_mutex_unlock(&mutx);
}

void* handle_client(void *arg)
{
    int client_soc = *((int*)arg);
    int recv_size = 0, i;
    char buffer[500];

    // Loop to receive/send messages
    memset(&buffer, 0, sizeof(buffer));
    while ((recv_size = recv(client_soc, buffer, sizeof(buffer), 0)) != 0)
    {
        send_message(buffer,recv_size, client_soc);
        memset(&buffer, 0, sizeof(buffer));
    }

    //remove disconnected client
    pthread_mutex_lock(&mutx);
    for (i = 0; i < client_count; i++) {
        if (client_soc == client_sockets_arr[i]) {
            while(i < client_count - 1) {
                client_sockets_arr[i] = client_sockets_arr[i+1];
            }
            break;
        }
    }
    client_count--;
    pthread_mutex_unlock(&mutx);
    closesocket(client_soc);
    return NULL;
}


int main() {

    struct sockaddr_in server_addr, client_addr;
    int server_socket, client_socket, client_addr_len;
    char buffer[1024];
    pthread_t tid;

    //to provide env for socket programming in windows
    WSADATA ws;

    if (WSAStartup(MAKEWORD(2, 2), &ws) < 0) {
        cout << endl << "WSA failed!";
        exit(EXIT_FAILURE);
    }

    pthread_mutex_init(&mutx, NULL);
    server_socket = socket(AF_INET, SOCK_STREAM, 0); // 0 USES UNDERLINK PROTOCOL (MEANS WHAT MY SYSTEM'S UNDERLINK FRAMEWORK IS USING)

    //binding
    memset(&server_addr, 0, sizeof(server_addr)); //to set all the socket structures with null values
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // my system is server so it will pick my system's ip address 

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr))<0) {
        cout << endl << "Binding to local port failed!";
        exit(EXIT_FAILURE);
    }

    //listening
    int listening = listen(server_socket, 5); //more than 5 clients will go to waiting queue and thes 5 clients will be in active queue
    if (listening < 0) {
        cout << endl << "listening to local port failed!";
        exit(EXIT_FAILURE);
    }
    
    while (true) {

        cout << endl << "Waiting for client..." << endl;
        //accepting
        memset(&client_addr, 0, sizeof(client_addr));
        client_addr_len = sizeof(client_addr);
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);
        
        //entering critical section here
        pthread_mutex_lock(&mutx);
        client_sockets_arr[client_count++] = client_socket;
        pthread_mutex_unlock(&mutx);

        pthread_create(&tid, NULL, handle_client, (void*)&client_socket);
        pthread_detach(tid);
       
    }

    closesocket(server_socket);
    return 0;
}
