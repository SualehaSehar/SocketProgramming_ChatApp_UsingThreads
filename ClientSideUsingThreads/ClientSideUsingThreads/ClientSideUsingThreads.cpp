
#include <stdio.h>
#include <stdlib.h>
#include <String>
#include <pthread.h>
#include<iostream>
#include <windows.h>
#include<winsock.h>
#include <string.h>
#include <thread>
#include <chrono>

#pragma comment(lib, "ws2_32.lib")
using namespace std;
using std::cout;
using std::endl;
using std::cin;
using std::string;
#define buf_size 1024
#define name_size 20
bool exit_flag = false;
string name;

void* send_msg(void* arg) {
    int client_socket = *((int*)arg);
    while (!exit_flag) {
        string message;
        cout << endl << "[" << name << "]: ";
        getline(cin, message);
        message = "[" + name + "]: " + message.c_str();
        int bytes_sent = send(client_socket, message.c_str(), message.length(), 0);
        this_thread::sleep_for(chrono::milliseconds(100));
    }
    return NULL;
}

void* rcv_msg(void* arg) {
    int client_socket = *((int*)arg);
    while (!exit_flag) {
        char buffer[buf_size] = { 0 };
        int bytes_received = recv(client_socket, buffer, buf_size, 0);
        if (bytes_received < 0) {
            cout << "Server disconnected\n";
            exit_flag = true;
        }
        else {
            
            cout << endl << buffer << endl;
        }
    }
    return NULL;
}

int main(int argc, char* argv[]) {

    struct sockaddr_in server_addr;  // the address we want to connect to
    int client_socket;
    pthread_t snd_thread, rcv_thread;
    void* thread_return;
    name = argv[1];

    //to provide env for socket programming in windows
    WSADATA ws;
    if (WSAStartup(MAKEWORD(2, 2), &ws) < 0) {
        cout << endl << "WSA failed!";
        exit(EXIT_FAILURE);
    }

    //create socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);

    //Connect
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        cout << endl << "Connection failed!";
        exit(EXIT_FAILURE);
    }

    cout << endl << "Connection Established You Can Chat." << endl;

    pthread_create(&snd_thread, NULL, send_msg, (void*)&client_socket);
    pthread_create(&rcv_thread, NULL, rcv_msg, (void*)&client_socket);
    pthread_join(snd_thread, &thread_return);
    pthread_join(rcv_thread, &thread_return);

    //close
    closesocket(client_socket);
    return 0;
}
