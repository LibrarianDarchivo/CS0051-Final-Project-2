#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <cstdlib>
#include <ctime>
#include <algorithm>
using namespace std;

const int PORT = 12345;
const int MAX_CLIENTS = 4;

vector<int> client_sockets;
vector<string> playerNames(MAX_CLIENTS);
vector<int> playerScores(MAX_CLIENTS, 0);

void broadcast(const string &message) {
    for (int sock : client_sockets) {
        send(sock, message.c_str(), message.size(), 0);
    }
}

void handleGame() {
    srand(time(0));
    string startMsg = "\n=== Pusoy Clash Game Simulation ===\n";
    broadcast(startMsg);
    sleep(1);

    for (int round = 1; round <= 3; ++round) {
        broadcast("\n--- Starting Round " + to_string(round) + " ---\n");
        vector<int> cards(MAX_CLIENTS);

        for (int i = 0; i < MAX_CLIENTS; ++i) {
            cards[i] = rand() % 13 + 2;
            broadcast(playerNames[i] + " draws a card: " + to_string(cards[i]) + "\n");
            sleep(1);
        }

        int maxCard = *max_element(cards.begin(), cards.end());
        vector<int> winners;
        for (int i = 0; i < MAX_CLIENTS; ++i) {
            if (cards[i] == maxCard) {
                winners.push_back(i);
            }
        }

        if (winners.size() == 1) {
            playerScores[winners[0]]++;
            broadcast("\nRound " + to_string(round) + " winner: " + playerNames[winners[0]] +
                      " with card " + to_string(maxCard) + "!\n");
        } else {
            broadcast("\nRound " + to_string(round) + " is a tie!\n");
        }
        sleep(2);
    }

    broadcast("\n=== Final Scores ===\n");
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        broadcast(playerNames[i] + ": " + to_string(playerScores[i]) + " points\n");
    }

    int maxScore = *max_element(playerScores.begin(), playerScores.end());
    vector<int> finalWinners;
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (playerScores[i] == maxScore) {
            finalWinners.push_back(i);
        }
    }

    if (finalWinners.size() == 1) {
        broadcast("\n" + playerNames[finalWinners[0]] + " won with " +
                  to_string(maxScore) + " points!\n");
    } else {
        broadcast("\nIt's a tie between:\n");
        for (int i : finalWinners) {
            broadcast("- " + playerNames[i] + "\n");
        }
    }
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, MAX_CLIENTS);

    cout << "Server started on port " << PORT << ". Waiting for players...\n";

    while (client_sockets.size() < MAX_CLIENTS) {
        new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);

        char nameBuffer[1024] = {0};
        read(new_socket, nameBuffer, sizeof(nameBuffer));
        string playerName(nameBuffer);

        client_sockets.push_back(new_socket);
        playerNames[client_sockets.size() - 1] = playerName;

        string joinMsg = playerName + " has joined the game!\n";
        cout << joinMsg;
        broadcast(joinMsg);
    }

    broadcast("\nAll players connected. Game is starting!\n");
    handleGame();

    for (int sock : client_sockets) close(sock);
    close(server_fd);
    return 0;
}

rizz
