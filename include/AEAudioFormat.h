/*
 *  Copyright (C) 2010-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "AEChannelInfo.h"
#include "AEStreamInfo.h"

struct AudioFormat
{
    AudioSampleFormat   sampleFormat    = AudioSampleFormat::Invalid;
    std::uint32_t       sampleRate      = 0;
    AudioChannelLayout  channelLayout;

    /**
   * The number of frames per period
   */
    unsigned int m_frames = 0;

    /**
   * The size of one frame in bytes
   */
    unsigned int m_frameSize = 0;

    /**
   * Stream info of raw passthrough
   */
    StreamInfo streamInfo;

    bool operator==(const AudioFormat& fmt) const
    {
        return  sampleFormat    ==  fmt.sampleFormat    &&
                sampleRate      ==  fmt.sampleRate      &&
                channelLayout   ==  fmt.channelLayout &&
                m_frames        ==  fmt.m_frames        &&
                m_frameSize     ==  fmt.m_frameSize     &&
                streamInfo    ==  fmt.streamInfo;
    }

    AudioFormat& operator=(const AudioFormat& fmt)
    {
        sampleFormat = fmt.sampleFormat;
        sampleRate = fmt.sampleRate;
        channelLayout = fmt.channelLayout;
        m_frames = fmt.m_frames;
        m_frameSize = fmt.m_frameSize;
        streamInfo = fmt.streamInfo;

        return *this;
    }
};
