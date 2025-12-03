#pragma once
#include <string>
#include <vector>
#include <mutex>
#include <atomic>
#include <thread>
#include <winsock2.h> // Needed for RTSP TCP Socket

#include "RtpReceiver.h"
#include "MjpegDecoder.h"

class Client {
public:
    enum State { INIT = 0, READY, PLAYING };

    Client(const std::string& serverAddr, int rtspPort, int rtpListenPort, const std::string& fileName);
    ~Client();

    // RTSP Control
    bool setup();
    bool play();
    bool pause();
    bool teardown();

    bool getLatestFrame(std::vector<uint8_t>& outRgb, int& outW, int& outH);
    State getState() const { return state.load(); }
    int getPlaySeconds() const { return playSeconds.load(); }

private:
    void receiveLoop();
    
    // Helper for RTSP
    bool sendRtspRequest(const std::string& method);

private:
    std::string serverAddr;
    int rtspPort;
    int rtpPort;
    std::string fileName;

    // RTSP Connection Variables
    SOCKET rtspSocket;
    int cseq;           // Sequence number (increments per request)
    std::string sessionID; 

    std::unique_ptr<RtpReceiver> rtpReceiver;
    std::thread workerThread;
    std::atomic<bool> workerRunning;

    // Decoding
    std::vector<uint8_t> latestRgb;
    int latestW;
    int latestH;
    std::mutex latestMutex;
    std::atomic<bool> frameAvailable;

    std::atomic<State> state;
    std::atomic<int> playSeconds;
};