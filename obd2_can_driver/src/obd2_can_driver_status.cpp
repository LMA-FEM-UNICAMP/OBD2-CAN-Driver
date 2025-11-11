#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <iostream>

static const char *LOG_SOCK_PATH = "/tmp/obd2_can_logging.sock";

int main() {
    
    unlink(LOG_SOCK_PATH);

    int log_sock = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (log_sock < 0) {
        std::cerr << "socket(): " << strerror(errno) << "\n";
        return 1;
    }

    sockaddr_un log_addr{};
    log_addr.sun_family = AF_UNIX;
    strncpy(log_addr.sun_path, LOG_SOCK_PATH, sizeof(log_addr.sun_path) - 1);

    if (bind(log_sock, (sockaddr*)&log_addr, sizeof(log_addr)) < 0) {
        std::cerr << "bind(): " << strerror(errno) << "\n";
        close(log_sock);
        return 1;
    }
    
    char log_buffer[2048];
    sockaddr_un log_sender{};
    socklen_t sender_len = sizeof(log_sender);


    while (true) {
        ssize_t n = recvfrom(log_sock, log_buffer, sizeof(log_buffer) - 1, 0,
                             (sockaddr*)&log_sender, &sender_len);
        if (n < 0) {
            if (errno == EINTR)
                continue;
            std::cerr << "recvfrom(): " << strerror(errno) << "\n";
            break;
        }

        log_buffer[n] = '\0';
        std::cout << log_buffer;
    }

    
    close(log_sock);
    unlink(LOG_SOCK_PATH);
    return 0;
}
