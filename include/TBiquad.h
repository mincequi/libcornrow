#pragma once

#include "Types.h"

namespace coro
{

template <typename CoeffT>
struct TBiquadCoeffs
{
    CoeffT b0 = 0.0, b1 = 0.0, b2 = 0.0;
    CoeffT a1 = 0.0, a2 = 0.0;
};

template <typename T>
class TBiquad
{
public:
    TBiquad(std::uint8_t channelCount, std::uint8_t cascadeCount, std::uint32_t rate = 44100);

    void setRate(std::uint32_t rate);

    void setFilter(const Filter& filter);

    void process(T* const _in, T* const _out, std::uint32_t frameCount, std::uint8_t inSpacing, std::uint8_t outSpacing);

    T process(T in);

protected:
    using AccT = typename std::conditional<std::is_floating_point<T>::value, double,
        typename std::conditional<std::is_same<T, int32_t>::value, int64_t,
        int32_t>::type>::type;

    bool update();
    static AccT scaleUp(double);
    static void scaleDown(AccT&);

    // The properties of a biquad
    std::uint32_t m_rate = 44100;
    Filter        m_filter;

    TBiquadCoeffs<AccT> m_coeffs;

    // We have a history for each channel and cascade
    struct History {
        T x1 = 0.0, x2 = 0.0;
        T y1 = 0.0, y2 = 0.0;
    };
    std::vector<std::vector<History>> m_history;

    struct IHistory {
        int64_t x1 = 0.0, x2 = 0.0;
        int64_t y1 = 0.0, y2 = 0.0;
        int64_t stateError = 0.0;
    };
    std::vector<std::vector<IHistory>> m_iHistory;

    History m_pHistory;
};

} // namespace coro

template class coro::TBiquad<float>;
template class coro::TBiquad<double>;
template class coro::TBiquad<int16_t>;
template class coro::TBiquad<int32_t>;
