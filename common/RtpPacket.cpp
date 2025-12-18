#include "RtpPacket.h"
#include <cstring>
#include <algorithm> // cho std::min

// Cấu hình
constexpr int MAX_RTP_PAYLOAD = 1400; // Tăng lên gần giới hạn MTU Ethernet
constexpr int TIMESTAMP_INCREMENT = 3600; // 90kHz / 25fps = 3600

RtpPacket::RtpPacket() {
    // Khởi tạo Header một lần duy nhất
    header.version_p_x_cc = 0x80; // Ver=2 (10), P=0, X=0, CC=0 -> 10000000
    header.m_pt = 26;             // Payload Type 26 (JPEG), Marker = 0 ban đầu
    header.sequence_number = 0;
    header.timestamp = 0;
    header.ssrc = htonl(123456);  // SSRC cố định, chuyển sang Network Order ngay
    
    dataPtr = nullptr;
    dataSize = 0;
    offset = 0;
}

void RtpPacket::beginFrame(const uint8_t* frameData, int frameSize) {
    // ZERO-COPY: Chỉ lưu tham chiếu. 
    // LƯU Ý: frameData phải tồn tại cho đến khi gửi xong frame này.
    dataPtr = frameData;
    dataSize = frameSize;
    offset = 0;

    // Tăng timestamp (cần chuyển đổi byte order khi ghi vào gói tin sau này)
    // Ở đây ta giữ giá trị Host order trong biến thành viên để tính toán
    uint32_t currentTs = ntohl(header.timestamp);
    header.timestamp = htonl(currentTs + TIMESTAMP_INCREMENT);
}

bool RtpPacket::getNextPacket(uint8_t* outBuffer, int& outSize) {
    if (offset >= dataSize) {
        return false; 
    }

    // 1. Tính toán kích thước payload cho gói tin này
    int remaining = dataSize - offset;
    int payloadSize = (remaining > MAX_RTP_PAYLOAD) ? MAX_RTP_PAYLOAD : remaining;
    bool isLastPacket = (remaining <= MAX_RTP_PAYLOAD);

    // 2. Xử lý Marker bit (Bit 1 của byte thứ 2)
    // Payload type 26 nằm ở 7 bit thấp, Marker ở bit cao nhất
    uint8_t basePT = 26; 
    if (isLastPacket) {
        header.m_pt = basePT | 0x80; // Set Marker bit = 1
    } else {
        header.m_pt = basePT & 0x7F; // Set Marker bit = 0
    }

    // 3. Copy Header vào buffer (12 bytes)
    // Lưu ý: Sequence number cần tăng và chuyển sang Network Order
    uint16_t currentSeq = ntohs(header.sequence_number);
    header.sequence_number = htons(currentSeq + 1); // Tăng seq num

    // Copy toàn bộ struct header vào buffer (nhanh hơn gán từng byte)
    std::memcpy(outBuffer, &header, 12);

    // 4. Copy Payload (Zero-copy logic phát huy tác dụng ở đây)
    // Đọc trực tiếp từ dataPtr nguồn vào outBuffer mạng
    std::memcpy(outBuffer + 12, dataPtr + offset, payloadSize);

    // 5. Cập nhật kích thước và offset
    outSize = 12 + payloadSize;
    offset += payloadSize;

    return true;
}