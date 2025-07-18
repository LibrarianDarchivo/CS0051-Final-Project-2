#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <random>
#include <thread>
#include <mutex>
#include <netinet/in.h>
#include <unistd.h>

#define PORT 12345
#define MAX_CLIENTS 4

using namespace std;

struct Card {
    string suit;
    string rank;

    string toString() const {
        return rank + " of " + suit;
    }
};

vector<Card> generateDeck() {
    vector<string> suits = {"Spades", "Hearts", "Diamonds", "Clubs"};
    vector<string> ranks = {"3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K", "A", "2"};

    vector<Card> deck;
    for (const auto& suit : suits) {
        for (const auto& rank : ranks) {
            deck.push_back({suit, rank});
        }
    }

    // Shuffle deck
    random_device rd;
    mt19937 g(rd());
    shuffle(deck.begin(), deck.end(), g);

    return deck;
}

void sendHand(int socket, const vector<Card>& hand) {
    string msg = "Your hand:\n";
    for (const auto& card : hand) {
        msg += "- " + card.toString() + "\n";
    }
    send(socket, msg.c_str(), msg.size(), 0);
}

int main() {
    int server_fd, new_socket;
    sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    vector<int> client_sockets;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    cout << "Server ready on port " << PORT << ". Waiting for players...\n";

    // Accept 4 players
    while (client_sockets.size() < MAX_CLIENTS) {
        new_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen);
        if (new_socket < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }
        client_sockets.push_back(new_socket);
        cout << "Player " << client_sockets.size() << " connected.\n";
    }

    // Generate and shuffle deck
    vector<Card> deck = generateDeck();

    // Deal 13 cards each
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        vector<Card> hand(deck.begin() + (i * 13), deck.begin() + (i + 1) * 13);
        sendHand(client_sockets[i], hand);
    }

    cout << "Cards dealt. Game setup complete.\n";

    close(server_fd);
    return 0;
}
