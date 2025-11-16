#include "VideoStream.h"
#include "../common/RtpPacket.h"
#include <sstream>
class ServerWorker {
private:
    SOCKET clientSocket;
    sockaddr_in clientAddr;
    VideoStream videoStream;
    RtpPacket rtpPacket;
public:
    ServerWorker(SOCKET clientsocket);
    ServerWorker(SOCKET clientSock, sockaddr_in addr);
    void processRtspRequest();
    void replyRtsp(const std::string &request);
    void sendRtp();
    void createRtpPacketVideo();
};
