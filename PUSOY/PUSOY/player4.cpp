#include <iostream>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
using namespace std;

int main() {
    string pipe_name = "/tmp/player4_pipe";
    char buffer[100];

    int fd_write = open(pipe_name.c_str(), O_WRONLY);
    string ready = "READY";
    write(fd_write, ready.c_str(), ready.length() + 1);
    close(fd_write);

    int fd_read = open(pipe_name.c_str(), O_RDONLY);
    read(fd_read, buffer, sizeof(buffer));
    close(fd_read);

    cout << "Player 4 received card: " << buffer << endl;
    return 0;
}
