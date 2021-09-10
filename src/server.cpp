#include <iostream>
#include <unistd.h>
#include "mysock.h"
#include <vector>
#include <fstream>

#ifdef WINDOWS
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif

using namespace std;
namespace fs = filesystem;

char buf[MAXTEXT];
char currdir[MAXTEXT];

int main(int args, char **argv) {
    if (args != 2) {
        cerr << "Usage: ./server <port>\n";
        exit(1);
    }

    if (atoi(argv[1]) <= 1024) {
        cerr << "Port number should be greater than 1024" << endl;
        exit(2);
    }
    mysock listenfd(INADDR_ANY, atoi(argv[1]));
    std::cout << "Server running...waiting for connections." << std::endl;

    listenfd.start();
    while (true) {
        mysock connfd(listenfd.accept());
        GetCurrentDir(currdir, MAXTEXT - 1);
        cout << "Received request..." << endl;

        if (auto pid = fork(); pid == 0) {
            cout << "Child created for dealing with client requests" << endl;

            while (connfd.recv(buf, MAXTEXT) > 0) {
                cout << "String received from client: " << buf << endl;
                string cmdline(buf);
                vector<string> cmds = split(cmdline);

                if (cmds[0] == "pwd") {
                    connfd.send(currdir, MAXTEXT);
                }

                if (cmds[0] == "ls") {
                    fs::path path(currdir);
                    for (auto& p: fs::directory_iterator(path)) {
                        connfd.send(p.path().filename().c_str(), MAXTEXT);
                    }   connfd.send("STOP_END_OF_FILE", MAXTEXT);
                }

                if (cmds[0] == "get") {
                    fstream in(cmds[1], ios::in | ios::binary);

                    size_t chunk, rest;
                    in.seekg(0, ios::end);
                    chunk = in.tellg() / MAXTEXT;
                    rest  = in.tellg() % MAXTEXT;
                    cout << in.tellg() << endl
                         << chunk << endl
                         << rest << endl;
                    in.seekg(0, ios::beg);

                    connfd.send((char *)&chunk, sizeof(size_t));
                    connfd.send((char *)&rest, sizeof(size_t));

                    for (size_t i = 0; i < chunk; ++i) {
                        in.read(buf, MAXTEXT);
                        connfd.send(buf, MAXTEXT);
                    }
                    in.read(buf, rest);
                    connfd.send(buf, rest);

                    in.close();
                    cout << "File upload done" << endl;
                }
            }

            exit(0);
        }
        connfd.close();
    }
    return 0;

}
