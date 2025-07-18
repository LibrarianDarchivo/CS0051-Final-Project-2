#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef int socklen_t;
#else
    #include <unistd.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
#endif

#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <algorithm>
#include <random>
#include <map>
using namespace std;

#define PORT 8080
#define MAX_PLAYERS 4

vector<SOCKET> clients;
vector<string> playerNames;
mutex mtx;

// Broadcast message to all clients
void broadcast(const string& message) {
    for (SOCKET client : clients) {
        send(client, message.c_str(), message.size(), 0);
    }
}

// Handle client connection
void handleClient(SOCKET clientSocket) {
    char nameBuffer[100] = {0};
    recv(clientSocket, nameBuffer, sizeof(nameBuffer), 0);

    lock_guard<mutex> lock(mtx);
    clients.push_back(clientSocket);
    playerNames.push_back(nameBuffer);
    cout << nameBuffer << " has joined the game.\n";

    // Notify all clients about the new player
    string joinMsg = nameBuffer;
    joinMsg += " has joined the game.\n";
    broadcast(joinMsg);
}

int main() {
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        cerr << "Socket creation failed.\n";
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "Bind failed.\n";
        return 1;
    }

    listen(serverSocket, MAX_PLAYERS);
    cout << "Server started on port " << PORT << ". Waiting for players...\n";

    vector<thread> threads;

    // Accept connections until we have MAX_PLAYERS
    while (clients.size() < MAX_PLAYERS) {
        sockaddr_in clientAddr;
        socklen_t clientSize = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientSize);
        if (clientSocket != INVALID_SOCKET) {
            threads.emplace_back(handleClient, clientSocket);
        }
    }

    // Wait for all threads to complete
    for (auto& t : threads) t.join();

    broadcast("All players connected. Game is starting!\n\n");

    map<int, int> scores;
    for (int round = 1; round <= 3; ++round) {
        vector<int> cards;
        string roundMsg = "--- Starting Round " + to_string(round) + " ---\n";

        // Generate and announce cards
        for (int i = 0; i < MAX_PLAYERS; ++i) {
            int card = rand() % 14 + 1;
            cards.push_back(card);
            roundMsg += playerNames[i] + " draws a card: " + to_string(card) + "\n";
        }

        // Find winner for the round
        int maxCard = *max_element(cards.begin(), cards.end());
        vector<int> winners;
        for (int i = 0; i < MAX_PLAYERS; ++i) {
            if (cards[i] == maxCard) winners.push_back(i);
        }

        if (winners.size() == 1) {
            scores[winners[0]]++;
            roundMsg += "\nRound " + to_string(round) + " winner: " + playerNames[winners[0]] + " with card " + to_string(maxCard) + "!\n\n";
        } else {
            roundMsg += "\nRound " + to_string(round) + " is a tie!\n\n";
        }

        broadcast(roundMsg);
    }

    // Final score summary
    string finalMsg = "=== Final Scores ===\n";
    int highestScore = 0;
    int winnerIndex = -1;
    for (int i = 0; i < MAX_PLAYERS; ++i) {
        finalMsg += playerNames[i] + ": " + to_string(scores[i]) + " points\n";
        if (scores[i] > highestScore) {
            highestScore = scores[i];
            winnerIndex = i;
        }
    }

    finalMsg += "\n";
    if (winnerIndex != -1) {
        finalMsg += playerNames[winnerIndex] + " won with " + to_string(highestScore) + " points!\n";
    } else {
        finalMsg += "It's a tie!\n";
    }

    broadcast(finalMsg);

    // Close sockets
    for (SOCKET client : clients) {
        closesocket(client);
    }
    closesocket(serverSocket);

#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}
