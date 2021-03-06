#include "TileUtils.hpp"
#include "jsonUtils.hpp"
#include <iostream>

#define HT_CHECK_SAMPLE_OVERFLOW

namespace HyperTiler {
    string IntToStringPadZeros(int value, int zeros) {
        string res = std::to_string(value);
        int off = res[0] == '-' ? 1 : 0;
        if (res.size() > zeros) throw std::runtime_error("Int too large to fit in format string");
        while (res.size() < zeros) res.insert(res.begin() + off, '0');
        return res;
    }
    string FormatTileStringInt(string const& format, int value) {
        htAssert(format.size() >= 3);
        string iv = format.substr(2, format.size() - 3);
        if (iv.length() == 0) {
            return std::to_string(value);
        } else {
            return IntToStringPadZeros(value, std::atoi(iv.c_str()));
        }
    }
    string FormatTileString(string orig, ivec3 coord) {
        static const std::regex formats[3] = {
            std::regex("\\{x[0-9]*\\}"),
            std::regex("\\{y[0-9]*\\}"),
            std::regex("\\{z[0-9]*\\}")
        };

        for (int i = 0; i < 3; ++i) {
            std::smatch match;
            if (!std::regex_search(orig, match, formats[i]) || match.size() > 1) throw std::runtime_error("Incorrect format string");

            orig.replace(orig.begin() + match.position(), orig.begin() + match.position() + match.length(), FormatTileStringInt(match.str(), coord[i]));
        }

        return orig;
    }

    bool TileExists(URI const& format, ivec3 coord) {
        string const& ResourceName = FormatTileString(format, coord);
        return format.IsFilesystemResource() ? FileExists(ResourceName) : CheckUrlExistence(ResourceName);
    }
    ImageData LoadTileData(DatasetConfig const& Conf, ivec3 coord) {
            // Get the name of this resource... could be made a lot faster but I doubt this line will be the bottleneck
            const string Name = FormatTileString(Conf.Format, coord);
            
            // Early return if its a file and the specified file doesn't exist
            if (Conf.Format.IsFilesystemResource() && !FileExists(Name)) return ImageData();

            vector<uint8_t> RawData = Conf.Format.IsFilesystemResource() ? ReadEntireFileBinary(Name) : ReadEntireUrlBinary(Name);

            if (RawData.empty()) return ImageData();

            ImageData res;
            if (Conf.Encoding.Encoding == FormatEncoding::PNG) {
                res = ReadPng(RawData, false);
            } else {
                if (Conf.Size.x * Conf.Size.y * (Conf.Encoding.BitDepth / 8) * Conf.Channels != RawData.size()) {
                    std::cout << "Conf does not match actual image\n";
                    return { };
                }
                res.width = Conf.Size.x;
                res.height = Conf.Size.y;
                res.numChannels = Conf.Channels;
                res.bitDepth = Conf.Encoding.BitDepth;
                std::swap(res.data, RawData);
            }

            if (Conf.Encoding.SwapEndian) {
                htAssert(Conf.Encoding.BitDepth == 16);
                for (int i = 0; i < res.data.size(); i += 2)
                    std::swap(res.data[i], res.data[i + 1]);
            }

            return res;
    }


    ImageSamples::SampleException::SampleException()
    : std::runtime_error("Sample overflow, output will be clipped")
    { }

    // Replace cells with no samples with a default value
    template<typename T>
    void ImageSamples::GenerateData(T* data, T const& defaultValue) const {
        for (int i = 0; i < m_data.size(); ++i) {
            if (m_numSamples[i] == 0) {
                data[i] = defaultValue;
            } else {
                double val = static_cast<double>(m_data[i]) / m_numSamples[i];
                data[i] = static_cast<T>(val + 0.5);
            }
        }
    }

    template void ImageSamples::GenerateData<uint8_t>(uint8_t*, uint8_t const&) const;
    template void ImageSamples::GenerateData<uint16_t>(uint16_t*, uint16_t const&) const;
    template void ImageSamples::GenerateData<uint32_t>(uint32_t*, uint32_t const&) const;
    template void ImageSamples::GenerateData<uint64_t>(uint64_t*, uint64_t const&) const;

    template<typename T>
    void ImageSamples::AddSample(ivec2 const& coord, T const& val) {
        ++m_numSamples[coord.y * m_dimension.x + coord.x];
        uint64_t& loc = m_data[coord.y * m_dimension.x + coord.x];
#ifdef HT_CHECK_SAMPLE_OVERFLOW
        if (std::numeric_limits<uint64_t>::max() - val < loc) {
            loc = std::numeric_limits<uint64_t>::max();
            throw SampleException();
        }
#endif
        loc += val;
        ++m_totalSamples;
    }
    
    template void ImageSamples::AddSample<uint8_t>(ivec2 const&, uint8_t const&);
    template void ImageSamples::AddSample<uint16_t>(ivec2 const&, uint16_t const&);
    template void ImageSamples::AddSample<uint32_t>(ivec2 const&, uint32_t const&);
    template void ImageSamples::AddSample<uint64_t>(ivec2 const&, uint64_t const&);

    uint64_t ImageSamples::GetTotalSamples() const {
        return m_totalSamples;
    }

    void ImageSamples::Clear() {
        m_data.clear();
        m_numSamples.clear();
        m_data.resize(m_dimension.x * m_dimension.y, 0);
        m_numSamples.resize(m_dimension.x * m_dimension.y, 0);
        m_totalSamples = 0;
    }

    ImageSamples::ImageSamples(ivec2 dimension) : m_dimension(dimension) {
        Clear();
    }
}
