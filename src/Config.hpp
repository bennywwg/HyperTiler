#pragma once

#include "Util.hpp"

namespace HyperTiler {
    enum class FormatEncoding : int {
        Raw = 0,
        PNG = 1
    };

    struct ImageEncoding {
        int BitDepth = 16;
        double Gamma = 1.0;
        bool SwapEndian = true;
        FormatEncoding Encoding = FormatEncoding::Raw;

        inline bool HasGammEncoding() const { return Gamma != 1.0; }

        operator json() const;
        ImageEncoding() = default;
        ImageEncoding(json const& j);
    };

    struct DatasetConfig {
        URI Format;
        int Channels;
        ivec2 Size;
        ImageEncoding Encoding;

        operator json() const;
        DatasetConfig();
        DatasetConfig(json const& j);
    };

    struct ConversionDatasetConfig {
        int Channels = 1;

        /// <summary>
        /// Format string for input tiles 
        /// </summary>
        URI InputURIFormat;
        ImageEncoding InputEncoding;

        /// <summary>
        /// Format string for output tiles
        /// </summary>
        URI OutputURIFormat;
        ImageEncoding OutputEncoding;

        operator json() const;
        ConversionDatasetConfig();
        ConversionDatasetConfig(json const& j);
    };

    struct ConversionSpatialConfig {
        //FileFormat inputImageFormat;

        //FileFormat          outputImageFormat;

        // range of pixels in the output
        DiscreteAABB2<int>  OutputPixelRange;

        // size of level 0 tiles of the output in pixels
        ivec2               OutputTileSize;

        // size of the input tiles
        ivec2               InputTileSize;

        // output pixels are shifted by this amount
        dvec2               OutputPixelOffset;

        // scale factor for pixels
        dvec2               OutputToInputPixelRatio;

        // lowest level tile to output
        int                 BeginOutputLevel;

        // highest level tile to output
        int                 EndOutputLevel;

        bool                HasNoOffset() const { return OutputPixelOffset.x == 0.0 && OutputPixelOffset.y == 0.0; }
        bvec2               HasUnitRatio() const { return bvec2(OutputToInputPixelRatio.x == 1.0, OutputToInputPixelRatio.y == 1.0); }


        DiscreteAABB2<int>  InputCoordTexels(ivec2 const& Coord) const { return DiscreteAABB2<int>(Coord) * InputTileSize; }
        DiscreteAABB2<int>  OutputCoordTexels(ivec3 const& Coord) const { return DiscreteAABB2<int>(ivec2(Coord)) * (OutputTileSize << Coord.z); }

        operator json() const;
        ConversionSpatialConfig();
        ConversionSpatialConfig(json const& j);
    };

    struct Config {
        ConversionDatasetConfig DatasetConfig;
        ConversionSpatialConfig SpatialConfig;

        operator json() const;
        Config() = default;
        Config(json const& j);
    };

    struct ConversionOptimizationConfig {
        /// <summary>
        /// Store the input dataset on local filesystem
        /// </summary>
        bool cacheOnFilesystem = true;

        /// <summary>
        /// Keep the cache on filesystem after the program executes
        /// </summary>
        bool persistCache = true;

        /// <summary>
        /// Base directory
        /// </summary>
        path cacheBaseDirectory = "./.HyperTilerCache/";

        /// <summary>
        /// How much memory to use to store images during processing
        /// </summary>
        uint64_t availableMemory = 2ull * 1024ull * 1024ull * 1024ull; // default of 2 gigs
    };
}