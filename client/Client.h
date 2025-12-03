#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

using namespace std;

// Cấu trúc gói tin RTP (để parse header)
#pragma pack(push, 1)
struct RtpHeader {
	uint8_t csrc_count : 4; // CSRC (CSRC) count
    uint8_t extension : 1;
    uint8_t padding : 1;
	uint8_t version : 2; // RTP version
    uint8_t payload_type : 7;
	uint8_t marker : 1; // Kết thúc frame
    uint16_t sequence_number;
    uint32_t timestamp;
    uint32_t ssrc;
};
#pragma pack(pop)

class Client {
public:
    // Trạng thái
    enum State { INIT, READY, PLAYING };

    // Constructor & Destructor
    Client(string serverAddr, int serverPort, int rtpPort, string fileName);
    ~Client();

    // Các hàm điều khiển (tương ứng các nút bấm)
    void setup();
    void play();
    void pause();
    void teardown();

    // Hàm chạy chính (thay cho root.mainloop)
    void runInterface();

private:
    // Biến cấu hình
    string serverAddr;
    int serverPort;
    int rtpPort;
    string fileName;

    // Biến trạng thái
    atomic<State> state;
    int rtspSeq;
    int sessionId;
    int frameNum;

    // Socket
    SOCKET rtspSocket;
    SOCKET rtpSocket;
    atomic<bool> isRunning;

    // Thread
    thread rtpThread;

    std::vector<uint8_t> frameBuffer;
    // Các hàm nội bộ
    void connectToServer();
    bool sendRtspRequest(string method);
    void listenRtp(); // Hàm chạy trong thread riêng
    bool handleServerReply();
};