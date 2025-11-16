#include <string>
#include <winsock.h>
#include <thread>
#include "../common/RtpPacket.h"
class Client {
private:
    std::string serverHost;
    int serverPort;
    int rtpPort;
    SOCKET rtspSocket;
    SOCKET rtpSocket;
    std::thread listenWrite;
    std::thread recvServerReply;
    RtpPacket rtpPacket;

public:
    Client(std::string host, int port, int rtpPort);
    void setup();
    void play();
    void pause();
    void teardown();
    void parseRTSP(std::string msg);
    void receiveRtpPackets();
};
