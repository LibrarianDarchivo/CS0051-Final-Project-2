#include <iostream>
#include <string>
using namespace std;

#ifdef _WIN32 // For Windows PCs
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else // For Ubuntu/Linux PCs
    #include <unistd.h>
    #include <arpa/inet.h>
    #include <sys/socket.h>
#endif

const int PORT = 12345; // Port number must match server for clients to be able to connect

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;

    sock = socket(AF_INET, SOCK_STREAM, 0);

    // Set server info
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT); // Convert port to network byte order

    // Get player name
    cout << "Enter your name: ";
    string name;
    getline(cin, name);

    // Convert IP from text to binary
    inet_pton(AF_INET, "192.168.254.143", &serv_addr.sin_addr); // Replace IP if needed

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        cerr << "Connection failed\n";
        return 1;
    }

    // Send player name to server
    send(sock, name.c_str(), name.size(), 0);

    // Listens for messages from the server and prints them to the console
    char buffer[1024];
    while (true) {
        int valread = read(sock, buffer, sizeof(buffer) - 1); // Read data from server
        if (valread <= 0) break; // Exit on disconnect
        buffer[valread] = '\0';  // Null-terminate string
        cout << buffer;          // Output message
    }

    // Close connection
    close(sock);
    return 0;
}
