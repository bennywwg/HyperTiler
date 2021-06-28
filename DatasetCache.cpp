#include "DatasetCache.hpp"

#include "ImageUtils.hpp"

namespace HyperTiler {
    size_t ImageMemoryAllocator::IndexOf(uint8_t const* loc) const {
        htAssert(loc);
        size_t diff = loc - m_data.data();
        htAssert(diff % m_elementSize == 0);
        diff /= m_elementSize;
        htAssert(diff < m_data.size());
        return diff;
    }
    uint8_t const* ImageMemoryAllocator::Begin() const {
        return m_data.data();
    }
    uint64_t ImageMemoryAllocator::ElementSize() const {
        return m_elementSize;
    }
    void ImageMemoryAllocator::SetAccessed(uint8_t* loc, int64_t time) {
        htAssert(time >= 0);
        m_lastUsed[IndexOf(loc)] = time;
    }
    uint8_t* ImageMemoryAllocator::Alloc(int64_t time, bool& evicted) {
        htAssert(time >= 0);
        int64_t lruValue = std::numeric_limits<int64_t>::max();
        uint8_t* res = m_data.data();
        size_t index = 0;
        for (size_t i = 0; i < m_lastUsed.size(); ++i) {
            int64_t lastUsed = m_lastUsed[i];
            if (lastUsed < 0) {
                evicted = false;
                --m_remaining;
                m_lastUsed[i] = time;
                return &m_data[i * m_elementSize];
            }
            else if (lastUsed < lruValue) {
                res = &m_data[i * m_elementSize];
                index = i;
            }
        }

        m_lastUsed[index] = time;
        evicted = true;
        return res;
    }
    void ImageMemoryAllocator::Free(uint8_t* loc) {
        //std::lock_guard<std::mutex> locK(m_mut);
        size_t index = IndexOf(loc);
        htAssert(m_lastUsed[index] >= 0);
        m_lastUsed[index] = -1;
        ++m_remaining;
    }
    uint64_t ImageMemoryAllocator::SlotsRemaining() const {
        return m_remaining;
    }
    ImageMemoryAllocator::ImageMemoryAllocator(uint32_t elementSize, int maxElements) {
        m_elementSize = elementSize;
        m_data.resize(m_elementSize * maxElements, 0);
        m_lastUsed.resize(maxElements, -1);
        m_remaining = maxElements;
    }

    set<pair<string, uint8_t*>>::iterator DatasetCache::FindInMemoryByName(string const& name) const {
        for (auto it = m_inMemory.begin(); it != m_inMemory.end(); ++it) {
            if (it->first == name) return it;
        }
        return m_inMemory.end();
    }
    set<pair<string, uint8_t*>>::iterator DatasetCache::FindInMemoryByPointer(uint8_t* const ptr) {
        for (auto it = m_inMemory.begin(); it != m_inMemory.end(); ++it) {
            if (it->second == ptr) return it;
        }
        return m_inMemory.end();
    }
    void DatasetCache::StoreInFilesystem(path const& name, uint8_t const* data) const {
        if (FileExists(name)) {
            if (FileSize(name) == m_memoryCache.ElementSize()) return;
            else RemoveFile(name);
        }
        WriteEntireFileBinary(name, data, m_memoryCache.ElementSize());
    }
    bool DatasetCache::LoadFromFilesystem(path const& name, uint8_t* data) const {
        if (!FileExists(name)) return false;
        if (FileSize(name) != m_memoryCache.ElementSize()) return false;
        ReadEntireFileBinary(name, data, static_cast<uint64_t>(m_memoryCache.ElementSize()));
        return true;
    }
    path DatasetCache::PathFromName(string const& name) const {
        return m_cacheBaseDirectory / name;
    }
    bool DatasetCache::IsInCache(string const& name) const {
        return FindInMemoryByName(name) != m_inMemory.end();
    }
    DatasetCache::CacheResult DatasetCache::operator[](string const& name) {
        const auto it = FindInMemoryByName(name);
        if (it != m_inMemory.end()) {
            m_memoryCache.SetAccessed(it->second, ++m_currentTime);
            return { true, it->second };
        } else {
            path const cachedPath = PathFromName(name);

            bool evicted;
            uint8_t* const res = m_memoryCache.Alloc(++m_currentTime, evicted);

            if (evicted) {
                auto found = FindInMemoryByPointer(res);

                htAssert(found != m_inMemory.end());

                path const evictedPath = PathFromName(found->first);

                StoreInFilesystem(evictedPath, res);

                m_inMemory.erase(found);
            }

            bool present = LoadFromFilesystem(cachedPath, res);

            htAssert(FindInMemoryByName(name) == m_inMemory.end());
            htAssert(FindInMemoryByPointer(res) == m_inMemory.end());

            m_inMemory.insert(pair<string, uint8_t*>(name, res));

            return { present, res };
        }
    }
    DatasetCache::DatasetCache(path cacheBaseDirectory, uint32_t elementSize, int maxElements, bool persist)
    : m_cacheBaseDirectory(cacheBaseDirectory)
    , m_memoryCache(elementSize, maxElements)
    , m_persist(persist)
    , m_currentTime(0)
    {
        if (!persist) {
            std::filesystem::remove_all(m_cacheBaseDirectory);
        }
        std::filesystem::create_directory(m_cacheBaseDirectory);
    }
    DatasetCache::~DatasetCache() {
        if (m_persist) {
            for (auto kvp : m_inMemory) {
                StoreInFilesystem(kvp.first, kvp.second);
            }
        } else {
            std::filesystem::remove_all(m_cacheBaseDirectory);
        }
    }
}