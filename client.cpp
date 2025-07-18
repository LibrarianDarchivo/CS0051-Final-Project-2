#include <iostream>
#include <string>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctime>

#define SERVER_IP "127.0.0.1"
#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    srand(time(0));

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr);

    if (connect(sock, (sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Failed to connect to server\n";
        return 1;
    }

    std::string name;
    std::cout << "Enter your name: ";
    std::getline(std::cin, name);
    send(sock, name.c_str(), name.length(), 0);

    char buffer[BUFFER_SIZE];
    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes = recv(sock, buffer, BUFFER_SIZE, 0);
        if (bytes <= 0) break;

        std::string msg(buffer);
        if (msg == "ROUND_START") {
            int card = (rand() % 13) + 2;
            std::cout << "[You drew]: " << card << std::endl;
            std::string cardMsg = "CARD:" + std::to_string(card);
            send(sock, cardMsg.c_str(), cardMsg.length(), 0);
        } else {
            std::cout << msg << std::endl;
            if (msg.find("GAME_OVER") != std::string::npos)
                break;
        }
    }

    close(sock);
    return 0;
}
