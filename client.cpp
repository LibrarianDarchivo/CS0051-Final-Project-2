#ifdef _WIN32 // For Windows
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else // Ubuntu, etc.
    #include <unistd.h>
    #include <arpa/inet.h>
    #include <sys/socket.h>
#endif

#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <cstring>
using namespace std;

const int PORT = 12345;
const string SERVER_IP = "192.168.254.143"; // Change to your server's IP

// Receive and print server messages continuously
void receiveMessages(int socket) {
    char buffer[1024];
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytesReceived = recv(socket, buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0) {
            cout << "Disconnected from server." << endl;
            break;
        }
        cout << string(buffer, bytesReceived);
    }
}

int main() {
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

    // Create client socket
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        cerr << "Failed to create socket." << endl;
        return 1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);

    // Convert IP string to binary
    if (inet_pton(AF_INET, SERVER_IP.c_str(), &serverAddr.sin_addr) <= 0) {
        cerr << "Invalid IP address." << endl;
        return 1;
    }

    // Connect to the server
    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        cerr << "Failed to connect to server." << endl;
        return 1;
    }

    // Program asks player's name
    string playerName;
    cout << "Enter your name: ";
    getline(cin, playerName);
    send(clientSocket, playerName.c_str(), playerName.length(), 0);

    // Starts thread to listen to server messages
    thread receiver(receiveMessages, clientSocket);

    // Wait for the game to finish
    receiver.join();

    // End connection
#ifdef _WIN32
    closesocket(clientSocket);
    WSACleanup();
#else
    close(clientSocket);
#endif

    return 0;
}
