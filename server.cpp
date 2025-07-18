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
using namespace std;

const int PORT = 12345;
const int MAX_CLIENTS = 4;

vector<int> client_sockets;
vector<string> playerNames;

// Basic card setup
vector<string> generateDeck() {
    vector<string> suits = {"♠", "♥", "♦", "♣"};
    vector<string> ranks = {"3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K", "A", "2"};
    vector<string> deck;
    for (const auto& s : suits) {
        for (const auto& r : ranks) {
            deck.push_back(r + s);
        }
    }
    return deck;
}

void dealCards(vector<vector<string>>& hands) {
    vector<string> deck = generateDeck();
    shuffle(deck.begin(), deck.end(), default_random_engine(time(0)));

    hands.resize(MAX_CLIENTS);
    for (int i = 0; i < 52; ++i) {
        hands[i % MAX_CLIENTS].push_back(deck[i]);
    }
}

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        cerr << "Socket creation error\n";
        exit(EXIT_FAILURE);
    }

    sockaddr_in address;
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    listen(server_fd, MAX_CLIENTS);
    cout << "Server started on port " << PORT << endl;

    // Accept all players
    while (client_sockets.size() < MAX_CLIENTS) {
        int new_socket = accept(server_fd, nullptr, nullptr);
        if (new_socket >= 0) {
            client_sockets.push_back(new_socket);
            char name_buf[100] = {0};
            read(new_socket, name_buf, 100);
            playerNames.push_back(string(name_buf));
            cout << "Player connected: " << name_buf << endl;
        }
    }

    cout << "All players connected. Game is starting!\n";

    // Deal cards
    vector<vector<string>> hands;
    dealCards(hands);

    // Send cards to players
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        string hand = "Your hand:\n";
        for (const string& card : hands[i]) {
            hand += card + " ";
        }
        hand += "\n";
        send(client_sockets[i], hand.c_str(), hand.size(), 0);
    }

    // Begin simple turn-based loop
    int currentPlayer = 0;
    while (true) {
        string turnMsg = "It's your turn. Enter a card to play:\n";
        send(client_sockets[currentPlayer], turnMsg.c_str(), turnMsg.size(), 0);

        char buffer[1024] = {0};
        int valread = read(client_sockets[currentPlayer], buffer, 1024);
        if (valread <= 0) {
            cout << playerNames[currentPlayer] << " disconnected.\n";
            break;
        }

        string playedCard(buffer, valread);
        string broadcast = playerNames[currentPlayer] + " played: " + playedCard + "\n";

        for (int i = 0; i < MAX_CLIENTS; ++i) {
            if (i != currentPlayer) {
                send(client_sockets[i], broadcast.c_str(), broadcast.size(), 0);
            }
        }

        currentPlayer = (currentPlayer + 1) % MAX_CLIENTS;
    }

    close(server_fd);
    return 0;
}
