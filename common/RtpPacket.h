
class RtpPacket {
private:
    char header[12];
    std::vector<unsigned char> payload;
    int seqNum;
    int timestamp;
    int ssrc;
public:
    RtpPacket();
    void buildPacket(char* data, int size);
    void decodePacket(char* buffer, int size);
    char* getPacketBuffer();
    int getPacketSize();
    unsigned char* getPayload();
    int getPayloadSize();
};
