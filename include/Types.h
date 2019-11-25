#pragma once

#include <cstdint>
#include <list>
#include <string>
#include <vector>
#include <math.h>

namespace coro
{

enum class FilterType : uint8_t
{
    Invalid     = 0,

    // Linear filters
    Peak        = 1,
    LowPass,
    HighPass,
    LowShelf,
    HighShelf,
    //AllPass
};

struct Filter
{
    Filter()
    {}

    Filter(FilterType _t, float _f, float _g, float _q)
        : type(_t),
          f(_f),
          g(_g),
          q(_q)
    {}

    FilterType  type = FilterType::Invalid;
    float       f    = 0.0;
    float       g    = 0.0;
    float       q    = 0.0;

    bool isValid() const
    {
        return (type != FilterType::Invalid && f >= 20.0 && f <= 20000.0 && q != 0.0);
    }

    bool operator==(const Filter& other)
    {
        return type == other.type && f == other.f && g == other.g && q == other.q;
    }
};

template <typename T>
struct StereoFrame
{
    T left;
    T right;
};

template <typename T>
struct StereoLfeFrame : public StereoFrame<T>
{
    T lfe;
};

template <typename T>
struct QuadFrame : public StereoFrame<T>
{
    T rearLeft;
    T rearRight;
};

template <typename T>
struct QuadLfeFrame : public StereoLfeFrame<T>
{
    T rearLeft;
    T rearRight;
};

enum class AudioDeviceType : uint8_t
{
    Invalid,
    Default,
    Spdif,
    Hdmi,
};

struct AudioDeviceInfo
{
    explicit AudioDeviceInfo(const std::string& name, const std::string& desc = std::string());

    std::string name;
    std::string desc;
    AudioDeviceType type;
};
using AudioDeviceInfoList = std::list<AudioDeviceInfo>;

} // namespace coro
