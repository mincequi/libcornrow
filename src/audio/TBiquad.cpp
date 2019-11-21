#include "TBiquad.h"

#include <assert.h>
#include <loguru/loguru.hpp>

namespace coro
{

template <typename T, typename U>
TBiquad<T,U>::TBiquad(std::uint8_t channelCount, std::uint8_t cascadeCount, std::uint32_t rate)
    : m_rate(rate),
      m_history(cascadeCount, std::vector<History>(channelCount)),
      m_iHistory(cascadeCount, std::vector<IHistory>(channelCount))
{
}

template <typename T, typename U>
void TBiquad<T,U>::setRate(std::uint32_t rate)
{
    if (m_rate == rate) {
        return;
    }
    m_rate = rate;
    update();
}

template <typename T, typename U>
void TBiquad<T,U>::setFilter(const Filter& filter)
{
    if (m_filter == filter) {
        return;
    }
    m_filter = filter;
    update();
}

template <typename T, typename U>
void TBiquad<T,U>::process(T* const _in, T* const _out, std::uint32_t frameCount, std::uint8_t inSpacing, std::uint8_t outSpacing)
{
    // Inner history size reflects channels (and spacing).
    assert(m_history.front().size() == inSpacing);

    // http://www.olliw.eu/2016/digital-filters/
    // https://dsp.stackexchange.com/questions/21792/best-implementation-of-a-real-time-fixed-point-iir-filter-with-constant-coeffic
    // https://web.archive.org/web/20181212024857/http://www.musicdsp.org/files/Audio-EQ-Cookbook.txt
    for (std::size_t i = 0; i < m_history.size(); ++i) {
        // After first run, we have to process result and not again the input data
        T* in = (i == 0) ? _in : _out;
        T* out = _out;

        // Ch1|Ch2|Ch3|Ch4 || Ch1|Ch2|Ch3|Ch4 || Ch1|Ch2|Ch3|Ch4    -> 4 Channels, 3 frames -> 12 samples
        for (std::uint32_t j = 0; j < frameCount; ++j) {
            for (auto& channel : m_history.at(i)) {
                double acc = 0.0;
                acc += m_coeffs.b0**in;
                acc += m_coeffs.b1*channel.x1;
                acc += m_coeffs.b2*channel.x2;
                acc -= m_coeffs.a1*channel.y1;
                acc -= m_coeffs.a2*channel.y2;

                channel.x2 = channel.x1;
                channel.x1 = *in;
                channel.y2 = channel.y1;
                channel.y1 = acc;

                *out = channel.y1;

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

/*
template <> template <typename U>
void TBiquad<int16_t,U>::process(int16_t* const _in, int16_t* const _out, std::uint32_t frameCount, std::uint8_t inSpacing, std::uint8_t outSpacing)
{
    // Inner history size reflects channels (and spacing).
    assert(m_history.front().size() == inSpacing);

    // Outer history are the cascades.
    for (std::size_t i = 0; i < m_iHistory.size(); ++i) {
        // After first run (or first cascade), we process the out data instead of input data.
        int16_t* in = (i == 0) ? _in : _out;
        int16_t* out = _out;

        // Ch1|Ch2|Ch3|Ch4 || Ch1|Ch2|Ch3|Ch4 || Ch1|Ch2|Ch3|Ch4    -> 4 Channels, 3 frames -> 12 samples
        // A frame is a group of samples containing all channels.
        for (std::uint32_t j = 0; j < frameCount; ++j) {
            for (auto& channel : m_iHistory.at(i)) {

                U acc = m_coeffs.b0 * *in
                        + m_coeffs.b1 * channel.x1
                        + m_coeffs.b2 * channel.x2
                        - m_coeffs.a1 * channel.y1
                        - m_coeffs.a2 * channel.y2;
                acc >>= 14;

                channel.y2 = channel.y1;
                channel.y1 = acc;

                channel.x2 = channel.x1;
                channel.x1 = *in;

                *out = acc;

                ++in;
                ++out;
            }
            in += inSpacing-m_iHistory.at(i).size();
            out += outSpacing-m_iHistory.at(i).size();
        }
        // After first run, we operate on out instead of in. So, we have to use outSpacing for inSpacing
        inSpacing = outSpacing;
    }
}
*/

template <typename T, typename U>
bool TBiquad<T,U>::update()
{
    double b0 = 0.0;
    double b1 = 0.0;
    double b2 = 0.0;
    double a0 = 0.0;
    double a1 = 0.0;
    double a2 = 0.0;

    switch (m_filter.type) {
    case FilterType::Peak: {
        double A = pow(10, m_filter.g/40.0);
        double w0 = 2*M_PI*m_filter.f/m_rate;
        double alpha = sin(w0)*0.5/m_filter.q;
        double alpha1 = alpha*A;
        double alpha2 = alpha/A;

        a0 = 1.0 + alpha2;
        b0 = ( 1.0 + alpha1 ) / a0;
        b1 = (-2.0 * cos(w0)) / a0;
        b2 = ( 1.0 - alpha1 ) / a0;
        a1 = b1;
        a2 = ( 1.0 - alpha2 ) / a0;
        break;
    }
    case FilterType::LowPass: {
        double w0 = 2*M_PI*m_filter.f/m_rate;
        double alpha = sin(w0)*0.5/m_filter.q;

        a0 = 1.0 + alpha;
        b1 = ( 1.0 - cos(w0) ) / a0;
        b0 = b1 * 0.5;
        b2 = b0;
        a1 = (-2.0 * cos(w0)) / a0;
        a2 = ( 1.0 - alpha  ) / a0;
        break;
    }
    case FilterType::HighPass: {
        double w0 = 2*M_PI*m_filter.f/m_rate;
        double alpha = sin(w0)*0.5/m_filter.q;

        a0    = 1.0 + alpha;
        b1 = -( 1.0 + cos(w0) ) / a0;
        b0 = b1 * -0.5;
        b2 = b0;
        a1 = (-2.0 * cos(w0)) / a0;
        a2 = ( 1.0 - alpha  ) / a0;
        break;
    }
    case FilterType::LowShelf: {
        double A = pow(10, m_filter.g/40.0);
        double w0 = 2*M_PI*m_filter.f/m_rate;
        double cosW0 = cos(w0);
        double alpha = sin(w0)*0.5/m_filter.q;
        double sqrtAalpha2 = 2.0*sqrt(A)*alpha;

        a0 = (A+1) + (A-1)*cosW0 + sqrtAalpha2;
        b0 =    A*( (A+1) - (A-1)*cosW0 + sqrtAalpha2) / a0;
        b1 =  2*A*( (A-1) - (A+1)*cosW0              ) / a0;
        b2 =    A*( (A+1) - (A-1)*cosW0 - sqrtAalpha2) / a0;
        a1 =   -2*( (A-1) + (A+1)*cosW0              ) / a0;
        a2 =      ( (A+1) + (A-1)*cosW0 - sqrtAalpha2) / a0;
        break;
    }
    case FilterType::HighShelf: {
        double A = pow(10, m_filter.g/40.0);
        double w0 = 2*M_PI*m_filter.f/m_rate;
        double cosW0 = cos(w0);
        double alpha = sin(w0)*0.5/m_filter.q;
        double sqrtAalpha2 = 2.0*sqrt(A)*alpha;

        a0 = (A+1) - (A-1)*cosW0 + sqrtAalpha2;
        b0 =    A*( (A+1) + (A-1)*cosW0 + sqrtAalpha2) / a0;
        b1 = -2*A*( (A-1) + (A+1)*cosW0              ) / a0;
        b2 =    A*( (A+1) + (A-1)*cosW0 - sqrtAalpha2) / a0;
        a1 =    2*( (A-1) - (A+1)*cosW0              ) / a0;
        a2 =      ( (A+1) - (A-1)*cosW0 - sqrtAalpha2) / a0;
        break;
    }
    case FilterType::Invalid:
        return false;
    }

    m_coeffs.b0 = scaleUp(b0);
    m_coeffs.b1 = scaleUp(b1);
    m_coeffs.b2 = scaleUp(b2);
    m_coeffs.a1 = scaleUp(a1);
    m_coeffs.a2 = scaleUp(a2);

    return true;
}

template <typename T, typename U>
T TBiquad<T,U>::process(T in)
{
    U acc = m_coeffs.b0 * in
            + m_coeffs.b1 * m_pHistory.x1
            + m_coeffs.b2 * m_pHistory.x2
            - m_coeffs.a1 * m_pHistory.y1
            - m_coeffs.a2 * m_pHistory.y2;

    scaleDown(acc);

    m_pHistory.y2 = m_pHistory.y1;
    m_pHistory.y1 = acc;

    m_pHistory.x2 = m_pHistory.x1;
    m_pHistory.x1 = in;

    return acc;
}

template <>
int32_t TBiquad<int16_t, int32_t>::scaleUp(double in)
{
    return int32_t(/*0.5*/ + in * (int32_t(1) << 14));
}

template <>
int64_t TBiquad<int32_t, int64_t>::scaleUp(double in)
{
    return int64_t(0.5 + in * (int64_t(1) << 32));
}

template <typename T, typename U>
U TBiquad<T, U>::scaleUp(double in)
{
    return in;
}

template<>
void TBiquad<int32_t, int64_t>::scaleDown(int64_t& in)
{
    in >>= 32;
}

template<>
void TBiquad<int16_t, int32_t>::scaleDown(int32_t& in)
{
    in >>= 14;
}

template<typename T, typename U>
void TBiquad<T,U>::scaleDown(U& in)
{
}


} // namespace coro
