#pragma once

#include "Types.h"

namespace coro
{

template <typename InT, typename AccT>
class TBiquad
{
public:
    TBiquad(uint8_t channelCount = 1, uint8_t cascadeCount = 1, uint32_t rate = 44100);

    /**
     * @brief Set number of cascades/passes.
     *
     * A biquad can have multiple cascades/passes to be applied to sample data.
     *
     * @param count
     */
    void setCascadeCount(uint8_t count);

    void setRate(uint32_t rate);

    void setFilter(const Filter& filter);

    void process(InT* const _in, InT* const _out, uint32_t frameCount, uint8_t inSpacing, uint8_t outSpacing);

public:
    bool isValid() const;
    bool update();

    static AccT scaleUp(double);
    static void scaleDown(AccT&);
    static AccT convert(InT& in);

    // The properties of a biquad
    uint8_t m_channelCount = 1;
    uint32_t m_rate = 44100;
    Filter        m_filter;

    struct Coeffs {
        AccT b0 = 0.0, b1 = 0.0, b2 = 0.0;
        AccT a1 = 0.0, a2 = 0.0;
    };
    Coeffs m_coeffs;

    // We have a history for each channel and cascade
    struct History {
        InT x1 = 0.0, x2 = 0.0;
        InT y1 = 0.0, y2 = 0.0;
    };
    std::vector<std::vector<History>> m_history;
};

} // namespace coro

template class coro::TBiquad<float, float>;
template class coro::TBiquad<float, double>;
template class coro::TBiquad<double, double>;
template class coro::TBiquad<int16_t, int32_t>;
template class coro::TBiquad<int16_t, int64_t>;
template class coro::TBiquad<int16_t, float>;
template class coro::TBiquad<int16_t, double>;
template class coro::TBiquad<int32_t, int64_t>;
