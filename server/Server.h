#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include "ServerWorker.h"
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")

class Server {
public:
    Server();
    ~Server();

    bool init(int port);
    bool startListening();
    ServerWorker* acceptClient();

private:
    SOCKET listenSock;
    int port;
};
