# libcornrow-gstmm

This library offers several supplementary gstreamer elements for the [cornrow project](https://github.com/mincequi/cornrow).

Currently, it is dependent on gstreamermm. This might change in future.

## Current elements
### peq
This is a full parametric equalizer offering low-pass, high-pass, peaking and volume filters.
The following additional filters are planned to come: low-shelf, high-shelf, all-pass.
### crossover
This element acts as a frequency crossover and creates low and high passed signal paths within a multichannel layout.
Also a separate subwoofer channel (aka LFE) is possible.
### alsapassthroughsink
This offers passthrough capabilities for direct (encoded) audio output though SPDIF or HDMI.

## Planned elements
### loudness
