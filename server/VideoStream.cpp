#include "VideoStream.h"
#include <stdexcept>
#include <iostream>

VideoStream::VideoStream(std::string fName)
    : fileName(std::move(fName)), frameNbr(0), position(0)
{
    if (!fileName.empty()) {
        openFile(fileName);
    }
}

void VideoStream::openFile(const std::string& filename)
{
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file) {
        throw std::runtime_error("Failed to open video file: " + filename);
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    buffer.resize(size);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        throw std::runtime_error("Failed to read video file: " + filename);
    }

    fileName = filename;
    frameNbr = 0;
    position = 0;
}

// Read next JPEG frame from a MJPEG-style file by scanning for 0xFFD8 ... 0xFFD9
int VideoStream::getNextFrame(uint8_t* frameBuf, int bufSize)
{
    if (!frameBuf || bufSize <= 0) return -1;
    if (position >= buffer.size()) return -1; // End of buffer

    size_t start_pos = -1;

    // Find JPEG start marker 0xFF 0xD8
    for (size_t i = position; i < buffer.size() - 1; ++i) {
        if (buffer[i] == 0xFF && buffer[i + 1] == 0xD8) {
            start_pos = i;
            break;
        }
    }

    if (start_pos == -1) return -1; // No more frames

    // Find JPEG end marker 0xFF 0xD9
    size_t end_pos = -1;
    for (size_t i = start_pos + 2; i < buffer.size() - 1; ++i) {
        if (buffer[i] == 0xFF && buffer[i + 1] == 0xD9) {
            end_pos = i + 2; // Include the end marker
            break;
        }
    }

    if (end_pos == -1) return -1; // End of frame not found

    size_t frame_size = end_pos - start_pos;
    if (frame_size > bufSize) {
        position = end_pos;
        return -1; // Buffer too small
    }

    std::copy(buffer.begin() + start_pos, buffer.begin() + end_pos, frameBuf);
    position = end_pos;
    ++frameNbr;

    return frame_size;
}