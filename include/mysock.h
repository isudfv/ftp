//
// Created by yieatn on 2021/9/8.
//

#ifndef FTP_MYSOCK_H
#define FTP_MYSOCK_H

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include "arpa/inet.h"
#include <cstring>
#include <regex>
#include <unistd.h>
#include <filesystem>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <Windows.h>
#elif defined(__linux__)

#include <sys/ioctl.h>

#endif // Windows/Linux

const int MAXTEXT = 4096;
const int LISTENQ = 8;
//std::string cmdline;


const std::regex space_regex("[^\\s \"]+|\"([^\"]*)\"");

class mysock {
public:
    // ip big-endian; port small-endian
    mysock(uint32_t ip, uint16_t port, sa_family_t fa = AF_INET) {
        addr.sin_family = fa;
        addr.sin_addr.s_addr = ip;
        addr.sin_port = htons(port);

        sockfd = socket(AF_INET, SOCK_STREAM, 0);
    }

    mysock() : sockfd() {
        memset(&addr, 0, sizeof addr);
    }

    explicit mysock(int sock) : sockfd(sock) {
        memset(&addr, 0, sizeof addr);
    }

    void start() {
        bind(sockfd, (sockaddr *) &addr, sizeof(addr));
        listen(sockfd, LISTENQ);
    }

    bool connect() {
        return ::connect(sockfd, (sockaddr *) &addr, sizeof addr);
    }

    int accept() {
        len = sizeof addr;
        return ::accept(sockfd, (sockaddr *) &addr, &len);
    }

    void close() {
        ::close(sockfd);
    }

    int recv(void *buf, size_t _n = MAXTEXT) {
//        std::cout << buf << std::endl;
        return ::recv(sockfd, buf, _n, 0);
    }

    int send(const void *buf, size_t _n = MAXTEXT) {
//        std::cout << buf << std::endl;
        return ::send(sockfd, buf, _n, 0);
    }

public:
    sockaddr_in addr{};
    int sockfd;
    socklen_t len;
};

std::vector<std::string> split(const std::string &s) {
    std::vector<std::string> cmds;
    for_each(std::sregex_iterator(s.begin(), s.end(), space_regex),
             std::sregex_iterator(),
             [&](const auto &match) {
                 int p = 0;
                 while (match[p].length()) p++;
                 cmds.push_back(match[p - 1]);
             });
    return cmds;
}

void show_process_bar(double progress) {
    static int barWidth = 70;

    std::cout << "[";
    int pos = barWidth * progress;
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << int(progress * 100.0) << " %\r";
    std::cout.flush();
}

void get_terminal_size(int &width, int &height) {
#if defined(_WIN32)
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    width = (int)(csbi.srWindow.Right-csbi.srWindow.Left+1);
    height = (int)(csbi.srWindow.Bottom-csbi.srWindow.Top+1);
#elif defined(__linux__)
    struct winsize w;
    ioctl(fileno(stdout), TIOCGWINSZ, &w);
    width = (int) (w.ws_col);
    height = (int) (w.ws_row);
#endif // Windows/Linux
}

template<typename InputIterator1, typename InputIterator2>
bool range_equal(InputIterator1 first1, InputIterator1 last1,
                 InputIterator2 first2, InputIterator2 last2) {
    while (first1 != last1 && first2 != last2) {
        if (*first1 != *first2) return false;
        ++first1;
        ++first2;
    }
    return (first1 == last1) && (first2 == last2);
}

#endif //FTP_MYSOCK_H
