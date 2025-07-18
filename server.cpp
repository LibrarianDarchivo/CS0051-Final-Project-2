#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 12345
#define MAX_CLIENTS 4

using namespace std;

vector<int> clients;
mutex clients_mutex;

void handle_client(int client_socket, int id) {
    cout << "Player " << id + 1 << " connected.\n";
    
    string welcome = "Welcome Player " + to_string(id + 1) + "!\n";
    send(client_socket, welcome.c_str(), welcome.size(), 0);

    // Wait until game starts (could add gameplay here)
}

void broadcast_start() {
    string message = "All players connected. Game is starting!\n";
    lock_guard<mutex> lock(clients_mutex);
    for (int sock : clients) {
        send(sock, message.c_str(), message.size(), 0);
    }
}

int main() {
    int server_fd, new_socket;
    sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;  // Listen on all interfaces
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    cout << "Server started on port " << PORT << ". Waiting for players...\n";

    vector<thread> threads;

    for (int i = 0; i < MAX_CLIENTS; ++i) {
        new_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen);
        if (new_socket < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }

        {
            lock_guard<mutex> lock(clients_mutex);
            clients.push_back(new_socket);
        }

        threads.emplace_back(handle_client, new_socket, i);
    }

    // All players connected
    broadcast_start();

    // Join threads (in real game you'd likely keep them running)
    for (auto& t : threads) t.join();

    close(server_fd);
    return 0;
}
