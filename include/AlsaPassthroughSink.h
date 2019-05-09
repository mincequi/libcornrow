/*
 *  Copyright (C) 2010-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include <stdint.h>

#include <alsa/asoundlib.h>

#include "AEAudioFormat.h"
#include "AEChannelData.h"
#include "AEChannelInfo.h"
#include "DeviceInfo.h"
#include "AEUtil.h"

#define AE_MIN_PERIODSIZE 256

class AlsaPassthroughSink
{
public:
    AlsaPassthroughSink();
    ~AlsaPassthroughSink();

    static AlsaPassthroughSink* create(std::string& device, AudioFormat& desiredFormat);
    static AlsaPassthroughSink* createPassthrough(std::string& device, AudioFormat& desiredFormat);
    static AudioDeviceInfos enumerateDevices();

    bool init(std::string& device, AudioFormat& format);
    bool initPassthrough(AudioDeviceInfo& device, AudioFormat& format);
    void deinit();

    void Stop();
    double GetCacheTotal();
    unsigned int AddPackets(uint8_t **data, unsigned int frames, unsigned int offset);
    void Drain();

private:
    AudioChannelLayout channelLayoutPassthrough(const AudioFormat& format);
    AudioChannelLayout GetChannelLayoutLegacy(const AudioFormat& format, unsigned int minChannels, unsigned int maxChannels);
    AudioChannelLayout GetChannelLayout(const AudioFormat& format, unsigned int channels);

    static AudioChannel ALSAChannelToAEChannel(unsigned int alsaChannel);
    static unsigned int AEChannelToALSAChannel(AudioChannel aeChannel);
    static AudioChannelLayout ALSAchmapToAEChannelMap(snd_pcm_chmap_t* alsaMap);
    static snd_pcm_chmap_t* AEChannelMapToALSAchmap(const AudioChannelLayout& info);
    static snd_pcm_chmap_t* CopyALSAchmap(snd_pcm_chmap_t* alsaMap);
    static std::string ALSAchmapToString(snd_pcm_chmap_t* alsaMap);
    static AudioChannelLayout GetAlternateLayoutForm(const AudioChannelLayout& info);
    snd_pcm_chmap_t* SelectALSAChannelMap(const AudioChannelLayout& info);

    std::string aesParameters(const AudioFormat& format);
    void HandleError(const char* name, int err);

    std::string m_initDevice;
    AudioFormat m_initFormat;
    AudioFormat m_format;
    unsigned int m_bufferSize = 0;
    double m_formatSampleRateMul = 0.0;
    bool m_passthrough = false;
    std::string m_device;
    snd_pcm_t *m_pcm;
    int m_timeout = 0;
    // support fragmentation, e.g. looping in the sink to get a certain amount of data onto the device
    bool m_fragmented = false;
    unsigned int m_originalPeriodSize = AE_MIN_PERIODSIZE;

    struct ALSAConfig
    {
        unsigned int sampleRate;
        unsigned int periodSize;
        unsigned int frameSize;
        unsigned int channels;
        AudioSampleFormat format;
    };

    static snd_pcm_format_t toAlsa(AudioSampleFormat format);

    bool InitializeHW(const ALSAConfig &inconfig, ALSAConfig &outconfig);
    bool InitializeSW(const ALSAConfig &inconfig);

    static void AppendParams(std::string &device, const std::string &params);
    static bool TryDevice(const std::string &name, snd_pcm_t **pcmp, snd_config_t *lconf);
    static bool TryDeviceWithParams(const std::string &name, const std::string &params, snd_pcm_t **pcmp, snd_config_t *lconf);
    static bool openAudioDevice(const std::string &name, const std::string &params, int channels, snd_pcm_t **pcmp, snd_config_t *lconf);

    static AudioDeviceType AEDeviceTypeFromName(const std::string &name);
    static std::string GetParamFromName(const std::string &name, const std::string &param);
    static void enumerateDevice(AudioDeviceInfos &list, const std::string &device, const std::string &description, snd_config_t *config);
    static bool SoundDeviceExists(const std::string& device);
    static bool GetELD(snd_hctl_t *hctl, int device, AudioDeviceInfo& info, bool& badHDMI);

    static void sndLibErrorHandler(const char *file, int line, const char *function, int err, const char *fmt, ...);
};

