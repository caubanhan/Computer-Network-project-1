#pragma once
#include "stb_image.h"
#include <vector>

class MjpegDecoder {
public:
    static bool decode(const std::vector<uint8_t>& jpegData,
                       std::vector<uint8_t>& rgbOut,
                       int& width, int& height)
    {
        int channels;
        unsigned char* data = stbi_load_from_memory(
            jpegData.data(),
            jpegData.size(),
            &width,
            &height,
            &channels,
            3 // force RGB
        );

        if (!data) return false;

        rgbOut.assign(data, data + width * height * 3);
        stbi_image_free(data);
        return true;
    }
};
