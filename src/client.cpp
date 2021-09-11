#include <iostream>
#include <fstream>
#include "mysock.h"
#include <chrono>

using namespace std;
namespace fs = filesystem;

char buf[MAXTEXT];
char currdir[MAXTEXT];
    std::string cmdline;

int main(int args, char **argv) {

//    string cmdline;
    cmdline.resize(MAXTEXT);

    if (args != 3) {
        cout << "Usage: ./client <ip> <port>\n";
        exit(1);
    }

    mysock connfd(inet_addr(argv[1]), atoi(argv[2]));

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

            size_t chunk, rest;
            connfd.recv(&chunk, sizeof(size_t));
            connfd.recv(&rest, sizeof(size_t));
            perror("Error: ");

            auto startTime = chrono::high_resolution_clock::now();

            for (size_t i = 0; i < chunk; ++i) {
                connfd.recv(buf, MAXTEXT);
                out.write(buf, MAXTEXT);
//                show_process_bar((double)(i+1)/(double)chunk);
            }
            connfd.recv(buf, rest);
            out.write(buf, rest);

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


            cout << "File download done, Total time: "
                 << totalTime
                 << "ms, Average speed: "
                 << chunk * MAXTEXT / 1024 / 1024 * 1000 / totalTime
                 << " MiB/s" << endl;
        }
        cout << "ftp>" ;
    }



    return 0;

}
