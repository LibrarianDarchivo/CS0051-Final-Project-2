#include <iostream>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 12345

using namespace std;

int main() {
    int sock = 0;
    sockaddr_in serv_addr;
    char buffer[4096] = {0};

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        cerr << "Socket creation error\n";
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "192.168.254.143", &serv_addr.sin_addr) <= 0) {
        cerr << "Invalid address\n";
        return -1;
    }

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        cerr << "Connection Failed\n";
        return -1;
    }

    cout << "Connected to server.\n";

    cout << "Enter your name: ";
    string name;
    getline(cin, name);
    send(sock, name.c_str(), name.length(), 0);

    int valread = read(sock, buffer, sizeof(buffer));
    if (valread > 0) {
        cout << string(buffer, valread) << endl;
    }

    close(sock);
    return 0;
}
