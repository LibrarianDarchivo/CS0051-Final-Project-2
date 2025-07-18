#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 12345
#define MAX_CLIENTS 4
#define BUFFER_SIZE 1024

struct Client {
    int socket;
    std::string name;
};

std::vector<Client> clients;
std::mutex clients_mutex;
int current_turn = 0;
bool game_started = false;

void broadcast_message(const std::string& sender, const std::string& message, int exclude_socket = -1) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    std::string full_message = sender + ": " + message;
    for (auto& client : clients) {
        if (client.socket != exclude_socket) {
            send(client.socket, full_message.c_str(), full_message.length(), 0);
        }
    }
}

void handle_client(int client_socket) {
    char name_buf[50];
    memset(name_buf, 0, sizeof(name_buf));
    recv(client_socket, name_buf, sizeof(name_buf), 0);

    std::string client_name = name_buf;

    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        clients.push_back({client_socket, client_name});

        std::cout << client_name << " has joined. Total: " << clients.size() << "\n";

        if (!game_started && clients.size() == MAX_CLIENTS) {
            game_started = true;
            broadcast_message("Server", "All 4 players joined! Game starting...\n");
            broadcast_message("Server", "Turn order: Player 1 → 2 → 3 → 4\n");
            broadcast_message("Server", "Player " + clients[current_turn].name + "'s turn.\n");
        }
    }

    char buffer[BUFFER_SIZE];
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) break;

        std::string input = buffer;

        std::lock_guard<std::mutex> lock(clients_mutex);
        if (game_started) {
            if (client_socket == clients[current_turn].socket) {
                broadcast_message(client_name, input);

                if (input == "win\n" || input == "win\r\n") {
                    broadcast_message("Server", client_name + " has won the game! Game over.");
                    close(client_socket);
                    break;
                }

                current_turn = (current_turn + 1) % clients.size();
                broadcast_message("Server", "Now it's " + clients[current_turn].name + "'s turn.\n");
            } else {
                std::string not_your_turn = "Not your turn.\n";
                send(client_socket, not_your_turn.c_str(), not_your_turn.size(), 0);
            }
        } else {
            broadcast_message(client_name, input, client_socket);
        }
    }

    close(client_socket);
    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        clients.erase(std::remove_if(clients.begin(), clients.end(),
            [client_socket](const Client& c) { return c.socket == client_socket; }), clients.end());
        std::cout << client_name << " has disconnected.\n";
    }
}

int main() {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in server_addr{}, client_addr{};
    socklen_t addr_len = sizeof(client_addr);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_socket, MAX_CLIENTS);

    std::cout << "Server started on port " << PORT << ". Waiting for players...\n";

    while (true) {
        int client_socket = accept(server_socket, (sockaddr*)&client_addr, &addr_len);
        std::thread(handle_client, client_socket).detach();
    }

    close(server_socket);
    return 0;
}
