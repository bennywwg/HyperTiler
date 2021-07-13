#include "Config.hpp"
#include "jsonUtils.hpp"

namespace HyperTiler {
    ConversionSpatialConfig::operator json() const {
        js::SaveContex ctx;
        ctx.Store(OutputPixelRange);
        ctx.Store(OutputTileSize);
        ctx.Store(InputTileSize);
        ctx.Store(OutputPixelOffset);
        ctx.Store(OutputToInputPixelRatio);
        ctx.Store(BeginOutputLevel);
        ctx.Store(EndOutputLevel);
        return ctx;
    }
    ConversionSpatialConfig::ConversionSpatialConfig()
        : OutputPixelRange(ivec2(0), ivec2(1))
        , OutputTileSize(512, 512)
        , InputTileSize(-1)
        , OutputPixelOffset(0.0)
        , OutputToInputPixelRatio(1.0)
        , BeginOutputLevel(0)
        , EndOutputLevel(0)
    { }
    ConversionSpatialConfig::ConversionSpatialConfig(json const& j) {
        js::ParseContext ctx = j;
        ctx.Destore(OutputPixelRange);
        ctx.Destore(OutputTileSize);
        ctx.Destore(InputTileSize);
        ctx.Destore(OutputPixelOffset);
        ctx.Destore(OutputToInputPixelRatio);
        ctx.Destore(BeginOutputLevel);
        ctx.Destore(EndOutputLevel);
        if (!ctx.er.empty()) throw ctx.er;
    }
    Config::operator json() const {
        js::SaveContex ctx;
        ctx.Store(DatasetConfig);
        ctx.Store(SpatialConfig);
        return ctx;
    }
    Config::Config(json const& j) {
        js::ParseContext ctx = j;
        ctx.Destore(DatasetConfig);
        ctx.Destore(SpatialConfig);
        if (!ctx.er.empty()) throw ctx.er;
    }
    DatasetConfig::operator json() const {
        js::SaveContex ctx;
        ctx.Store(Channels);
        ctx.Store(Format);
        ctx.Store(Size);
        ctx.Store(Encoding);
        return ctx;
    }
    DatasetConfig::DatasetConfig()
    : Channels(1)
    , Format()
    , Size()
    , Encoding()
    { }
    DatasetConfig::DatasetConfig(json const& j) {
        js::ParseContext ctx = j;
        ctx.Destore(Channels);
        ctx.Destore(Format);
        ctx.Destore(Size);
        ctx.Destore(Encoding);
        if (!ctx.er.empty()) throw ctx.er;
    }
    ConversionDatasetConfig::operator json() const {
        js::SaveContex ctx;
        ctx.Store(InputURIFormat);
        ctx.Store(InputEncoding);
        ctx.Store(OutputURIFormat);
        ctx.Store(OutputEncoding);
        return ctx;
    }
    ConversionDatasetConfig::ConversionDatasetConfig()
        : InputURIFormat("{x6}_{y6}_{z6}.png")
        , OutputURIFormat("output/{x6}_{y6}_{z6}.png")
    {
    }
    ConversionDatasetConfig::ConversionDatasetConfig(json const& j) {
        js::ParseContext ctx = j;
        ctx.Destore(InputURIFormat);
        ctx.Destore(InputEncoding);
        ctx.Destore(OutputURIFormat);
        ctx.Destore(OutputEncoding);
        if (!ctx.er.empty()) throw ctx.er;
    }
    ImageEncoding::operator json() const {
        js::SaveContex ctx;
        ctx.Store(BitDepth);
        ctx.Store(Gamma);
        ctx.Store(SwapEndian);
        ctx.Store(Encoding);
        return ctx;
    }
    ImageEncoding::ImageEncoding(json const& j) {
        js::ParseContext ctx = j;
        ctx.Destore(BitDepth);
        ctx.Destore(Gamma);
        ctx.Destore(SwapEndian);
        ctx.Destore(Encoding);
        if (!ctx.er.empty()) throw ctx.er;
    }
}