#pragma once

#include "TileUtils.hpp"

namespace HyperTiler {
    class ImageMemoryAllocator {
        uint64_t m_elementSize;
        vector<uint8_t> m_data;
        vector<int64_t> m_lastUsed;
        uint64_t m_remaining;

        size_t IndexOf(uint8_t const* loc) const;
    public:
        const uint8_t* Begin() const;
        uint64_t ElementSize() const;
        void SetAccessed(uint8_t* loc, int64_t time);
        uint8_t* Alloc(int64_t time, bool& evicted);
        void Free(uint8_t* loc);
        uint64_t SlotsRemaining() const;
        ImageMemoryAllocator(uint32_t elementSize, int maxElements);
    };

    class DatasetCache {
        const path m_cacheBaseDirectory;
        ImageMemoryAllocator m_memoryCache;
        set<pair<string, uint8_t*>> m_inMemory;
        const bool m_persist;
        uint64_t m_currentTime;

        set<pair<string, uint8_t*>>::iterator FindInMemoryByName(string const& name) const;
        set<pair<string, uint8_t*>>::iterator FindInMemoryByPointer(uint8_t*const ptr);

        // stores the image in the filesystem uncompressed if it doesn't already exist
        // destroys existing file if the size of the file does not match the expected size
        void StoreInFilesystem(path const& name, uint8_t const* data) const;

        // returns true if the file is present valid and readable
        // returns false if it isn't
        bool LoadFromFilesystem(path const& name, uint8_t* data) const;

        path PathFromName(string const& name) const;

    public:
        struct CacheResult {
            bool IsValid;
            uint8_t* Data;
        };

        bool IsInCache(string const& name) const;

        CacheResult operator[](string const& name);
        DatasetCache(path cacheBaseDirectory, uint32_t elementSize, int maxElements, bool persist);
        ~DatasetCache();
    private:
        DatasetCache(DatasetCache const& other) = delete;
        DatasetCache& operator=(DatasetCache const& other) = delete;
        DatasetCache(DatasetCache&& other) = delete;
        DatasetCache&& operator=(DatasetCache&& other) = delete;
    };
}