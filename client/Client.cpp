#define STB_IMAGE_IMPLEMENTATION
#include "Client.h"
#include <iostream>
#include <sstream>
#include <fstream>

#pragma comment(lib, "ws2_32.lib")

Client::Client(const std::string& serverAddr_, int rtspPort_, int rtpListenPort_, const std::string& fileName_)
    : serverAddr(serverAddr_), rtspPort(rtspPort_), rtpPort(rtpListenPort_), fileName(fileName_),
      rtspSocket(INVALID_SOCKET), cseq(1), sessionID(""),
      rtpReceiver(nullptr), workerRunning(false),
      latestW(0), latestH(0), frameAvailable(false),
      state(INIT), playSeconds(0)
{
    // Initialize Winsock if not done elsewhere
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
}

Client::~Client() {
    teardown();
    WSACleanup();
}

bool Client::sendRtspRequest(const std::string& method) {
    if (rtspSocket == INVALID_SOCKET) return false;

    std::stringstream ss;
    ss << method << " " << fileName << " RTSP/1.0\r\n";
    ss << "CSeq: " << cseq++ << "\r\n";
    
    // SETUP requires Transport header
    if (method == "SETUP") {
        ss << "Transport: RTP/UDP; client_port=" << rtpPort << "\r\n";
    }
    // Others require Session if we have it
    else if (!sessionID.empty()) {
        ss << "Session: " << sessionID << "\r\n";
    }
    
    ss << "\r\n"; // End of header

    std::string request = ss.str();
    send(rtspSocket, request.c_str(), request.size(), 0);
    std::cout << "[RTSP Request]\n" << request << "\n";

    // Read Response
    char buf[4096] = {0};
    int bytes = recv(rtspSocket, buf, 4096, 0);
    if (bytes > 0) {
        std::string response(buf, bytes);
        std::cout << "[RTSP Response]\n" << response << "\n";

        // Simple parse for Session ID
        if (method == "SETUP") {
            size_t pos = response.find("Session: ");
            if (pos != std::string::npos) {
                sessionID = response.substr(pos + 9);
                // Trim newline
                sessionID = sessionID.substr(0, sessionID.find_first_of("\r\n"));
            }
        }
        return response.find("200 OK") != std::string::npos;
    }
    return false;
}

bool Client::setup() {
    std::ofstream logFile("client_log.txt", std::ios::app);
    if (state.load() != INIT) return false;

    // 1. Create RTSP TCP Socket
    rtspSocket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in server{};
    server.sin_family = AF_INET;
    server.sin_port = htons(rtspPort);
    server.sin_addr.s_addr = inet_addr(serverAddr.c_str());

    if (connect(rtspSocket, (sockaddr*)&server, sizeof(server)) < 0) {
        logFile << "Failed to connect to RTSP Server\n";
        return false;
    }

    // 2. Prepare RTP Receiver (UDP)
    try {
        rtpReceiver = std::make_unique<RtpReceiver>(rtpPort);
    } catch (...) { return false; }

    // 3. Send SETUP
    if (sendRtspRequest("SETUP")) {
        state.store(READY);
        return true;
    }
    logFile.close();
    return false;
}

bool Client::play() {
    if (state.load() != READY) return false;

    if (sendRtspRequest("PLAY")) {
        workerRunning.store(true);
        playSeconds.store(0);
        workerThread = std::thread(&Client::receiveLoop, this);
        state.store(PLAYING);
        return true;
    }
    return false;
}

bool Client::pause() {
    if (state.load() != PLAYING) return false;

    if (sendRtspRequest("PAUSE")) {
        workerRunning.store(false);
        if (workerThread.joinable()) workerThread.join();
        state.store(READY);
        return true;
    }
    return false;
}

bool Client::teardown() {
    sendRtspRequest("TEARDOWN"); // Try to send, even if state is mess

    workerRunning.store(false);
    if (workerThread.joinable()) workerThread.join();
    
    if (rtpReceiver) rtpReceiver.reset();
    
    if (rtspSocket != INVALID_SOCKET) {
        closesocket(rtspSocket);
        rtspSocket = INVALID_SOCKET;
    }

    state.store(INIT);
    cseq = 1;
    sessionID = "";
    return true;
}

// ... receiveLoop and getLatestFrame remain mostly the same ...
// Copy your existing receiveLoop and getLatestFrame here.
// IMPORTANT: In getLatestFrame, use your existing code.
void Client::receiveLoop()
{
    // Just make sure to include the logic I provided originally or your own.
    // The key is calling rtpReceiver->getFrame and MjpegDecoder::decode
    using clock = std::chrono::steady_clock;
    auto lastSecond = clock::now();
    std::vector<uint8_t> jpegBuf;
    int countFrames = 1;
    
    while (workerRunning.load()) {
        jpegBuf.clear();
        if (rtpReceiver->getFrame(jpegBuf)) {
            std::vector<uint8_t> rgb;
            int w=0, h=0;
            if (MjpegDecoder::decode(jpegBuf, rgb, w, h)) {
                std::lock_guard<std::mutex> lk(latestMutex);
                latestRgb.swap(rgb);
                latestW = w;
                latestH = h;
                frameAvailable.store(true);
            }
            std::cout << "Received Frame: " << countFrames++ << "\n";
        }
        // Time keeping
        auto now = clock::now();
        if (now - lastSecond >= std::chrono::seconds(1)) {
            if (state.load() == PLAYING) playSeconds.fetch_add(1);
            lastSecond = now;
        }
        //std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}

bool Client::getLatestFrame(std::vector<uint8_t>& outRgb, int& outW, int& outH)
{
    if (!frameAvailable.load()) return false;
    std::lock_guard<std::mutex> lk(latestMutex);
    if (latestRgb.empty()) return false;
    outRgb = latestRgb;
    outW = latestW;
    outH = latestH;
    frameAvailable.store(false);
    return true;
}