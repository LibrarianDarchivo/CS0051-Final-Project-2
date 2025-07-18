#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <random>
#include <ctime>
#include <thread>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>

#define PORT 12345
#define MAX_CLIENTS 4

using namespace std;

// ----- Card Struct -----
struct Card {
    string suit;
    string rank;

    string toString() const {
        return rank + " of " + suit;
    }
};

// ----- Generate a deck of cards -----
vector<Card> generateDeck() {
    vector<Card> deck;
    vector<string> suits = {"Hearts", "Diamonds", "Clubs", "Spades"};
    vector<string> ranks = {
        "3", "4", "5", "6", "7", "8", "9", "10",
        "J", "Q", "K", "A", "2"
    };

    for (const auto& suit : suits) {
        for (const auto& rank : ranks) {
            deck.push_back({suit, rank});
        }
    }

    // Shuffle
    random_device rd;
    mt19937 g(rd());
    shuffle(deck.begin(), deck.end(), g);

    return deck;
}

// ----- Send hand to client -----
void sendHand(int socket, const vector<Card>& hand, const string& playerName) {
    string msg = "Hello " + playerName + ", your hand:\n";
    for (const auto& card : hand) {
        msg += "- " + card.toString() + "\n";
    }
    send(socket, msg.c_str(), msg.size(), 0);
}

// ----- Main -----
int main() {
    int server_fd, new_socket;
    sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    vector<int> client_sockets;
    vector<string> playerNames;

    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

    // Bind
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen
    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    cout << "Server started on port " << PORT << ". Waiting for players...\n";

    // Accept clients
    while (client_sockets.size() < MAX_CLIENTS) {
        new_socket = accept(server_fd, (sockaddr*)&address, (socklen_t*)&addrlen);
        if (new_socket < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }

        // Receive player name
        char nameBuffer[100] = {0};
        int nameLen = read(new_socket, nameBuffer, sizeof(nameBuffer));
        string playerName = string(nameBuffer, nameLen);

        client_sockets.push_back(new_socket);
        playerNames.push_back(playerName);

        cout << "Player " << client_sockets.size() << " connected as \"" << playerName << "\"\n";
    }

    cout << "All players connected. Dealing cards...\n";

    // Generate and deal deck
    vector<Card> deck = generateDeck();

    for (int i = 0; i < MAX_CLIENTS; ++i) {
        vector<Card> hand(deck.begin() + (i * 13), deck.begin() + (i + 1) * 13);
        sendHand(client_sockets[i], hand, playerNames[i]);
    }

    cout << "Cards sent. Game setup complete.\n";

    // Close all client sockets after sending hands (for now)
    for (int sock : client_sockets) {
        close(sock);
    }

    close(server_fd);
    return 0;
}
