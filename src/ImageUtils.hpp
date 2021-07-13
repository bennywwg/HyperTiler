#pragma once

#include "Util.hpp"

namespace HyperTiler {
    struct ImageData {
        // will be 3, or 4
        int numChannels = -1;

        // will be 8 or 16
        int bitDepth = -1;

        int width = -1;
        int height = -1;

        vector<uint8_t> data;
    };

    ImageData ReadPng(vector<uint8_t> const& data, bool expand);

    bool WritePng(vector<uint8_t>& outputData, uint8_t* inputData, int width, int height, bool swapEndian);

    bool WritePng(vector<uint8_t>& outputData, ImageData const& img);
}