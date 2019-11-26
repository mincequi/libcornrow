#pragma once

#include "Types.h"

namespace coro
{
namespace audio
{
class Peq;
}

struct BiquadCoeffs
{
    double b0 = 0.0, b1 = 0.0, b2 = 0.0;
    double a1 = 0.0, a2 = 0.0;
};

class Biquad
{
public:
    Biquad(std::uint8_t channelCount, std::uint8_t cascadeCount, std::uint32_t rate = 0);

    void setRate(std::uint32_t rate);
    void setFilter(const Filter& filter);

    void process(float* const in, float* const out, std::uint32_t frameCount, std::uint8_t inSpacing, std::uint8_t outSpacing);

private:
    bool isValid() const;
    bool update();

    // The properties of a biquad
    std::uint32_t m_rate = 44100;
    Filter        m_filter;

    // Coeffs are shared for all channels and cascades
    BiquadCoeffs m_coeffs;

    // We have a history for each channel and cascade
    struct History {
        float x1 = 0.0, x2 = 0.0;
        float y1 = 0.0, y2 = 0.0;
    };
    std::vector<std::vector<History>> m_history;

    friend class audio::Peq;
};

} // namespace coro
