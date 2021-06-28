#pragma once

#include "TileUtils.hpp"
#include "Config.hpp"
#include "jsonUtils.hpp"

#include <optional>
#include <atomic>

namespace HyperTiler {
    struct LogItem {
        virtual json content() const = 0;
    };

    struct TileLoadedItem : public LogItem {
        ivec3 coord;
        std::chrono::system_clock::duration timeTaken;
        inline virtual json content() const override {
            return {
                {"type", json("TileLoadedItem")},
                {"coord", js::Save(coord)},
                {"timeTaken", json(std::chrono::duration_cast<std::chrono::microseconds>(timeTaken).count())}
            };
        }
        TileLoadedItem(ivec3 coord, std::chrono::system_clock::duration timeTaken)
            : coord(coord)
            , timeTaken(timeTaken)
        { }
    };

    struct TileSavedItem : public LogItem {
        ivec3 coord;
        std::chrono::system_clock::duration generationTimeTaken;
        std::chrono::system_clock::duration savingTimeTaken;
        inline virtual json content() const override {
            return {
                {"type", json("TileSavedItem")},
                {"coord", js::Save(coord)},
                {"generationTimeTaken", json(std::chrono::duration_cast<std::chrono::microseconds>(generationTimeTaken).count())},
                {"savingTimeTaken", json(std::chrono::duration_cast<std::chrono::microseconds>(savingTimeTaken).count())}
            };
        }
        TileSavedItem(ivec3 coord, std::chrono::system_clock::duration generationTimeTaken, std::chrono::system_clock::duration savingTimeTaken)
        : coord(coord)
        , generationTimeTaken(generationTimeTaken)
        , savingTimeTaken(savingTimeTaken)
        { }
    };

    struct TileCacheChangeItem : public LogItem {
        ivec3 loadedCoord;
        std::optional<ivec3> evictedCoord;
        inline virtual json content() const override {
            return {
                {"type", json("TileCacheChangeItem")},
                {"loadedCoord", js::Save(loadedCoord)},
                {"evictedCoord", evictedCoord.has_value() ? js::Save(evictedCoord.value()) : json(nullptr)}
            };
        }
    };

    struct Log {
        std::chrono::system_clock::time_point time;
        std::shared_ptr<LogItem> item;

        Log(LogItem* i)
        : time(std::chrono::system_clock::now())
        , item(i)
        { }
    };

    typedef std::function<void(Log)> LogStreamFunc;

    bool Convert(Config const& Conf, LogStreamFunc const& StreamLog, std::atomic_bool& RunningFlag);
}