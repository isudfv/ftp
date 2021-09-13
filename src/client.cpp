#include <iostream>
#include <fstream>
#include "mysock.h"
#include <chrono>
#include <thread>

using namespace std;
namespace fs = filesystem;

char buf[MAXTEXT + 10];
char currdir[MAXTEXT + 10];
std::string cmdline;

void test() {
    cout << "TEST" << endl;
}


int main(int args, char **argv) {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Bind or Listen function failed with error %d\n", WSAGetLastError());
    }
#endif //windows

//    string cmdline;
    cmdline.resize(MAXTEXT + 10);

    if (args != 3) {
        cout << "Usage: ./client <ip> <port>\n";
        exit(1);
    }

    mysock connfd(inet_addr(argv[1]), htons(atoi(argv[2])));

    if (connfd.connect() < 0) {
        cerr << "Problem in connecting to the server" << endl;
        exit(3);
    }

    cout << "ftp>";

    while (getline(cin, cmdline)) {
//        const char *temp = cmdline.c_str();
        auto n = connfd.send(cmdline.c_str(), MAXTEXT);
        auto cmds = split(cmdline);

        if (cmds[0] == "ls") {
            while (connfd.recv(buf, MAXTEXT)) {
                if (strcmp(buf, "STOP_END_OF_FILE") == 0)
                    break;
                cout << buf << endl;
            }
        }

        if (cmds[0] == "pwd") {
            connfd.recv(currdir, MAXTEXT);
            cout << currdir << endl;
        }

        if (cmds[0] == "get") {
            fstream out(cmds[2], ios::out | ios::binary);

            auto startTime = chrono::high_resolution_clock::now();

            uint16_t _port;
            connfd.recv(&_port, sizeof(_port)) ;
//            cout << _port << endl;
            mysock datafd(inet_addr(argv[1]), _port);
            if (datafd.connect() < 0) {
                cerr << "Problem in connecting to the server data trans socket" << endl;
                exit(3);
            }

            cout << "Starting transferring files" << endl;
            size_t fileSize, sendSize = 0;
            datafd.recv(&fileSize, sizeof(size_t));

            double ratio = 0;
//            thread bar(test);
            thread bar(show_process_bar, std::ref(ratio));

            while ((n = datafd.recv(buf, MAXTEXT)) > 0) {
//                cout << n << endl;
                out.write(buf, n);
                sendSize += n;
                ratio = (double)sendSize / fileSize;
            }

            bar.join();


            auto endTime = chrono::high_resolution_clock::now();
            auto totalTime = chrono::duration_cast<chrono::milliseconds>(endTime - startTime).count();

            out.close();

            /*{
                std::ifstream file1(cmds[1]);
                std::ifstream file2(cmds[2]);

                std::istreambuf_iterator<char> begin1(file1);
                std::istreambuf_iterator<char> begin2(file2);

                std::istreambuf_iterator<char> end;

                if (range_equal(begin1, end, begin2, end)){
                    cout << "equal\n";
                } else {
                    cout << "not equal\n";
                }
            }*/


//            perror("Error: ");
            cout << "File download done, Total time: "
                 << totalTime
                 << "ms, Average speed: "
                 << fileSize / 1024 / 1024 * 1000 / totalTime
                 << " MiB/s" << endl;
        }
        cout << "ftp>" ;
    }



    return 0;

}
