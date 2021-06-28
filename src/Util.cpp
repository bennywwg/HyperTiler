#include "Util.hpp"

#include <curl/curl.h>

namespace HyperTiler {
    template class DiscreteAABB2<int>;

    template<typename T>
    T DiscreteAABB2<T>::Area() const {
        if (!(Begin.x <= End.x) ||
            !(Begin.y <= End.y)) {
            return T(0);
        }
        return (End.x - Begin.x) * (End.y - Begin.y);
    }
    template<typename T>
    bool DiscreteAABB2<T>::Empty() const {
        return End.x <= Begin.x || End.y <= Begin.y;
    }

    template<typename T>
    DiscreteAABB2<T> DiscreteAABB2<T>::operator&&(DiscreteAABB2<T> const& other) const {
        DiscreteAABB2<T> res = *this;
        res.ClampInto(other);
        return res;
    }

    template<typename T>
    DiscreteAABB2<T> DiscreteAABB2<T>::operator+(glm::vec<2, T> const& shift) const {
        return DiscreteAABB2<T>(
            Begin + shift,
            End + shift
        );
    }

    template<typename T>
    DiscreteAABB2<T> DiscreteAABB2<T>::operator-(glm::vec<2, T> const& shift) const {
        return *this + (-shift);
    }

    template<typename T>
    DiscreteAABB2<T> DiscreteAABB2<T>::operator*(T const& rhs) const {
        return DiscreteAABB2<T>(
            Begin * rhs,
            End * rhs
        );
    }

    template<typename T>
    DiscreteAABB2<T> DiscreteAABB2<T>::operator*(glm::vec<2, T> const& rhs) const {
        return DiscreteAABB2<T>(
            Begin * rhs,
            End * rhs
        );
    }

    template<typename T>
    DiscreteAABB2<T> DiscreteAABB2<T>::operator/(glm::vec<2, T> const& rhs) const {
        return DiscreteAABB2<T>(
            Begin / rhs,
            End / rhs
        );
    }

    template<typename T>
    void DiscreteAABB2<T>::ClampInto(DiscreteAABB2<T> const& other) {
        if (End.x > other.End.x) End.x = other.End.x;
        if (Begin.x < other.Begin.x) Begin.x = other.Begin.x;
        if (End.y > other.End.y) End.y = other.End.y;
        if (Begin.y < other.Begin.y) Begin.y = other.Begin.y;
    }

    template<typename T>
    bool DiscreteAABB2<T>::IsCompletelyInside(DiscreteAABB2 const& other) const {
        return
            Begin.x >= other.Begin.x &&
            Begin.y >= other.Begin.y &&
            End.x <= other.End.x &&
            End.y <= other.End.y;
    }

    template<typename T>
    typename DiscreteAABB2<T>::Iterator DiscreteAABB2<T>::begin() const {
        Iterator res;
        res.m_owner = this;
        res.m_val = glm::vec<2, T>(Begin.x, Begin.y);
        return res;
    }

    template<typename T>
    typename DiscreteAABB2<T>::Iterator DiscreteAABB2<T>::end() const {
        Iterator res;
        res.m_owner = this;
        res.m_val = glm::vec<2, T>(Begin.x, End.y);
        return res;
    }

    template<typename T>
    DiscreteAABB2<T>::DiscreteAABB2(glm::vec<2, T> const& Begin)
    : DiscreteAABB2<T>(Begin, Begin + ivec2(1))
    { }

    template<typename T>
    DiscreteAABB2<T>::DiscreteAABB2(glm::vec<2, T> const& Begin, glm::vec<2, T> const& End)
    : Begin(Begin)
    , End(End)
    { }

    template<typename T>
    DiscreteAABB2<T>::DiscreteAABB2()
    : DiscreteAABB2(glm::vec<2, T>(0), glm::vec<2, T>(0))
    { }

    template<typename T>
    DiscreteAABB2<T>::DiscreteAABB2(T const& x0, T const& y0, T const& x1, T const& y1)
    : Begin(x0, y0)
    , End(x1, y1)
    { }

    void htAssert(bool expression) {
        if (!expression) {
            int val = 5;
            throw std::runtime_error("assertion failed");
        }
    }

    int CorrectMod(int val, int mod) {
        int modVal = abs(val) % mod;
        if (val < 0 && modVal) return mod - modVal;
        else return modVal;
    }
    ivec2 CorrectMod(ivec2 val, ivec2 mod) {
        return ivec2(CorrectMod(val.x, mod.x), CorrectMod(val.y, mod.y));
    }
    int FloorOnInterval(int val, int mod) {
        return val - CorrectMod(val, mod);
    }
    ivec2 FloorOnInterval(ivec2 val, ivec2 mod) {
        return val - CorrectMod(val, mod);
    }
    int CeilOnInterval(int val, int mod) {
        return FloorOnInterval(val, mod) + mod;
    }
    ivec2 CeilOnInterval(ivec2 val, ivec2 mod) {
        return FloorOnInterval(val, mod) + mod;
    }

    vector<uint8_t> ReadEntireFileBinary(path const& path) {
        std::ifstream f(path, std::ios::binary | std::ios::ate);
        f.exceptions(::std::ios_base::failbit | ::std::ios_base::badbit | ::std::ios_base::eofbit);
        f.seekg(0, std::ios::end);
        size_t size = static_cast<size_t>(f.tellg());
        std::vector<uint8_t> res; res.resize(size);
        f.seekg(0, std::ios::beg);
        f.read((char*)res.data(), res.size());
        return res;
    }
    void ReadEntireFileBinary(path const& path, uint8_t* data, uint64_t size) {
        std::ifstream f(path, std::ios::binary | std::ios::ate);
        f.read((char*)data, size);
    }
    string ReadEntireFileText(path const& path) {
        string res; res.resize(FileSize(path));
        std::ifstream f(path, std::ios::ate);
        f.read((char*)res.data(), res.size());
        return res;
    }
    void WriteEntireFileBinary(path const& outPath, uint8_t const* data, uint64_t size) {
        std::ofstream f(outPath, std::ios::binary | std::ios::trunc);
        f.write(reinterpret_cast<char const*>(data), size);
    }
    void WriteEntireFileBinary(path const& outPath, vector<uint8_t> const& data) {
        WriteEntireFileBinary(outPath, data.data(), data.size());
    }
    void WriteEntireFileText(path const& outPath, string const& data) {
        WriteEntireFileBinary(outPath, reinterpret_cast<uint8_t const*>(data.data()), data.size());
    }

    bool FileExists(path const& path) {
        return std::filesystem::exists(path) && std::filesystem::is_regular_file(path);
    }
    uint64_t FileSize(path const& path) {
        return std::filesystem::file_size(path);
    }
    bool RemoveFile(path const& path) {
        return std::filesystem::remove(path);
    }

    struct CurlMemoryStruct {
        char* memory;
        size_t size;
    };

    static size_t
        WriteMemoryCallback(void* contents, size_t size, size_t nmemb, void* userp)
    {
        size_t realsize = size * nmemb;
        CurlMemoryStruct* mem = reinterpret_cast<CurlMemoryStruct*>(userp);

        char* ptr = reinterpret_cast<char*>(realloc(mem->memory, mem->size + realsize + 1));

        htAssert(ptr);
        if (!ptr) return 0;

        mem->memory = ptr;
        memcpy(&(mem->memory[mem->size]), contents, realsize);
        mem->size += realsize;
        mem->memory[mem->size] = 0;

        return realsize;
    }

    vector<uint8_t> ReadEntireUrlBinary(string const& path) {
        CURL* curl_handle;
        CURLcode res;

        CurlMemoryStruct chunk;

        chunk.memory = reinterpret_cast<char*>(malloc(1));
        chunk.size = 0;

        curl_global_init(CURL_GLOBAL_ALL);

        curl_handle = curl_easy_init();

        curl_easy_setopt(curl_handle, CURLOPT_URL, path.c_str());

        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void*)&chunk);

        curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

        curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, false);

        curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, true);

        res = curl_easy_perform(curl_handle);

        long http_code = 0;
        curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &http_code);

        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));
        }

        if (http_code == 200) {
            vector<uint8_t> mres;
            mres.resize(chunk.size);
            memcpy(mres.data(), chunk.memory, chunk.size);

            curl_easy_cleanup(curl_handle);

            free(chunk.memory);

            return mres;
        }
        else {
            curl_easy_cleanup(curl_handle);

            free(chunk.memory);

            return { };
        }
    }
}