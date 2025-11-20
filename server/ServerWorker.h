#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include <atomic>
#include <cstdint>
#include <string>
#include <sstream>

#include "VideoStream.h"
#include "../common/RtpPacket.h"
#pragma comment(lib, "ws2_32.lib") 
enum STATE {
    INIT,
	READY,
    PLAYING,
};
class ServerWorker {
private:
    SOCKET clientSocket;
	SOCKET rtpSocket;
    sockaddr_in clientAddr;
    VideoStream videoStream;
    RtpPacket rtpPacket;
	STATE state;

    std::string method;
    std::string fileName;
    std::string uri;

    std::string cseq;
    std::string sessionId;
    int UDPport;

	std::atomic<bool> sending{ false }; // RTP sending flag
    
	uint8_t frameBuf[65536]; // Buffer to hold video frame data
public:
    ServerWorker(SOCKET clientsocket, const sockaddr_in& clientAddr);
    void processRtspRequest();
	void executeRtspRequest();
    void replyRtsp(std::string str);
    void sendRtp();
};
