//
// Created by yieatn on 2021/9/8.
//

#ifndef FTP_MYSOCK_H
#define FTP_MYSOCK_H

#include <cstring>
#include <regex>
#include <filesystem>
#include <iostream>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <Windows.h>
#include "WinSock2.h"
#pragma comment(lib, "Ws2_32.lib")

typedef int socklen_t;
#elif defined(__linux__)
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include "arpa/inet.h"
#include <sys/ioctl.h>

typedef int SOCKET;
#endif // Windows/Linux

const int MAXTEXT = 4096;
const int LISTENQ = 8;
//std::string cmdline;


const std::regex space_regex("[^\\s \"]+|\"([^\"]*)\"");

class mysock {
public:
    // ip big-endian; port small-endian
    mysock(uint32_t ip, uint16_t port, uint16_t fa = AF_INET) {
        addr.sin_family = fa;
        addr.sin_addr.s_addr = ip;
        addr.sin_port = port;
        len = sizeof(addr);

        sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sockfd == -1) {
            std::cerr << "Socket generate error" << std::endl;
        }
    }

    mysock() : sockfd(), len(sizeof(addr)) {
        memset(&addr, 0, sizeof addr);
    }

    explicit mysock(SOCKET sock) : sockfd(sock), len(sizeof(addr)) {
        memset(&addr, 0, sizeof addr);
    }

    bool start() {
        if (bind(sockfd, (sockaddr*)&addr, sizeof(addr)) == -1) {
            return false;
        }
        //        printf("n: %d\n", n );
        if (listen(sockfd, LISTENQ) == -1) {
            return false;
        }
        return true;
    }

    int connect() {
        return ::connect(sockfd, (sockaddr*)&addr, sizeof addr);
    }

    int accept() {
        len = sizeof addr;
        return ::accept(sockfd, (sockaddr*)&addr, &len);
    }

    void close() const {
#ifdef _WIN32
        ::closesocket(sockfd);
#elif defined __linux__
        ::close(sockfd);
#endif // _WIN32/ Linux
    }

    //parameter of recv in linux is void*, while it is char* in windows
    int recv(void* buf, size_t _n = MAXTEXT) const {
        //        std::cout << buf << std::endl;
        return ::recv(sockfd, (char*)buf, _n, 0);
    }

    int send(const void* buf, size_t _n = MAXTEXT) {
        //        std::cout << buf << std::endl;
        return ::send(sockfd, (char*)buf, _n, 0);
    }

    uint16_t getPort() {
        int n = getsockname(sockfd, (sockaddr*)&addr, (socklen_t*)&len);
        return addr.sin_port;
    }

public:
    sockaddr_in addr;
    SOCKET sockfd;
    socklen_t len;
};

std::vector<std::string> split(const std::string& s) {
    std::vector<std::string> cmds;
    for_each(std::sregex_iterator(s.begin(), s.end(), space_regex),
             std::sregex_iterator(),
             [&](const auto& match) {
        int p = 0;
        while (match[p].length()) p++;
        cmds.push_back(match[p - 1]);
    });
    return cmds;
}

void getTerminalSize(int& width, int& height) {
#if defined(_WIN32)
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    width = (int)(csbi.srWindow.Right - csbi.srWindow.Left + 1);
    height = (int)(csbi.srWindow.Bottom - csbi.srWindow.Top + 1);
#elif defined(__linux__)
    struct winsize w;
    ioctl(fileno(stdout), TIOCGWINSZ, &w);
    width = (int)(w.ws_col);
    height = (int)(w.ws_row);
#endif // Windows/Linux
}

void show_process_bar(double& _progress) {
    int width, height;
    int barWidth;
    double progress;
    while ((progress = _progress) <= 1.0) {
        getTerminalSize(width, height);
        barWidth = width - 8;

        std::cout << "[";
        int pos = barWidth * progress;
        for (int i = 0; i < barWidth; ++i) {
            if (i < pos) std::cout << "=";
            else if (i == pos) std::cout << ">";
            else std::cout << " ";
        }
        std::cout << "] " << int(progress * 100.0) << " %\r";
        std::cout.flush();
        if (progress == 1.0)
            break;
    }
    std::cout << std::endl;
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
