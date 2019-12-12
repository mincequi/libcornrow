# libcornrow

This library aims to provide a *very simple* way to construct media graphs for audio/video processing. Currently, its capabilities are very limited and only audio is supported for now.

It is loosely inspired by GStreamer. However I wanted to have a *much simpler* API utilizing modern approaches (C++17) for constructing audio/video filter pipelines. Contributions are welcome. Anyone can add add new codecs and filters by using existing base classes.

Originally, this lib was born out of the [cornrow project](https://github.com/mincequi/cornrow).

## Current elements
### Peq
This is a full parametric equalizer offering low-pass, high-pass, peaking and volume filters.
The following additional filters are planned to come: low-shelf, high-shelf, all-pass.
### Crossover
This element acts as a frequency crossover and creates low and high passed signal paths within a multichannel layout.
Also a separate subwoofer channel (aka LFE) is possible.
### AlsaSink
Outputs audio to ALSA devices including passthrough capabilities for direct (encoded) audio output though SPDIF or HDMI.
### Loudness
This filters offers a similar auditory impression at different volumes, than the recording engineer intended (who usually mixes to a target pressure of 80-85 dB). In typical listening environments we listen to music quieter (40-60 dB).
