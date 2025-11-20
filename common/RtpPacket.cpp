#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")
#include "RtpPacket.h"
#include <cstring>

RtpPacket::RtpPacket()
{
    // RTP header fields (example values)
    header[0] = (2 << 6);   // Version 2, no padding, no extension, 0 CSRC
    header[1] = 96;         // Payload type 96 (dynamic)

    seqNum = 0;
    timestamp = 0;
    ssrc = 123456; // example SSRC

    offset = 0;
}

void RtpPacket::beginFrame(const uint8_t* frameData, int frameSize)
{
    // Reset payload and offset for new frame
    payload.assign(frameData, frameData + frameSize);
    offset = 0;

    // Update timestamp per frame (example: +3600)
    timestamp += 3600;

    // Prepare RTP header
    header[0] = (2 << 6);  // Version 2
    header[1] = 96;        // Payload type 96 (dynamic)
}

bool RtpPacket::getNextPacket(uint8_t* outBuffer, int& outSize)
{
    const int MAX_RTP_PAYLOAD = 1200;   // safe MTU-sized payload

    if (offset >= payload.size())
        return false; // no more packets

    // Compute packet payload size
    int remaining = payload.size() - offset;
    int packetPayload = remaining > MAX_RTP_PAYLOAD ? MAX_RTP_PAYLOAD : remaining;

    // Build RTP header
    uint16_t seq = htons(seqNum++);
    uint32_t ts = htonl(timestamp);
    uint32_t id = htonl(ssrc);

    // Copy header bytes
    std::memcpy(outBuffer, header, 12);

    // Set sequence number
    outBuffer[2] = (seq >> 8) & 0xFF;
    outBuffer[3] = seq & 0xFF;

    // Timestamp
    outBuffer[4] = (ts >> 24) & 0xFF;
    outBuffer[5] = (ts >> 16) & 0xFF;
    outBuffer[6] = (ts >> 8) & 0xFF;
    outBuffer[7] = ts & 0xFF;

    // SSRC
    outBuffer[8] = (id >> 24) & 0xFF;
    outBuffer[9] = (id >> 16) & 0xFF;
    outBuffer[10] = (id >> 8) & 0xFF;
    outBuffer[11] = id & 0xFF;

    // Copy payload chunk
    std::memcpy(outBuffer + 12, payload.data() + offset, packetPayload);

    // Output size includes header + payload
    outSize = 12 + packetPayload;

    // Advance offset
    offset += packetPayload;

    return true;
}
