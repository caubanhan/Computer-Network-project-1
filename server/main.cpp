#include "Server.h"
#include <thread>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

#define DEFAULT_PORT 554

int main()
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
    server.init(DEFAULT_PORT);
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
