#pragma once

#include "Types.h"

namespace GstDsp
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
    TBiquad(std::uint8_t channelCount, std::uint8_t cascadeCount, std::uint32_t rate = 0)
        : m_rate(rate),
          m_history(cascadeCount, std::vector<History>(channelCount))
    {
    }

    void setRate(std::uint32_t rate)
    {
        if (m_rate == rate) {
            return;
        }
        m_rate = rate;
        update();
    }

    void setFilter(const Filter& filter)
    {
        if (m_filter == filter) {
            return;
        }
        m_filter = filter;
        update();
    }

    void process(T* const _in, T* const _out, std::uint32_t frameCount, std::uint8_t inSpacing, std::uint8_t outSpacing)
    {
        for (std::size_t i = 0; i < m_history.size(); ++i) {
            // After first run, we have to process result and not again the input data
            T* in = (i == 0) ? _in : _out; T* out = _out;

            // Ch1|Ch2|Ch3|Ch4 || Ch1|Ch2|Ch3|Ch4 || Ch1|Ch2|Ch3|Ch4    -> 4 Channels, 3 frames -> 12 samples
            for (std::uint32_t j = 0; j < frameCount; ++j) {
                for (auto& channel : m_history.at(i)) {
                    T __out;
                    __out = m_coeffs.b0**in + m_coeffs.b1*channel.x1 + m_coeffs.b2*channel.x2 - m_coeffs.a1*channel.y1 - m_coeffs.a2*channel.y2;
                    channel.y2 = channel.y1;
                    channel.y1 = __out;
                    channel.x2 = channel.x1;
                    channel.x1 = *in;

                    *out = __out;

                    ++in;
                    ++out;
                }
                in += inSpacing-m_history.at(i).size();
                out += outSpacing-m_history.at(i).size();
            }
            // After first run, we operate on out instead of in. So, we have to use outSpacing for inSpacing
            inSpacing = outSpacing;
        }
    }

private:
    bool update()
    {
        switch (m_filter.type) {
        case FilterType::Peak: {
            double A = pow(10, m_filter.g/40.0);
            double w0 = 2*M_PI*m_filter.f/m_rate;
            double alpha = sin(w0)*0.5/m_filter.q;

            double alpha1 = alpha*A;
            double alpha2 = alpha/A;
            double a0     = 1.0 + alpha2;

            m_coeffs.b0 = ( 1.0 + alpha1 ) / a0;
            m_coeffs.b1 = (-2.0 * cos(w0)) / a0;
            m_coeffs.b2 = ( 1.0 - alpha1 ) / a0;
            m_coeffs.a1 = m_coeffs.b1;
            m_coeffs.a2 = ( 1.0 - alpha2 ) / a0;
            break;
        }
        case FilterType::LowPass: {
            double w0 = 2*M_PI*m_filter.f/m_rate;
            double alpha = sin(w0)*0.5/m_filter.q;
            double a0    = 1.0 + alpha;

            m_coeffs.b1 = ( 1.0 - cos(w0) ) / a0;
            m_coeffs.b0 = m_coeffs.b1 * 0.5;
            m_coeffs.b2 = m_coeffs.b0;
            m_coeffs.a1 = (-2.0 * cos(w0)) / a0;
            m_coeffs.a2 = ( 1.0 - alpha  ) / a0;
            break;
        }
        case FilterType::HighPass: {
            double w0 = 2*M_PI*m_filter.f/m_rate;
            double alpha = sin(w0)*0.5/m_filter.q;
            double a0    = 1.0 + alpha;

            m_coeffs.b1 = -( 1.0 + cos(w0) ) / a0;
            m_coeffs.b0 = m_coeffs.b1 * -0.5;
            m_coeffs.b2 = m_coeffs.b0;
            m_coeffs.a1 = (-2.0 * cos(w0)) / a0;
            m_coeffs.a2 = ( 1.0 - alpha  ) / a0;
            break;
        }
        case FilterType::Invalid:
            return false;
        }

        return true;
    }

    // The properties of a biquad
    std::uint32_t m_rate = 44100;
    Filter        m_filter;

    // Coeffs are shared for all channels and cascades
    TBiquadCoeffs<T> m_coeffs;

    // We have a history for each channel and cascade
    struct History {
        T x1 = 0.0, x2 = 0.0;
        T y1 = 0.0, y2 = 0.0;
    };
    std::vector<std::vector<History>> m_history;
};

} // namespace GstDsp
