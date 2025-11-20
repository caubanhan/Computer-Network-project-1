
#pragma once
#include <vector>
#include <cstdint>
class RtpPacket {
private:
    char header[12];
    std::vector<unsigned char> payload;
    int seqNum;
    int timestamp;
    int ssrc;
	size_t offset;
public:
    RtpPacket();
	void beginFrame(const uint8_t* frameData, int frameSize); // Initialize RTP packet with frame data
	bool getNextPacket(uint8_t* outBuffer, int& outSize); // Get next RTP packet
};
