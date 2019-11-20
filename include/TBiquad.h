#pragma once

#include "Types.h"

namespace coro
{

template <typename T>
struct TBiquadCoeffs
{
    T b0 = 0.0, b1 = 0.0, b2 = 0.0;
    T a1 = 0.0, a2 = 0.0;
};

template <typename T>
class TBiquad
{
public:
    TBiquad(std::uint8_t channelCount, std::uint8_t cascadeCount, std::uint32_t rate = 0);

    void setRate(std::uint32_t rate);

    void setFilter(const Filter& filter);

    void process(T* const _in, T* const _out, std::uint32_t frameCount, std::uint8_t inSpacing, std::uint8_t outSpacing);

private:
    bool update();

    // The properties of a biquad
    std::uint32_t m_rate = 44100;
    Filter        m_filter;

    // Coeffs are shared for all channels and cascades
    TBiquadCoeffs<double> m_coeffs;
    TBiquadCoeffs<int32_t> m_iCoeffs;

    // We have a history for each channel and cascade
    struct History {
        T x1 = 0.0, x2 = 0.0;
        T y1 = 0.0, y2 = 0.0;
    };
    std::vector<std::vector<History>> m_history;
};

} // namespace coro

template class coro::TBiquad<float>;
template class coro::TBiquad<double>;
template class coro::TBiquad<int16_t>;
template class coro::TBiquad<int32_t>;
