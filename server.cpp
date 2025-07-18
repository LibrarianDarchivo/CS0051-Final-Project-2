#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <algorithm>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 4

struct ClientInfo {
    int socket;
    std::string name;
};

std::vector<ClientInfo> clients;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void* handle_client(void* arg) {
    int client_socket = *static_cast<int*>(arg);
    delete static_cast<int*>(arg);

    char buffer[BUFFER_SIZE]{};
    if (recv(client_socket, buffer, BUFFER_SIZE, 0) <= 0) {
        close(client_socket);
        return nullptr;
    }

    std::string name(buffer);

    pthread_mutex_lock(&clients_mutex);
    clients.push_back({client_socket, name});
    pthread_mutex_unlock(&clients_mutex);

    return nullptr;
}

int main() {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_socket, MAX_CLIENTS);
    std::cout << "Server waiting for 4 players...\n";

    while (clients.size() < MAX_CLIENTS) {
        sockaddr_in client_addr{};
        socklen_t len = sizeof(client_addr);
        int client_socket = accept(server_socket, (sockaddr*)&client_addr, &len);
        if (client_socket >= 0) {
            int* p = new int(client_socket);
            pthread_t tid;
            pthread_create(&tid, nullptr, handle_client, p);
            pthread_detach(tid);
            sleep(1); // wait for name
        }
    }

    std::map<int, int> scores;
    const int NUM_ROUNDS = 3;
    for (int round = 0; round < NUM_ROUNDS; ++round) {
        for (auto& c : clients)
            send(c.socket, "ROUND_START", 11, 0);

        std::map<int, int> cards;
        for (auto& c : clients) {
            char buf[BUFFER_SIZE] = {};
            int bytes = recv(c.socket, buf, BUFFER_SIZE, 0);
            if (bytes > 0 && strncmp(buf, "CARD:", 5) == 0) {
                int val = std::stoi(buf + 5);
                cards[c.socket] = val;
            }
        }

        int winner_socket = -1;
        int highest = -1;
        for (auto& [sock, val] : cards) {
            if (val > highest) {
                highest = val;
                winner_socket = sock;
            }
        }
        scores[winner_socket]++;

        std::string msg = "[Round " + std::to_string(round + 1) + "] Winner: ";
        for (auto& c : clients)
            if (c.socket == winner_socket)
                msg += c.name;
        msg += " with card " + std::to_string(highest) + "\nScores:\n";
        for (auto& c : clients)
            msg += c.name + ": " + std::to_string(scores[c.socket]) + "\n";

        for (auto& c : clients)
            send(c.socket, msg.c_str(), msg.length(), 0);
    }

    std::string final = "GAME_OVER\nFinal Scores:\n";
    int maxScore = -1;
    for (auto& [sock, score] : scores)
        if (score > maxScore) maxScore = score;

    for (auto& c : clients)
        final += c.name + ": " + std::to_string(scores[c.socket]) + "\n";

    final += "Winner(s): ";
    for (auto& c : clients)
        if (scores[c.socket] == maxScore)
            final += c.name + " ";
    final += "\n";

    for (auto& c : clients)
        send(c.socket, final.c_str(), final.length(), 0);

    close(server_socket);
    return 0;
}
