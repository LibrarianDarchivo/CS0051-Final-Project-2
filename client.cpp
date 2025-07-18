#include <iostream>
#include <string>
#include <unistd.h>
#include <arpa/inet.h>
using namespace std;

const int PORT = 12345;

int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        cerr << "Socket creation error\n";
        return 1;
    }

    sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Replace this with your server's actual IP
    string server_ip = "192.168.254.143";
    inet_pton(AF_INET, server_ip.c_str(), &serv_addr.sin_addr);

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        cerr << "Failed to connect to server\n";
        return 1;
    }

    cout << "Enter your name: ";
    string name;
    getline(cin, name);
    send(sock, name.c_str(), name.size(), 0);

    while (true) {
        char buffer[1024] = {0};
        int valread = read(sock, buffer, 1024);
        if (valread <= 0) break;

        string serverMsg(buffer, valread);
        cout << serverMsg;

        if (serverMsg.find("your turn") != string::npos) {
            string move;
            cout << ">> ";
            getline(cin, move);
            send(sock, move.c_str(), move.size(), 0);
        }
    }

    close(sock);
    return 0;
}
