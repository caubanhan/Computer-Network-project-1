#pragma once
#include <fstream>
#include <string>
#include <cstdint>
#include <vector>

class VideoStream {
private:
    std::string fileName;
    int frameNbr = 0;
    std::vector<uint8_t> buffer;
    size_t position = 0;
public:
    VideoStream() = default; // Add default constructor
    VideoStream(std::string fName);
    int getNextFrame(uint8_t* frameBuf, int bufSize);
    void openFile(const std::string& filename);
};
