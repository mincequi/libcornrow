#pragma once

#include <ostream>
#include <vector>

#include <gstreamermm/pad.h>

#include "Types.h"

namespace coro
{

template <typename T>
bool computeBiQuad(int sampleRate, const Filter& filter, T* biquad);
bool computeResponse(const Filter& filter, const std::vector<float>& freqs, std::vector<float>* magnitudes, std::vector<float>* phases);
std::ostream& operator<<(std::ostream&, const Gst::PadDirection&);

} // namespace coro
