#include "RtpReceiver.h"
#include <iostream>

#pragma comment(lib, "ws2_32.lib")

RtpReceiver::RtpReceiver(int listenPort)
{
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Failed to create RTP UDP socket\n";
        return;
    }

    // Set socket to non-blocking mode to avoid freezing the UI
    u_long mode = 1;
    if (ioctlsocket(sock, FIONBIO, &mode) != NO_ERROR) {
        std::cerr << "Failed to set non-blocking mode\n";
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(listenPort);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        std::cerr << "RTP bind failed\n";
        closesocket(sock);
        sock = INVALID_SOCKET;
    }

    mjpegBuffer.reserve(2000000); // ~2 MB buffer
}

RtpReceiver::~RtpReceiver()
{
    if (sock != INVALID_SOCKET)
        closesocket(sock);
    WSACleanup();
}

// parse RTP header (12 bytes)
// RTP sanity security checks and extract payload
bool RtpReceiver::parseRtpPacket(const uint8_t* data, int size, bool& outMarker,
                                 const uint8_t*& outPayload, int& outPayloadSize)
{
    if (size < 12) return false;

    const uint8_t vpxcc = data[0];
    const uint8_t mpt   = data[1];

    uint8_t version = (vpxcc >> 6) & 0x03;
    if (version != 2) return false;

    outMarker = (mpt >> 7) & 1;
    uint8_t payloadType = mpt & 0x7F;

    // Accept both payload type 26 (MJPEG RFC 2435) and 96 (dynamic)
    if (payloadType != (uint8_t)26) return false;

    // RTP fixed header length = 12 bytes (vì CC = 0)
    outPayload = data + 12;
    outPayloadSize = size - 12;

    return true;
}

// Nhận 1 frame JPEG hoàn chỉnh
bool RtpReceiver::getFrame(std::vector<uint8_t>& outFrame)
{
    if (sock == INVALID_SOCKET) return false;

    uint8_t buf[20000];
    sockaddr_in src{};
    int srcLen = sizeof(src);

    int bytes = recvfrom(sock, reinterpret_cast<char*>(buf), sizeof(buf), 0,
                         (sockaddr*)&src, &srcLen);
    if (bytes <= 0) {
        // In non-blocking mode, WSAEWOULDBLOCK is not an error
        int err = WSAGetLastError();
        if (err != WSAEWOULDBLOCK && bytes < 0) {
            std::cerr << "recvfrom error: " << err << "\n";
        }
        return false;
    }

    bool marker = false;
    const uint8_t* payload = nullptr;
    int payloadSize = 0;

    if (!parseRtpPacket(buf, bytes, marker, payload, payloadSize))
        return false;

    // append MJPEG data
    if (payloadSize > 0)
        mjpegBuffer.insert(mjpegBuffer.end(), payload, payload + payloadSize);

    if (mjpegBuffer.size() > 500000) { 
        std::cerr << "Warning: Packet loss detected (Missed Marker). Resetting buffer.\n";
        mjpegBuffer.clear();
        return false; 
    }

    // Frame END: marker bit = 1
    if (marker) {
        outFrame = mjpegBuffer;   // copy out
        mjpegBuffer.clear();      // reset buffer
        return true;
    }

    return false; // frame chưa hoàn chỉnh
}
