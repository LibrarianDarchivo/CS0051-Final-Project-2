#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <string>
using namespace std;

const int PORT = 12345;

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    cout << "Enter your name: ";
    string name;
    getline(cin, name);

    inet_pton(AF_INET, "192.168.254.143", &serv_addr.sin_addr); // Replace IP if needed

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        cerr << "Connection failed\n";
        return 1;
    }

    send(sock, name.c_str(), name.size(), 0);

    char buffer[1024];
    while (true) {
        int valread = read(sock, buffer, sizeof(buffer) - 1);
        if (valread <= 0) break;
        buffer[valread] = '\0';
        cout << buffer;
    }

    close(sock);
    return 0;
}

rizz
