#pragma once

#include "Util.hpp"

namespace HyperTiler {
    string IntToStringPadZeros(int value, int zeros);

    string FormatTileStringInt(string const& format, int value);
    string FormatTileString(string format, ivec3 coord);

    // SLOW!
    bool TileExists(URI const& format, ivec3 coord);

    class ImageSamples {
        vector<uint64_t> m_data;
        vector<int> m_numSamples;
        uint64_t m_totalSamples;
        ivec2 m_dimension;
    public:
        struct SampleException
        : public std::runtime_error
        { SampleException(); };

        template<typename T>
        void GenerateData(T* data, T const& defaultValue) const;

        template<typename T>
        void AddSample(ivec2 const& coord, T const& val);

        uint64_t GetTotalSamples() const;
        void Clear();
        ImageSamples(ivec2 dimension);
    };
}
