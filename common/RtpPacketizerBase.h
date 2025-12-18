#ifndef RTP_PACKETIZER_BASE_H
#define RTP_PACKETIZER_BASE_H

#pragma once
#include <vector>
#include <cstdint>

class RtpPacketizerBase {
protected:
    uint16_t seqNum = 0;
    uint32_t timestamp = 0;
    uint32_t ssrc = 0x11223344;
    uint8_t payloadType = 96;      // H264 thường dùng PT=96
    int maxPayload = 1200;         // thường ~1200 bytes
    uint32_t clockRate = 90000;    // chuẩn video
    int frameRate = 30;            // fps
public:
    virtual ~RtpPacketizerBase() {}

    virtual std::vector<std::vector<uint8_t>>
    packetizeFrame(const uint8_t* frame, int frameSize, bool setMarker) = 0;

    void setSeq(uint16_t s) { seqNum = s; }
    void setTimestamp(uint32_t ts) { timestamp = ts; }
    uint16_t getSeq() const { return seqNum; }

    // Hardcode: tăng timestamp = 3600 mỗi frame
    void advanceTimestamp() { timestamp += 3600; }
};

#endif