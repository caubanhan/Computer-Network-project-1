#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include "ServerWorker.h"
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <memory>
#include <string>
#include <memory>

// optional: to link with Ws2_32.lib (for msvc compiler)
#pragma comment (lib, "Ws2_32.lib")

class Server {
public:
    Server();
    ~Server();

    bool init(int port);
    bool startListening();
    std::unique_ptr<ServerWorker> acceptClient();

private:
    SOCKET listenSock;
    int port;
	bool wsaInitialized = false;
};
