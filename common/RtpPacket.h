#ifndef RTP_PACKET_H    // for c++ compilers
#define RTP_PACKET_H

#pragma once            // for c compilers

#include <vector>
#include <cstdint>

constexpr int TIMESTAMP_INCREMENT = 3600; // Example timestamp increment per frame

class RtpPacket {
private:
    char header[12];
    std::vector<unsigned char> payload;
    int seqNum;
    int timestamp;
    int ssrc;
	size_t offset;
    uint32_t clockrate() { return 90000; } // standard RTP clock rate for video
public:
    RtpPacket();
	void beginFrame(const uint8_t* frameData, int frameSize); // Initialize RTP packet with frame data
	bool getNextPacket(uint8_t* outBuffer, int& outSize); // Get next RTP packet
};

#endif