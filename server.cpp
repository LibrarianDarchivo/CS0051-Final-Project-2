#ifdef _WIN32 // For Windows
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef int socklen_t;
#else // Ubuntu, etc.
    #include <unistd.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <sys/socket.h>
#endif

#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <map>
using namespace std;

const int PORT = 12345;
const int MAX_PLAYERS = 4;
vector<int> clientSockets;
vector<string> playerNames;
map<int, int> playerScores;
mutex gameMutex;

// Send message to one client
void sendToClient(int clientSocket, const string& message) {
    send(clientSocket, message.c_str(), message.size(), 0);
}

// Send message to all clients
void broadcast(const string& message) {
    for (int sock : clientSockets) {
        sendToClient(sock, message);
    }
}

// Handle individual clients connection
void handleClient(int clientSocket) {
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));

    // Gets player name
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
    string name = (bytesReceived > 0) ? string(buffer, bytesReceived) : "Player";
    
    {
        lock_guard<mutex> lock(gameMutex);
        playerNames.push_back(name);
        playerScores[clientSocket] = 0;
    }

    cout << name << " has joined the game!" << endl;
}

int main() {
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

    srand(static_cast<unsigned>(time(0)));

    // Create server socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        cerr << "Failed to create socket.\n";
        return 1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    // Bind socket to IP/port
    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        cerr << "Bind failed.\n";
        return 1;
    }

    // Listens for connections on specified port number
    listen(serverSocket, MAX_PLAYERS);
    cout << "Server started on port " << PORT << ". Waiting for players...\n";

    // Accept connections
    while (clientSockets.size() < MAX_PLAYERS) {
        sockaddr_in clientAddr{};
        socklen_t clientSize = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientSize);
        if (clientSocket >= 0) {
            clientSockets.push_back(clientSocket);
            thread(handleClient, clientSocket).detach();
        }
    }

    // Waits to ensure all names are received
    this_thread::sleep_for(chrono::seconds(2));
    broadcast("\nAll players connected. Game is starting!\n");

    for (int round = 1; round <= 3; ++round) {
        broadcast("\n--- Starting Round " + to_string(round) + " ---\n");

        int highestCard = -1;
        int winnerSocket = -1;
        map<int, int> drawnCards;

        for (int sock : clientSockets) {
            int card = rand() % 13 + 2;
            drawnCards[sock] = card;

            string name = playerNames[distance(clientSockets.begin(), find(clientSockets.begin(), clientSockets.end(), sock))];
            broadcast(name + " draws a card: " + to_string(card) + "\n");

            if (card > highestCard) {
                highestCard = card;
                winnerSocket = sock;
            }
        }

        // Point allocation/Awarding
        playerScores[winnerSocket]++;
        string winnerName = playerNames[distance(clientSockets.begin(), find(clientSockets.begin(), clientSockets.end(), winnerSocket))];
        broadcast("\nRound " + to_string(round) + " winner: " + winnerName + " with card " + to_string(highestCard) + "!\n");
    }

    // Show final scores
    broadcast("\n=== Final Scores ===\n");
    int topScore = -1;
    string topPlayer;
    for (size_t i = 0; i < clientSockets.size(); ++i) {
        string name = playerNames[i];
        int score = playerScores[clientSockets[i]];
        broadcast(name + ": " + to_string(score) + " points\n");

        if (score > topScore) {
            topScore = score;
            topPlayer = name;
        }
    }

    broadcast("\n" + topPlayer + " won with " + to_string(topScore) + " points!\n");

    // Cleanup
    for (int sock : clientSockets) {
#ifdef _WIN32
        closesocket(sock);
#else
        close(sock);
#endif
    }

#ifdef _WIN32
    closesocket(serverSocket);
    WSACleanup();
#else
    close(serverSocket);
#endif

    return 0;
}
