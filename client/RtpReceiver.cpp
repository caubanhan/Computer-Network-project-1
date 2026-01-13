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
    currentTimestamp = 3600;
}

RtpReceiver::~RtpReceiver()
{
    if (sock != INVALID_SOCKET)
        closesocket(sock);
    WSACleanup();
}

// parse RTP header (12 bytes)
// RTP sanity security checks and extract payload
bool RtpReceiver::parseRtpPacket(const uint8_t* data, int size, bool& outMarker, uint32_t& outTimestamp,
                                 uint16_t& outSeqNum, const uint8_t*& outPayload, int& outPayloadSize)
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

    // sequence number
    outSeqNum = ((uint16_t)data[2] << 8) | data[3];

    // timestamp
    outTimestamp = (data[4] << 24) | (data[5] << 16) | (data[6] << 8) | data[7];

    // RTP fixed header length = 12 bytes (vì CC = 0)
    outPayload = data + 12;
    outPayloadSize = size - 12;

    return true;
}

// Nhận 1 frame JPEG hoàn chỉnh
/*
    this function just gets 1 RTP packet each call in the while loop,
    but the parent funtion calls it many times to accumulate MJPEG data until a complete frame is formed. 
        std::vector<uint8_t> jpegBuf;
        while ( rtpReceiver->getFrame(jpegBuf) ) {
            // store jpegBuf
        }
*/
bool RtpReceiver::getFrame(std::vector<uint8_t>& outFrame)
{
    if (sock == INVALID_SOCKET) return false;

    sockaddr_in src{};
    int srcLen = sizeof(src);

    int bytes = recvfrom(sock, reinterpret_cast<char*>(recvBuffer), sizeof(recvBuffer), 0,
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
    uint16_t seqNum = 0;
    uint32_t timestamp = 0;
    int payloadSize = 0;

    if (!parseRtpPacket(recvBuffer, bytes, marker, timestamp, seqNum, payload, payloadSize))
        return false;

    std::cout << "Seq Num: " << seqNum << "\n";

    // Check for timestamp change (new frame started)
    if (timestamp != currentTimestamp) {
        // If the buffer has data, it means the PREVIOUS frame is done (but missed marker bit)
        if (!mjpegBuffer.empty()) {
            std::cout << "[RtpReceiver] Timestamp changed. Force-finishing previous frame.\n";
            
            // A. Save the PREVIOUS frame to return it (our frame is already done)
            outFrame = mjpegBuffer;
            
            // B. Clear current frame buffer and start the NEW frame with the first CURRENT packet
            mjpegBuffer.clear();
            if (payloadSize > 0) {
                mjpegBuffer.insert(mjpegBuffer.end(), payload, payload + payloadSize);
            }
            
            // C. Update state
            currentTimestamp = timestamp;
            
            return true;
        }

        // If buffer was empty (e.g., first packet of the stream), just update the timestamp
        currentTimestamp = timestamp;
    }

    // append MJPEG data
    if (payloadSize > 0)
        mjpegBuffer.insert(mjpegBuffer.end(), payload, payload + payloadSize);

    // for robsut debug (average frame size ~ 100KB for 1280x720)
    //                                      ~ 400KB for 1920x1080
    // interpolation check: if timestamp is same but no marker bit and buffer too large, likely packet loss
    if (mjpegBuffer.size() > 400000) { 
        std::cerr << "Warning: Packet loss detected (Missed Marker). Resetting buffer.\n";
        mjpegBuffer.clear();
        return false; 
    }

    // improve robustness: if we receive a packet with marker bit, process it immediately, dont need to wait for timestamp change
    // Frame END: marker bit = 1
    if (marker) {
        outFrame = mjpegBuffer;   // copy out
        mjpegBuffer.clear();      // reset buffer
        return true;
    }

    return false; // frame chưa hoàn chỉnh (loop again)
}