#pragma once
#include <fstream>
#include <string>
#include <cstdint>

class VideoStream {
private:
    std::ifstream videoFile;
    std::string fileName;
    int frameNbr = 0;
public:
    VideoStream() = default; // Add default constructor
    VideoStream(std::string fName);
    int getNextFrame(uint8_t* frameBuf, int bufSize);
    void openFile(const std::string& filename);
};
