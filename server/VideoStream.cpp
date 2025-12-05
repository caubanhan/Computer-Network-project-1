#include "VideoStream.h"
#include <stdexcept>
#include <iostream>

VideoStream::VideoStream(std::string fName)
    : videoFile(), fileName(std::move(fName)), frameNbr(0)
{
    if (!fileName.empty()) {
        openFile(fileName);
    }
}

void VideoStream::openFile(const std::string& filename)
{
    if (videoFile.is_open()) videoFile.close();
    videoFile.open(filename, std::ios::binary);
    if (!videoFile) {
        throw std::runtime_error("Failed to open video file: " + filename);
    }
    fileName = filename;
    frameNbr = 0;
}

// Read next JPEG frame from a MJPEG-style file by scanning for 0xFFD8 ... 0xFFD9
int VideoStream::getNextFrame(uint8_t* frameBuf, int bufSize)
{
    if (!frameBuf || bufSize <= 0) return -1;
    if (!videoFile.is_open()) return -1;

    int prev = -1;
    int cur = -1;

    // Find JPEG start marker 0xFF 0xD8
    while ((cur = videoFile.get()) != EOF) {
        if (prev == 0xFF && cur == 0xD8) {
            // start of frame found; write the two start bytes
            int written = 0;
            if (bufSize < 2) return -1;
            frameBuf[written++] = 0xFF;
            frameBuf[written++] = 0xD8;

            int prevByte = 0xD8; // last written byte value
            int byteRead;
            // Read until end marker 0xFF 0xD9 (inclusive)
            while ((byteRead = videoFile.get()) != EOF) {
                if (written >= bufSize) {
                    // buffer too small: skip until end marker, then return error
                    int p = prevByte;
                    int q;
                    while ((q = videoFile.get()) != EOF) {
                        if (p == 0xFF && q == 0xD9) break;
                        p = q;
                    }
                    return -1;
                }
                frameBuf[written++] = static_cast<uint8_t>(byteRead);
                if (prevByte == 0xFF && byteRead == 0xD9) {
                    ++frameNbr;
                    return written; // complete frame size
                }
                prevByte = byteRead;
            }
            return -1; // EOF before end marker
            std::cout << "file is too large\n";
        }
        prev = cur;
    }

    // No more frames
    return -1;
}           