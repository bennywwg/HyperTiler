#pragma once

#include <glm/ext.hpp>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <filesystem>
#include <fstream>
#include <regex>
#include <nlohmann/json.hpp>

namespace HyperTiler {
    using std::string;
    using std::vector;
    using std::pair;
    using std::map;
    using std::set;
    using std::filesystem::path;
    using nlohmann::json;

    using glm::bvec2;
    using glm::bvec3;
    using glm::bvec4;

    using glm::vec2;
    using glm::vec3;
    using glm::vec4;

    using glm::dvec2;
    using glm::dvec3;
    using glm::dvec4;

    using glm::ivec2;
    using glm::ivec3;
    using glm::ivec4;

    using glm::uvec2;
    using glm::uvec3;
    using glm::uvec4;

    using glm::mat3;
    using glm::mat3x4;
    using glm::mat4;

    using glm::dvec2;
    using glm::dvec3;
    using glm::dvec4;

    using glm::quat;

    using glm::dot;

    struct URI : public string {
        bool IsFilesystemResource() const { return !IsNetworkResource(); }
        bool IsNetworkResource() const { return std::regex_search(*this, std::regex(R"(^https?\:\/\/)")); }
        URI() = default;
        URI(string const& other) : string(other) { }
    };

    // iterable int bounds
    template<typename T>
    class DiscreteAABB2 {
    public:
        struct Iterator {
            using iterator_category = std::forward_iterator_tag;
            using difference_type = std::ptrdiff_t;
            using value_type = glm::vec<2, T>;
            using pointer = value_type*;
            using reference = value_type&;

            inline const value_type& operator*() const { return m_val; }

            inline Iterator& operator++() {
                ++m_val.x;
                if (m_val.x >= m_owner->End.x) {
                    m_val.x = m_owner->Begin.x;
                    ++m_val.y;
                }
                return *this;
            }

            inline Iterator operator++(int) { Iterator tmp = *this; ++(*this); return tmp; }

            inline friend bool operator== (const Iterator& a, const Iterator& b) { return a.m_val == b.m_val && a.m_owner == b.m_owner; };
            inline friend bool operator!= (const Iterator& a, const Iterator& b) { return !(a == b); };

            friend class DiscreteAABB2<T>;
        private:
            value_type m_val;
            const DiscreteAABB2<T>* m_owner;
        };

        using value_type = glm::vec<2, T>;

        value_type Begin, End;

        T Area() const;

        bool Empty() const;

        void ClampInto(DiscreteAABB2<T> const& other);

        bool IsCompletelyInside(DiscreteAABB2 const& other) const;

        DiscreteAABB2<T> operator&&(DiscreteAABB2<T> const& other) const;

        // Mathematical operations take place on begin and end

        DiscreteAABB2<T> operator+(glm::vec<2, T> const& shift) const;
        DiscreteAABB2<T> operator-(glm::vec<2, T> const& shift) const;
        DiscreteAABB2<T> operator*(T const& rhs) const;
        DiscreteAABB2<T> operator*(glm::vec<2, T> const& rhs) const;
        DiscreteAABB2<T> operator/(glm::vec<2, T> const& rhs) const;

        Iterator begin() const;
        Iterator end() const;

        DiscreteAABB2();
        explicit DiscreteAABB2(glm::vec<2, T> const& begin);
        DiscreteAABB2(glm::vec<2, T> const& begin, glm::vec<2, T> const& end);
        DiscreteAABB2(T const& x0, T const& y0, T const& x1, T const& y1);
    };

    void htAssert(bool expression);

    // Math
    int CorrectMod(int val, int mod);
    ivec2 CorrectMod(ivec2 val, ivec2 mod);
    int FloorOnInterval(int val, int mod);
    ivec2 FloorOnInterval(ivec2 val, ivec2 mod);
    int CeilOnInterval(int val, int mod);
    ivec2 CeilOnInterval(ivec2 val, ivec2 mod);

    // io
    vector<uint8_t> ReadEntireFileBinary(path const& path);
    vector<uint8_t> ReadEntireUrlBinary(string const& path);
    bool            CheckUrlExistence(string const& path);

    string ReadEntireFileText(path const& path);
    string ReadEntireUrlText(string const& path);

    void ReadEntireFileBinary(path const& path, uint8_t* data, uint64_t size);
    void WriteEntireFileBinary(path const& outPath, uint8_t const* data, uint64_t size);
    void WriteEntireFileBinary(path const& outPath, vector<uint8_t> const& data);
    void WriteEntireFileText(path const& outPath, string const& data);

    bool FileExists(path const& path);
    uint64_t FileSize(path const& path);
    bool RemoveFile(path const& path);

}