// H264Packetizer.h
#pragma once
#include "RtpPacketizerBase.h"
#include <vector>
#include <cstdint>
#include <cstring>

class H264Packetizer : public RtpPacketizerBase {
public:
    H264Packetizer() {}

private:
    // helper: generate RTP header
    void writeRtpHeader(uint8_t* b, bool marker) {
        b[0] = 0x80; 
        b[1] = (marker ? 0x80 : 0x00) | (payloadType & 0x7F);

        b[2] = (seqNum >> 8) & 0xFF;
        b[3] = (seqNum) & 0xFF;

        b[4] = (timestamp >> 24) & 0xFF;
        b[5] = (timestamp >> 16) & 0xFF;
        b[6] = (timestamp >> 8) & 0xFF;
        b[7] = (timestamp) & 0xFF;

        b[8] = (ssrc >> 24) & 0xFF;
        b[9] = (ssrc >> 16) & 0xFF;
        b[10] = (ssrc >> 8) & 0xFF;
        b[11] = (ssrc) & 0xFF;
    }

    // Helper: split Annex-B stream into NAL units (00 00 01)
    std::vector<std::pair<const uint8_t*, int>>
    splitAnnexB(const uint8_t* data, int size) {
        std::vector<std::pair<const uint8_t*, int>> nalus;

        int i = 0;
        while (i + 3 < size) {
            if (data[i] == 0x00 && data[i+1] == 0x00 &&
               (data[i+2] == 0x01 || (data[i+2] == 0x00 && data[i+3] == 0x01))) {

                int start = (data[i+2] == 0x01) ? i+3 : i+4;
                int next = start;

                while (next + 3 < size &&
                      !(data[next] == 0x00 && data[next+1] == 0x00 &&
                       (data[next+2] == 0x01 || (data[next+2]==0x00 && data[next+3]==0x01)))) {
                    next++;
                }
                nalus.push_back({data + start, next - start});
                i = next;
            } else {
                i++;
            }
        }

        return nalus;
    }

    // create FU-A fragments for NAL > maxPayload
    void packetizeFUA(std::vector<std::vector<uint8_t>>& out,
                      const uint8_t* nal, int nalSize, bool isLastNalOfFrame) {
        uint8_t nalHeader = nal[0];
        uint8_t nalType = nalHeader & 0x1F;

        const uint8_t* payload = nal + 1;
        int payloadSize = nalSize - 1;

        bool start = true;
        bool end = false;

        while (!end) {
            int chunk = maxPayload - 2;
            if (chunk > payloadSize) {
                chunk = payloadSize;
                end = true;
            }

            std::vector<uint8_t> packet(12 + 2 + chunk);
            writeRtpHeader(packet.data(), end && isLastNalOfFrame);

            packet[12] = (nalHeader & 0xE0) | 28;  
            packet[13] = (start ? 0x80 : 0x00) |
                         (end ? 0x40 : 0x00) |
                         (nalType & 0x1F);

            std::memcpy(packet.data() + 14, payload, chunk);
            out.push_back(packet);

            payload += chunk;
            payloadSize -= chunk;
            seqNum++;
            start = false;
        }
    }

public:

    std::vector<std::vector<uint8_t>>
    packetizeFrame(const uint8_t* frame, int frameSize, bool setMarker) override {
        std::vector<std::vector<uint8_t>> packets;

        auto nalus = splitAnnexB(frame, frameSize);

        for (size_t i = 0; i < nalus.size(); i++) {
            const uint8_t* nal = nalus[i].first;
            int nalSize = nalus[i].second;

            bool isLastNal = (i == nalus.size() - 1);

            if (nalSize <= maxPayload) {
                // Single NALU packet
                std::vector<uint8_t> pkt(12 + nalSize);
                writeRtpHeader(pkt.data(), isLastNal && setMarker);
                std::memcpy(pkt.data() + 12, nal, nalSize);
                packets.push_back(pkt);
                seqNum++;
            } else {
                // FU-A fragmentation
                packetizeFUA(packets, nal, nalSize, isLastNal && setMarker);
            }
        }

        return packets;
    }
};
