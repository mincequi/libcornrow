/*
 *  Copyright (C) 2010-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */
#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#include "AEUtil.h"

#include <cassert>

extern "C" {
#include <libavutil/channel_layout.h>
}

const char* CAEUtil::GetStdChLayoutName(const enum AEStdChLayout layout)
{
    if (layout < 0 || layout >= AE_CH_LAYOUT_MAX)
        return "UNKNOWN";

    static const char* layouts[AE_CH_LAYOUT_MAX] =
    {
        "1.0",
        "2.0", "2.1", "3.0", "3.1", "4.0",
        "4.1", "5.0", "5.1", "7.0", "7.1"
    };

    return layouts[layout];
}

unsigned int CAEUtil::DataFormatToBits(const enum DataFormat dataFormat)
{
    if (dataFormat < 0 || dataFormat >= AE_FMT_MAX)
        return 0;

    static const unsigned int formats[AE_FMT_MAX] =
    {
        8,                   /* U8     */

        16,                  /* S16BE  */
        16,                  /* S16LE  */
        16,                  /* S16NE  */

        32,                  /* S32BE  */
        32,                  /* S32LE  */
        32,                  /* S32NE  */

        sizeof(double) << 3, /* DOUBLE */
        sizeof(float ) << 3, /* FLOAT  */

        8,                  /* RAW    */
    };

    return formats[dataFormat];
}

const char* CAEUtil::StreamTypeToStr(const enum StreamInfo::StreamType dataType)
{
    switch (dataType) {
    case StreamInfo::StreamType::Null:
        return "NULL";
    case StreamInfo::StreamType::Ac3:
        return "AC3";
    case StreamInfo::StreamType::STREAM_TYPE_DTSHD:
        return "DTSHD";
    case StreamInfo::StreamType::STREAM_TYPE_DTSHD_MA:
        return "DTSHD_MA";
    case StreamInfo::StreamType::STREAM_TYPE_DTSHD_CORE:
        return "DTSHD_CORE";
    case StreamInfo::StreamType::STREAM_TYPE_DTS_1024:
        return "DTS_1024";
    case StreamInfo::StreamType::STREAM_TYPE_DTS_2048:
        return "DTS_2048";
    case StreamInfo::StreamType::Dts512:
        return "DTS_512";
    case StreamInfo::StreamType::STREAM_TYPE_EAC3:
        return "EAC3";
    case StreamInfo::StreamType::STREAM_TYPE_MLP:
        return "MLP";
    case StreamInfo::StreamType::STREAM_TYPE_TRUEHD:
        return "TRUEHD";
    }
}

const char* CAEUtil::DataFormatToStr(const enum DataFormat dataFormat)
{
    if (dataFormat < 0 || dataFormat >= AE_FMT_MAX)
        return "UNKNOWN";

    static const char *formats[AE_FMT_MAX] = {
        "U8",
        "S16BE",
        "S16LE",
        "S16NE",
        "S32BE",
        "S32LE",
        "S32NE",
        "DOUBLE",
        "FLOAT",
        "RAW",
    };

    return formats[dataFormat];
}

inline float CAEUtil::SoftClamp(const float x)
{
#if 1
    /*
       This is a rational function to approximate a tanh-like soft clipper.
       It is based on the pade-approximation of the tanh function with tweaked coefficients.
       See: http://www.musicdsp.org/showone.php?id=238
    */
    if (x < -3.0f)
        return -1.0f;
    else if (x >  3.0f)
        return 1.0f;
    float y = x * x;
    return x * (27.0f + y) / (27.0f + 9.0f * y);
#else
    /* slower method using tanh, but more accurate */

    static const double k = 0.9f;
    /* perform a soft clamp */
    if (x >  k)
        x = (float) (tanh((x - k) / (1 - k)) * (1 - k) + k);
    else if (x < -k)
        x = (float) (tanh((x + k) / (1 - k)) * (1 - k) - k);

    /* hard clamp anything still outside the bounds */
    if (x >  1.0f)
        return  1.0f;
    if (x < -1.0f)
        return -1.0f;

    /* return the final sample */
    return x;
#endif
}

void CAEUtil::ClampArray(float *data, uint32_t count)
{
    for (uint32_t i = 0; i < count; ++i)
        data[i] = SoftClamp(data[i]);
}

bool CAEUtil::S16NeedsByteSwap(DataFormat in, DataFormat out)
{
    const DataFormat nativeFormat =
        #ifdef WORDS_BIGENDIAN
            AE_FMT_S16BE;
#else
            AE_FMT_S16LE;
#endif

    if (in == AE_FMT_S16NE || (in == AE_FMT_RAW))
        in = nativeFormat;
    if (out == AE_FMT_S16NE || (out == AE_FMT_RAW))
        out = nativeFormat;

    return in != out;
}

uint64_t CAEUtil::GetAVChannelLayout(const CAEChannelInfo &info)
{
    uint64_t channelLayout = 0;
    if (info.HasChannel(AE_CH_FL))   channelLayout |= AV_CH_FRONT_LEFT;
    if (info.HasChannel(AE_CH_FR))   channelLayout |= AV_CH_FRONT_RIGHT;
    if (info.HasChannel(AE_CH_FC))   channelLayout |= AV_CH_FRONT_CENTER;
    if (info.HasChannel(AE_CH_LFE))  channelLayout |= AV_CH_LOW_FREQUENCY;
    if (info.HasChannel(AE_CH_BL))   channelLayout |= AV_CH_BACK_LEFT;
    if (info.HasChannel(AE_CH_BR))   channelLayout |= AV_CH_BACK_RIGHT;
    if (info.HasChannel(AE_CH_FLOC)) channelLayout |= AV_CH_FRONT_LEFT_OF_CENTER;
    if (info.HasChannel(AE_CH_FROC)) channelLayout |= AV_CH_FRONT_RIGHT_OF_CENTER;
    if (info.HasChannel(AE_CH_BC))   channelLayout |= AV_CH_BACK_CENTER;
    if (info.HasChannel(AE_CH_SL))   channelLayout |= AV_CH_SIDE_LEFT;
    if (info.HasChannel(AE_CH_SR))   channelLayout |= AV_CH_SIDE_RIGHT;
    if (info.HasChannel(AE_CH_TC))   channelLayout |= AV_CH_TOP_CENTER;
    if (info.HasChannel(AE_CH_TFL))  channelLayout |= AV_CH_TOP_FRONT_LEFT;
    if (info.HasChannel(AE_CH_TFC))  channelLayout |= AV_CH_TOP_FRONT_CENTER;
    if (info.HasChannel(AE_CH_TFR))  channelLayout |= AV_CH_TOP_FRONT_RIGHT;
    if (info.HasChannel(AE_CH_TBL))   channelLayout |= AV_CH_TOP_BACK_LEFT;
    if (info.HasChannel(AE_CH_TBC))   channelLayout |= AV_CH_TOP_BACK_CENTER;
    if (info.HasChannel(AE_CH_TBR))   channelLayout |= AV_CH_TOP_BACK_RIGHT;

    return channelLayout;
}

CAEChannelInfo CAEUtil::GetAEChannelLayout(uint64_t layout)
{
    CAEChannelInfo channelLayout;
    channelLayout.Reset();

    if (layout & AV_CH_FRONT_LEFT)       channelLayout += AE_CH_FL;
    if (layout & AV_CH_FRONT_RIGHT)      channelLayout += AE_CH_FR;
    if (layout & AV_CH_FRONT_CENTER)     channelLayout += AE_CH_FC;
    if (layout & AV_CH_LOW_FREQUENCY)    channelLayout += AE_CH_LFE;
    if (layout & AV_CH_BACK_LEFT)        channelLayout += AE_CH_BL;
    if (layout & AV_CH_BACK_RIGHT)       channelLayout += AE_CH_BR;
    if (layout & AV_CH_FRONT_LEFT_OF_CENTER)  channelLayout += AE_CH_FLOC;
    if (layout & AV_CH_FRONT_RIGHT_OF_CENTER) channelLayout += AE_CH_FROC;
    if (layout & AV_CH_BACK_CENTER)      channelLayout += AE_CH_BC;
    if (layout & AV_CH_SIDE_LEFT)        channelLayout += AE_CH_SL;
    if (layout & AV_CH_SIDE_RIGHT)       channelLayout += AE_CH_SR;
    if (layout & AV_CH_TOP_CENTER)       channelLayout += AE_CH_TC;
    if (layout & AV_CH_TOP_FRONT_LEFT)   channelLayout += AE_CH_TFL;
    if (layout & AV_CH_TOP_FRONT_CENTER) channelLayout += AE_CH_TFC;
    if (layout & AV_CH_TOP_FRONT_RIGHT)  channelLayout += AE_CH_TFR;
    if (layout & AV_CH_TOP_BACK_LEFT)    channelLayout += AE_CH_BL;
    if (layout & AV_CH_TOP_BACK_CENTER)  channelLayout += AE_CH_BC;
    if (layout & AV_CH_TOP_BACK_RIGHT)   channelLayout += AE_CH_BR;

    return channelLayout;
}

AVSampleFormat CAEUtil::GetAVSampleFormat(DataFormat format)
{
    switch (format)
    {
    case DataFormat::AE_FMT_U8:
        return AV_SAMPLE_FMT_U8;
    case DataFormat::AE_FMT_S16NE:
        return AV_SAMPLE_FMT_S16;
    case DataFormat::AE_FMT_S32NE:
        return AV_SAMPLE_FMT_S32;
    case DataFormat::AE_FMT_FLOAT:
        return AV_SAMPLE_FMT_FLT;
    case DataFormat::AE_FMT_DOUBLE:
        return AV_SAMPLE_FMT_DBL;
    case DataFormat::AE_FMT_RAW:
        return AV_SAMPLE_FMT_U8;
    default:
    {
        return AV_SAMPLE_FMT_FLT;
    }
    }
}

uint64_t CAEUtil::GetAVChannel(enum AEChannel aechannel)
{
    switch (aechannel)
    {
    case AE_CH_FL:   return AV_CH_FRONT_LEFT;
    case AE_CH_FR:   return AV_CH_FRONT_RIGHT;
    case AE_CH_FC:   return AV_CH_FRONT_CENTER;
    case AE_CH_LFE:  return AV_CH_LOW_FREQUENCY;
    case AE_CH_BL:   return AV_CH_BACK_LEFT;
    case AE_CH_BR:   return AV_CH_BACK_RIGHT;
    case AE_CH_FLOC: return AV_CH_FRONT_LEFT_OF_CENTER;
    case AE_CH_FROC: return AV_CH_FRONT_RIGHT_OF_CENTER;
    case AE_CH_BC:   return AV_CH_BACK_CENTER;
    case AE_CH_SL:   return AV_CH_SIDE_LEFT;
    case AE_CH_SR:   return AV_CH_SIDE_RIGHT;
    case AE_CH_TC:   return AV_CH_TOP_CENTER;
    case AE_CH_TFL:  return AV_CH_TOP_FRONT_LEFT;
    case AE_CH_TFC:  return AV_CH_TOP_FRONT_CENTER;
    case AE_CH_TFR:  return AV_CH_TOP_FRONT_RIGHT;
    case AE_CH_TBL:  return AV_CH_TOP_BACK_LEFT;
    case AE_CH_TBC:  return AV_CH_TOP_BACK_CENTER;
    case AE_CH_TBR:  return AV_CH_TOP_BACK_RIGHT;
    default:
        return 0;
    }
}
