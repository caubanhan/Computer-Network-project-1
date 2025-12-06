#include "Server.h"
#include <thread>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

#define DEFAULT_PORT 8089

int main(int argc, char* argv[])  // Luong Nhan: I updated main() to get arguments from command line
{
    // Initialize Winsock at the start
    WSADATA wsaData;
    int wsaResult = WSAStartup(MAKEWORD(2, 2), &wsaData); 
    if (wsaResult != 0) {
        printf("WSAStartup failed: %d\n", wsaResult);
        return 1;
    }
    printf("WSAStartup called!\n");

    Server server;
    server.init(argv[1] ? atoi(argv[1]) : DEFAULT_PORT); // Use port from command line or default
    server.startListening();

    while (true)
    {
        auto worker = server.acceptClient();
        if (!worker) continue;

        std::thread t([w = std::move(worker)]()
            {
                w->processRtspRequest();
            });
        t.detach();
    }

    // Cleanup Winsock before exiting (optional here since loop never ends)
    WSACleanup();

    return 0;
}
