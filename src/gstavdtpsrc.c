/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2012  Collabora Ltd.
 *
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "gstavdtpsrc.h"

#include <poll.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include <gst/rtp/gstrtppayloads.h>

GST_DEBUG_CATEGORY_STATIC (avdtpsrc_debug);
#define GST_CAT_DEFAULT (avdtpsrc_debug)

enum
{
    PROP_0,
    PROP_FD,
    PROP_BLOCKSIZE,
    PROP_RATE
};

#define parent_class gst_avdtp_src_parent_class
G_DEFINE_TYPE (GstAvdtpSrc2, gst_avdtp_src, GST_TYPE_BASE_SRC);

static GstStaticPadTemplate gst_avdtp_src_template =
    GST_STATIC_PAD_TEMPLATE ("src", GST_PAD_SRC, GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("application/x-rtp, "
        "media = (string) \"audio\","
        "payload = (int) "
        GST_RTP_PAYLOAD_DYNAMIC_STRING ", "
        "clock-rate = (int) { 44100, 48000 }, "
        "encoding-name = (string) \"SBC\"; "));

static void gst_avdtp_src_finalize (GObject * object);
static void gst_avdtp_src_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);
static void gst_avdtp_src_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);

static GstCaps *gst_avdtp_src_getcaps (GstBaseSrc * bsrc, GstCaps * filter);
static gboolean gst_avdtp_src_query (GstBaseSrc * bsrc, GstQuery * query);
static gboolean gst_avdtp_src_start (GstBaseSrc * bsrc);
static gboolean gst_avdtp_src_stop (GstBaseSrc * bsrc);
static GstFlowReturn gst_avdtp_src_create (GstBaseSrc * bsrc, guint64 offset,
    guint length, GstBuffer ** outbuf);
static gboolean gst_avdtp_src_unlock (GstBaseSrc * bsrc);
static gboolean gst_avdtp_src_unlock_stop (GstBaseSrc * bsrc);

static void
gst_avdtp_src_class_init (GstAvdtpSrc2Class * klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GstElementClass *element_class = GST_ELEMENT_CLASS (klass);
  GstBaseSrcClass *basesrc_class = GST_BASE_SRC_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  gobject_class->finalize = GST_DEBUG_FUNCPTR (gst_avdtp_src_finalize);
  gobject_class->set_property = GST_DEBUG_FUNCPTR (gst_avdtp_src_set_property);
  gobject_class->get_property = GST_DEBUG_FUNCPTR (gst_avdtp_src_get_property);

  basesrc_class->start = GST_DEBUG_FUNCPTR (gst_avdtp_src_start);
  basesrc_class->stop = GST_DEBUG_FUNCPTR (gst_avdtp_src_stop);
  basesrc_class->create = GST_DEBUG_FUNCPTR (gst_avdtp_src_create);
  basesrc_class->unlock = GST_DEBUG_FUNCPTR (gst_avdtp_src_unlock);
  basesrc_class->unlock_stop = GST_DEBUG_FUNCPTR (gst_avdtp_src_unlock_stop);
  basesrc_class->get_caps = GST_DEBUG_FUNCPTR (gst_avdtp_src_getcaps);
  basesrc_class->query = GST_DEBUG_FUNCPTR (gst_avdtp_src_query);

  g_object_class_install_property (gobject_class, PROP_FD,
                                   g_param_spec_int ("fd",
                                                     "File descriptor",
                                                     "Acquired file descriptor",
                                                     -1, 1023, -1, G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class, PROP_BLOCKSIZE,
                                   g_param_spec_uint ("blocksize",
                                                     "blocksize",
                                                     "blocksize",
                                                     0, 4096, 0, G_PARAM_READWRITE));

  g_object_class_install_property (gobject_class, PROP_RATE,
                                   g_param_spec_int ("rate",
                                                     "Sample rate",
                                                     "Sampe rate",
                                                     0, 192000, 44100, G_PARAM_READWRITE));

  gst_element_class_set_static_metadata (element_class,
      "Bluetooth AVDTP Source",
      "Source/Audio/Network/RTP",
      "Receives audio from an A2DP device",
      "Arun Raghavan <arun.raghavan@collabora.co.uk>");

  GST_DEBUG_CATEGORY_INIT (avdtpsrc_debug, "avdtpsrc", 0,
      "Bluetooth AVDTP Source");

  gst_element_class_add_static_pad_template (element_class,
      &gst_avdtp_src_template);
}

static void
gst_avdtp_src_init (GstAvdtpSrc2 * avdtpsrc)
{
  avdtpsrc->poll = gst_poll_new (TRUE);

  avdtpsrc->duration = GST_CLOCK_TIME_NONE;
  avdtpsrc->rate = 44100;

  gst_base_src_set_format (GST_BASE_SRC (avdtpsrc), GST_FORMAT_TIME);
  gst_base_src_set_live (GST_BASE_SRC (avdtpsrc), TRUE);
  gst_base_src_set_do_timestamp (GST_BASE_SRC (avdtpsrc), TRUE);
}

static void
gst_avdtp_src_finalize (GObject * object)
{
  GstAvdtpSrc2 *avdtpsrc = GST_AVDTP_SRC (object);

  gst_poll_free (avdtpsrc->poll);
  close(avdtpsrc->pfd.fd);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gst_avdtp_src_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
    GstAvdtpSrc2 *avdtpsrc = GST_AVDTP_SRC (object);

    switch (prop_id) {
    case PROP_FD:
        g_value_set_int (value, avdtpsrc->fd);
        break;
    case PROP_BLOCKSIZE:
        g_value_set_uint (value, avdtpsrc->blockSize);
        break;
    case PROP_RATE:
        g_value_set_int (value, avdtpsrc->rate);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
gst_avdtp_src_set_property (GObject * object, guint prop_id,
                            const GValue * value, GParamSpec * pspec)
{
    GstAvdtpSrc2 *avdtpsrc = GST_AVDTP_SRC (object);

    switch (prop_id) {
    case PROP_FD:
        avdtpsrc->fd = g_value_get_int (value);
        break;
    case PROP_BLOCKSIZE:
        avdtpsrc->blockSize = g_value_get_uint(value);
        break;
    case PROP_RATE:
        avdtpsrc->rate = g_value_get_int (value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static gboolean
gst_avdtp_src_query (GstBaseSrc * bsrc, GstQuery * query)
{
  GstAvdtpSrc2 *avdtpsrc = GST_AVDTP_SRC (bsrc);
  gboolean ret = FALSE;

  switch (GST_QUERY_TYPE (query)) {
    case GST_QUERY_DURATION:{
      GstFormat format;

      if (avdtpsrc->duration != GST_CLOCK_TIME_NONE) {
        gst_query_parse_duration (query, &format, NULL);

        if (format == GST_FORMAT_TIME) {
          gst_query_set_duration (query, format, (gint64) avdtpsrc->duration);
          ret = TRUE;
        }
      }

      break;
    }

    default:
      ret = GST_BASE_SRC_CLASS (parent_class)->query (bsrc, query);
  }

  return ret;
}

static GstCaps *
gst_avdtp_src_getcaps (GstBaseSrc * bsrc, GstCaps * filter)
{
    GstAvdtpSrc2 *avdtpsrc = GST_AVDTP_SRC (bsrc);
    GstCaps *caps = NULL, *ret = NULL;

    caps = gst_caps_new_simple ("application/x-rtp",
                                "media", G_TYPE_STRING, "audio",
                                "payload", GST_TYPE_INT_RANGE, 96, 127,
                                "encoding-name", G_TYPE_STRING, "SBC", NULL);

    gst_caps_set_simple (caps, "clock-rate", G_TYPE_INT, avdtpsrc->rate, NULL);

    if (filter) {
        ret = gst_caps_intersect_full (filter, caps, GST_CAPS_INTERSECT_FIRST);
        gst_caps_unref (caps);
    } else {
        ret = caps;
    }

    return ret;
}

static gboolean
gst_avdtp_src_start (GstBaseSrc * bsrc)
{
  GstAvdtpSrc2 *avdtpsrc = GST_AVDTP_SRC (bsrc);

  GST_DEBUG_OBJECT (avdtpsrc, "Setting block size to link MTU (%d)", avdtpsrc->blockSize);
  gst_base_src_set_blocksize (GST_BASE_SRC (avdtpsrc), avdtpsrc->blockSize);

  gst_poll_fd_init (&avdtpsrc->pfd);
  avdtpsrc->pfd.fd = avdtpsrc->fd;

  gst_poll_add_fd (avdtpsrc->poll, &avdtpsrc->pfd);
  gst_poll_fd_ctl_read (avdtpsrc->poll, &avdtpsrc->pfd, TRUE);
  gst_poll_set_flushing (avdtpsrc->poll, FALSE);

  g_atomic_int_set (&avdtpsrc->unlocked, FALSE);

  return TRUE;
}

static gboolean
gst_avdtp_src_stop (GstBaseSrc * bsrc)
{
  GstAvdtpSrc2 *avdtpsrc = GST_AVDTP_SRC (bsrc);

  gst_poll_remove_fd (avdtpsrc->poll, &avdtpsrc->pfd);
  gst_poll_set_flushing (avdtpsrc->poll, TRUE);

  close(avdtpsrc->pfd.fd);

  return TRUE;
}

static GstFlowReturn
gst_avdtp_src_create (GstBaseSrc * bsrc, guint64 offset, guint length,
    GstBuffer ** outbuf)
{
  GstAvdtpSrc2 *avdtpsrc = GST_AVDTP_SRC (bsrc);
  GstBuffer *buf = NULL;
  GstMapInfo info;
  int ret;

  if (g_atomic_int_get (&avdtpsrc->unlocked))
    return GST_FLOW_FLUSHING;

  /* We don't operate in GST_FORMAT_BYTES, so offset is ignored */

  while ((ret = gst_poll_wait (avdtpsrc->poll, GST_CLOCK_TIME_NONE))) {
    if (g_atomic_int_get (&avdtpsrc->unlocked))
      /* We're unlocked, time to gtfo */
      return GST_FLOW_FLUSHING;

    if (ret < 0)
      /* Something went wrong */
      goto read_error;

    if (ret > 0)
      /* Got some data */
      break;
  }

  ret = GST_BASE_SRC_CLASS (parent_class)->alloc (bsrc, offset, length, outbuf);
  if (G_UNLIKELY (ret != GST_FLOW_OK))
    goto alloc_failed;

  buf = *outbuf;

  gst_buffer_map (buf, &info, GST_MAP_WRITE);

  ret = read (avdtpsrc->pfd.fd, info.data, length);

  if (ret < 0)
    goto read_error;
  else if (ret == 0) {
    GST_INFO_OBJECT (avdtpsrc, "Got EOF on the transport fd");
    goto eof;
  }

  if (ret < length)
    gst_buffer_set_size (buf, ret);

  GST_LOG_OBJECT (avdtpsrc, "Read %d bytes", ret);

  gst_buffer_unmap (buf, &info);
  *outbuf = buf;

  return GST_FLOW_OK;

alloc_failed:
  {
    GST_DEBUG_OBJECT (bsrc, "alloc failed: %s", gst_flow_get_name (ret));
    return ret;
  }

read_error:
  GST_ERROR_OBJECT (avdtpsrc, "Error while reading audio data: %s",
      strerror (errno));
  gst_buffer_unref (buf);
  return GST_FLOW_ERROR;

eof:
  gst_buffer_unref (buf);
  return GST_FLOW_EOS;
}

static gboolean
gst_avdtp_src_unlock (GstBaseSrc * bsrc)
{
  GstAvdtpSrc2 *avdtpsrc = GST_AVDTP_SRC (bsrc);

  g_atomic_int_set (&avdtpsrc->unlocked, TRUE);

  gst_poll_set_flushing (avdtpsrc->poll, TRUE);

  return TRUE;
}

static gboolean
gst_avdtp_src_unlock_stop (GstBaseSrc * bsrc)
{
  GstAvdtpSrc2 *avdtpsrc = GST_AVDTP_SRC (bsrc);

  g_atomic_int_set (&avdtpsrc->unlocked, FALSE);

  gst_poll_set_flushing (avdtpsrc->poll, FALSE);

  return TRUE;
}

gboolean
gst_avdtp_src_plugin_init (GstPlugin * plugin)
{
  return gst_element_register (plugin, "avdtpsrc", GST_RANK_NONE,
      GST_TYPE_AVDTP_SRC2);
}
