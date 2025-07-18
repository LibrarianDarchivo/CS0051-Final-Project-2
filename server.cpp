#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <cstdlib>
#include <ctime>
#include <algorithm>
using namespace std;

#ifdef _WIN32 // For Windows PCs
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else // For Ubuntu/Linux/Unix PCs
    #include <unistd.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <netinet/in.h>
    #include <sys/socket.h>
#endif

const int PORT = 12345;
const int MAX_CLIENTS = 4; // Max players allowed in a "session"

vector<int> client_sockets; // Stores connected client socket descriptors
vector<string> playerNames(MAX_CLIENTS);  // Stores player names
vector<int> playerScores(MAX_CLIENTS, 0); // Tracks player scores

// Sends message to all connected clients
void broadcast(const string &message) {
    for (int sock : client_sockets) {
        send(sock, message.c_str(), message.size(), 0);
    }
}

void handleGame() {
    srand(time(0));  // Seed RNG
    string startMsg = "\n=== Pusoy Clash Game Simulation ===\n";
    broadcast(startMsg);
    sleep(1);

    for (int round = 1; round <= 3; ++round) {
        broadcast("\n--- Starting Round " + to_string(round) + " ---\n");
        vector<int> cards(MAX_CLIENTS); // Store a drawn card per player

        for (int i = 0; i < MAX_CLIENTS; ++i) {
            cards[i] = rand() % 13 + 2; // Random card between 2 and 14
            broadcast(playerNames[i] + " draws a card: " + to_string(cards[i]) + "\n");
            sleep(1);
        }

        int maxCard = *max_element(cards.begin(), cards.end()); // Find highest card
        vector<int> winners;

        // Checks which player has the highest card
        for (int i = 0; i < MAX_CLIENTS; ++i) {
            if (cards[i] == maxCard) {
                winners.push_back(i);
            }
        }

        // If one winner, award point
        if (winners.size() == 1) {
            playerScores[winners[0]]++;
            broadcast("\nRound " + to_string(round) + " winner: " + playerNames[winners[0]] +
                      " with card " + to_string(maxCard) + "!\n");
        } else {
            broadcast("\nRound " + to_string(round) + " is a tie!\n");
        }
        sleep(2);
    }

    // Show final score
    broadcast("\n=== Final Scores ===\n");
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        broadcast(playerNames[i] + ": " + to_string(playerScores[i]) + " points\n");
    }

    // Determine winner
    int maxScore = *max_element(playerScores.begin(), playerScores.end());
    vector<int> finalWinners;
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (playerScores[i] == maxScore) {
            finalWinners.push_back(i);
        }
    }

    // Announce if win or tie
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
    address.sin_port = htons(PORT); // Bind to port
    bind(server_fd, (struct sockaddr *)&address, sizeof(address)); // Bind socket
    listen(server_fd, MAX_CLIENTS); // Start listening

    cout << "Server started on port " << PORT << ". Waiting for players...\n";

    // Accept up to MAX_CLIENTS players
    while (client_sockets.size() < MAX_CLIENTS) {
        new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);

        // Receive player name
        char nameBuffer[1024] = {0};
        read(new_socket, nameBuffer, sizeof(nameBuffer));
        string playerName(nameBuffer);

        // Store new client and name
        client_sockets.push_back(new_socket);
        playerNames[client_sockets.size() - 1] = playerName;

        string joinMsg = playerName + " has joined the game!\n";
        cout << joinMsg;
        broadcast(joinMsg);
    }

    // When all players are connected - start game
    broadcast("\nAll players connected. Game is starting!\n");
    handleGame();

    // Close all client sockets and server socket after a game
    for (int sock : client_sockets) close(sock);
    close(server_fd);
    return 0;
}
