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
            connfd.recv((char *)&chunk, sizeof(size_t));
            connfd.recv((char *)&rest, sizeof(size_t));

            cout << chunk << endl
                 << rest << endl;

            auto startTime = chrono::high_resolution_clock::now();

            for (size_t i = 0; i < chunk; ++i) {
                connfd.recv(buf, MAXTEXT);
                out.write(buf, MAXTEXT);
                show_process_bar((double)(i+1)/(double)chunk);
            }
            cout << endl;
            connfd.recv(buf, rest);
            out.write(buf, rest);

            auto endTime = chrono::high_resolution_clock::now();



            out.close();
            cout << "File download done, Total time: "
                 << chrono::duration_cast<chrono::milliseconds>(endTime - startTime).count()
                 << "s" << endl;
        }
        cout << "ftp>" ;
    }



    return 0;

}
