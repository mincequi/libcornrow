/*
 *  Copyright (C) 2010-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include <stdint.h>
#include <vector>
#include <string>

#include "AEChannelData.h"

class CHelper_libKODI_audioengine;

class AudioChannelLayout {
    friend class CHelper_libKODI_audioengine;

public:
    AudioChannelLayout();
    explicit AudioChannelLayout(const enum AudioChannel* rhs);
    AudioChannelLayout(const enum AEStdChLayout rhs);
    ~AudioChannelLayout() = default;
    AudioChannelLayout& operator=(const AudioChannelLayout& rhs);
    AudioChannelLayout& operator=(const enum AudioChannel* rhs);
    AudioChannelLayout& operator=(const enum AEStdChLayout rhs);
    bool operator==(const AudioChannelLayout& rhs) const;
    bool operator!=(const AudioChannelLayout& rhs) const;
    AudioChannelLayout& operator+=(AudioChannel rhs);
    AudioChannelLayout& operator-=(AudioChannel rhs);
    AudioChannel operator[](unsigned int i) const;
    operator std::string() const;

    // remove any channels that dont exist in the provided info
    void ResolveChannels(const AudioChannelLayout& rhs);
    void clear();
    inline unsigned int count() const { return m_channelCount; }
    static const char* GetChName(const enum AudioChannel ch);
    bool HasChannel(const enum AudioChannel ch) const;
    bool IsChannelValid(const unsigned int pos);
    bool IsLayoutValid();
    bool ContainsChannels(const AudioChannelLayout& rhs) const;
    void ReplaceChannel(const enum AudioChannel from, const enum AudioChannel to);
    int BestMatch(const std::vector<AudioChannelLayout>& dsts, int* score = NULL) const;
    void AddMissingChannels(const AudioChannelLayout& rhs);

private:
    std::uint8_t    m_channelCount;
    AudioChannel    m_channels[AE_CH_MAX];
};

