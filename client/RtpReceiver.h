#pragma once
#include <vector>
#include <cstdint>
#include <winsock2.h>

class RtpReceiver {
public:
    RtpReceiver(int listenPort);
    ~RtpReceiver();

    // Nhận frame hoàn chỉnh (JPEG)
    // return true nếu trả được 1 frame
    bool getFrame(std::vector<uint8_t>& outFrame);

private:
    SOCKET sock = INVALID_SOCKET;
    sockaddr_in addr{};
    uint32_t currentTimestamp;
    uint8_t recvBuffer[65536];

    // buffer để ghép MJPEG
    std::vector<uint8_t> mjpegBuffer;

    // parser RTP header
    bool parseRtpPacket(const uint8_t* data, int size, bool& outMarker, uint32_t& outTimestamp,
                        uint16_t& outSeqNum, const uint8_t*& outPayload, int& outPayloadSize);
};
