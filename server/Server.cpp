#include "Server.h"


// Need WSACleanup() when init = false
bool Server::init(int port)
{
    int iResult;
    this->port = port;
    std::string poststr = std::to_string(port);
    const char *str = poststr.c_str();

    struct addrinfo *result = NULL;
    struct addrinfo hints;
    ZeroMemory(&hints,sizeof(hints));

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    iResult = getaddrinfo(NULL, str, &hints, &result);
    if (iResult != 0 ) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        return false;
    }

    listenSock = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (listenSock == INVALID_SOCKET)      
    {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        return false;
    }
    
    iResult = bind(listenSock,result->ai_addr,result->ai_addrlen);
    if (iResult == SOCKET_ERROR)
    {
        printf("bind failed with error: %ld\n", WSAGetLastError());
        closesocket(listenSock);
        freeaddrinfo(result);
        return false;
    }

    freeaddrinfo(result);
    return true;
}

// Need WSACleanup() when init = false
bool Server::startListening()
{
    int iResult = INVALID_SOCKET;
    iResult = listen(listenSock, SOMAXCONN);
    if (iResult == SOCKET_ERROR)
    {
        printf("listen failed with error: %ld\n", WSAGetLastError());
        closesocket(listenSock);
        return false;
    }
    return true;
}

ServerWorker* Server::acceptClient() {
    sockaddr_in clientAddr;
    int addrLen = sizeof(clientAddr);
    
    SOCKET clientSock = accept(listenSock, (sockaddr*)&clientAddr, &addrLen);
    if (clientSock == INVALID_SOCKET) {
        std::cerr << "accept() failed with error: " << WSAGetLastError() << std::endl;
        return nullptr;   // DO NOT close listenSock here
    }

    char ipStr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &clientAddr.sin_addr, ipStr, sizeof(ipStr));
    
    std::cout << "Client connected from "
              << ipStr << ":" << ntohs(clientAddr.sin_port) << std::endl;

    return new ServerWorker(clientSock);
}

