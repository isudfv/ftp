#include <iostream>
#include <fstream>
#include "mysock.h"
using namespace std;

char buf[MAXTEXT];
string cmdline;

int main(int args, char **argv) {

    if (args != 3) {
        cout << "Usage: ./client <ip> <port>\n";
        exit(1);
    }

    mysock connfd(inet_addr(argv[1]), atoi(argv[2]));

    if (connfd.connect() == 0) {
        cerr << "Problem in connecting to the server" << endl;
        exit(3);
    }

    cout << "ftp>";

    while (getline(cin, cmdline)) {
        auto cmds = split(cmdline);

        if (cmds[0] == "ls") {

        }

        if (cmds[0] == "get") {
            fstream out(cmds[2], ios::out | ios::binary);

            size_t chunk, rest;
            connfd.recv((char *)&chunk, MAXTEXT);
            connfd.recv((char *)&rest, MAXTEXT);

            for (size_t i = 0; i < chunk; ++i) {
                connfd.recv(buf, MAXTEXT);
                out.write(buf, MAXTEXT);
            }
            connfd.recv(buf, rest);
            out.write(buf, rest);

            cout << "File download done" << endl;
        }
        cout << "ftp>" ;
    }



    return 0;

}
