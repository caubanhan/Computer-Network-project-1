#ifndef RTP_PACKET_H
#define RTP_PACKET_H

#include <cstdint>
#include <winsock2.h> // Cho htons, htonl

// Định nghĩa RTP Header struct để thao tác nhanh hơn
struct RtpHeader {
    uint8_t version_p_x_cc; // Version, Padding, Extension, CSRC Count
    uint8_t m_pt;           // Marker, Payload Type
    uint16_t sequence_number;
    uint32_t timestamp;
    uint32_t ssrc;
};

class RtpPacket {
private:
    RtpHeader header;       // Header struct thay vì mảng char
    const uint8_t* dataPtr; // Zero-copy: Chỉ trỏ tới dữ liệu, không copy
    size_t dataSize;        // Kích thước frame
    size_t offset;          // Vị trí hiện tại đang đọc

public:
    RtpPacket();
    
    // Hàm này phải rất nhẹ
    void beginFrame(const uint8_t* frameData, int frameSize);
    
    // Trả về false nếu hết dữ liệu
    bool getNextPacket(uint8_t* outBuffer, int& outSize);
};

#endif