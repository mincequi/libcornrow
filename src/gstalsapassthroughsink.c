/* GStreamer
 * Copyright (C) 2005 Wim Taymans <wim@fluendo.com>
 * Copyright (C) 2006 Tim-Philipp Müller <tim centricular net>
 *
 * GstAlsaPassthroughSink.c:
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <alsa/asoundlib.h>

#include "gstalsapassthroughsink.h"

#include <gst/audio/gstaudioiec61937.h>
//#include <gst/gst-i18n-plugin.h>
#define _

#ifndef ESTRPIPE
#define ESTRPIPE EPIPE
#endif

#define DEFAULT_DEVICE		"default"
#define DEFAULT_DEVICE_NAME	""
#define DEFAULT_CARD_NAME	""
#define SPDIF_PERIOD_SIZE 1536
#define SPDIF_BUFFER_SIZE 15360

enum
{
  PROP_0,
  PROP_DEVICE,
  PROP_LAST
};

static void gst_alsapassthroughsink_init_interfaces (GType type);
#define gst_alsapassthroughsink_parent_class parent_class
G_DEFINE_TYPE_WITH_CODE (GstAlsaPassthroughSink, gst_alsapassthroughsink,
    GST_TYPE_AUDIO_SINK, gst_alsapassthroughsink_init_interfaces (g_define_type_id));

static void gst_alsapassthroughsink_finalise (GObject * object);
static void gst_alsapassthroughsink_set_property (GObject * object,
    guint prop_id, const GValue * value, GParamSpec * pspec);
static void gst_alsapassthroughsink_get_property (GObject * object, guint prop_id, GValue * value, GParamSpec * pspec);

static gboolean gst_alsapassthroughsink_query (GstBaseSink * bsink, GstQuery * query);

static gboolean gst_alsapassthroughsink_open (GstAudioSink * asink);
static gboolean gst_alsapassthroughsink_prepare (GstAudioSink * asink, GstAudioRingBufferSpec * spec);
static gboolean gst_alsapassthroughsink_unprepare (GstAudioSink * asink);
static gboolean gst_alsapassthroughsink_close (GstAudioSink * asink);
static gint gst_alsapassthroughsink_write (GstAudioSink * asink, gpointer data, guint length);
static guint gst_alsapassthroughsink_delay (GstAudioSink * asink);
static void gst_alsapassthroughsink_reset (GstAudioSink * asink);
static gboolean gst_alsapassthroughsink_acceptcaps (GstAlsaPassthroughSink * alsa, GstCaps * caps);
static GstBuffer* gst_alsapassthroughsink_payload (GstAudioBaseSink * sink, GstBuffer * buf);
static snd_pcm_t* gst_alsapassthroughsink_open_spdif (GstObject * obj, gchar * device, GstAudioRingBufferSpec * spec);

static gint output_ref;         /* 0    */
static snd_output_t *output;    /* NULL */
static GMutex output_mutex;

#define PASSTHROUGH_CAPS \
    "audio/x-ac3, framed = (boolean) true;" \
    "audio/x-eac3, framed = (boolean) true; " \
    "audio/x-dts, framed = (boolean) true, " \
      "block-size = (int) { 512, 1024, 2048 }; " \
    "audio/mpeg, mpegversion = (int) 1, " \
      "mpegaudioversion = (int) [ 1, 3 ], parsed = (boolean) true;"

static GstStaticPadTemplate alsasink_sink_factory =
    GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS (PASSTHROUGH_CAPS)
    );

static void
gst_alsapassthroughsink_finalise (GObject * object)
{
  GstAlsaPassthroughSink *sink = GST_ALSA_PASSTHROUGH_SINK (object);

  g_free (sink->device);
  g_mutex_clear (&sink->alsa_lock);
  g_mutex_clear (&sink->delay_lock);

  g_mutex_lock (&output_mutex);
  --output_ref;
  if (output_ref == 0) {
    snd_output_close (output);
    output = NULL;
  }
  g_mutex_unlock (&output_mutex);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gst_alsapassthroughsink_init_interfaces (GType type)
{
#if 0
  gst_alsa_type_add_device_property_probe_interface (type);
#endif
}

static void
gst_alsapassthroughsink_class_init (GstAlsaPassthroughSinkClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;
  GstBaseSinkClass *gstbasesink_class;
  GstAudioBaseSinkClass *gstbaseaudiosink_class;
  GstAudioSinkClass *gstaudiosink_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;
  gstbasesink_class = (GstBaseSinkClass *) klass;
  gstbaseaudiosink_class = (GstAudioBaseSinkClass *) klass;
  gstaudiosink_class = (GstAudioSinkClass *) klass;

  parent_class = g_type_class_peek_parent (klass);

  gobject_class->finalize = gst_alsapassthroughsink_finalise;
  gobject_class->get_property = gst_alsapassthroughsink_get_property;
  gobject_class->set_property = gst_alsapassthroughsink_set_property;

  gst_element_class_set_static_metadata (gstelement_class,
      "Passthrough audio sink (ALSA)", "Sink/Audio",
      "Output encoded stream to a sound card via ALSA", "Manuel Weichselbaumer <mincequi@web.de>");

  gst_element_class_add_static_pad_template (gstelement_class,
      &alsasink_sink_factory);

  gstbasesink_class->query = GST_DEBUG_FUNCPTR (gst_alsapassthroughsink_query);

  gstbaseaudiosink_class->payload = GST_DEBUG_FUNCPTR (gst_alsapassthroughsink_payload);

  gstaudiosink_class->open = GST_DEBUG_FUNCPTR (gst_alsapassthroughsink_open);
  gstaudiosink_class->prepare = GST_DEBUG_FUNCPTR (gst_alsapassthroughsink_prepare);
  gstaudiosink_class->unprepare = GST_DEBUG_FUNCPTR (gst_alsapassthroughsink_unprepare);
  gstaudiosink_class->close = GST_DEBUG_FUNCPTR (gst_alsapassthroughsink_close);
  gstaudiosink_class->write = GST_DEBUG_FUNCPTR (gst_alsapassthroughsink_write);
  gstaudiosink_class->delay = GST_DEBUG_FUNCPTR (gst_alsapassthroughsink_delay);
  gstaudiosink_class->reset = GST_DEBUG_FUNCPTR (gst_alsapassthroughsink_reset);

  g_object_class_install_property (gobject_class, PROP_DEVICE,
      g_param_spec_string ("device", "Device",
          "ALSA device, as defined in an asound configuration file",
          DEFAULT_DEVICE, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
}

static void
gst_alsapassthroughsink_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstAlsaPassthroughSink *sink = GST_ALSA_PASSTHROUGH_SINK (object);

  switch (prop_id) {
    case PROP_DEVICE:
      g_free (sink->device);
      sink->device = g_value_dup_string (value);
      /* setting NULL restores the default device */
      if (sink->device == NULL) {
        sink->device = g_strdup (DEFAULT_DEVICE);
      }
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_alsapassthroughsink_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  GstAlsaPassthroughSink *sink;

  sink = GST_ALSA_PASSTHROUGH_SINK (object);

  switch (prop_id) {
    case PROP_DEVICE:
      g_value_set_string (value, sink->device);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_alsapassthroughsink_init (GstAlsaPassthroughSink * alsasink)
{
  GST_DEBUG_OBJECT (alsasink, "initializing alsasink");

  alsasink->device = g_strdup (DEFAULT_DEVICE);
  alsasink->handle = NULL;
  g_mutex_init (&alsasink->alsa_lock);
  g_mutex_init (&alsasink->delay_lock);

  g_mutex_lock (&output_mutex);
  if (output_ref == 0) {
    snd_output_stdio_attach (&output, stdout, 0);
    ++output_ref;
  }
  g_mutex_unlock (&output_mutex);
}

#define CHECK(call, error) \
G_STMT_START {             \
  if ((err = call) < 0) {  \
    GST_WARNING_OBJECT (self, "Error %d (%s) calling " #call, err, snd_strerror (err)); \
    goto error;            \
  }                        \
} G_STMT_END;

static gboolean
gst_alsapassthroughsink_acceptcaps (GstAlsaPassthroughSink * alsa, GstCaps * caps)
{
  GstPad *pad = GST_BASE_SINK (alsa)->sinkpad;
  GstCaps *pad_caps;
  GstStructure *st;
  gboolean ret = FALSE;
  GstAudioRingBufferSpec spec = { 0 };

  pad_caps = gst_pad_query_caps (pad, caps);
  if (!pad_caps || gst_caps_is_empty (pad_caps)) {
    if (pad_caps)
      gst_caps_unref (pad_caps);
    ret = FALSE;
    goto done;
  }
  gst_caps_unref (pad_caps);

  /* If we've not got fixed caps, creating a stream might fail, so let's just
   * return from here with default acceptcaps behaviour */
  if (!gst_caps_is_fixed (caps))
    goto done;

  /* parse helper expects this set, so avoid nasty warning
   * will be set properly later on anyway  */
  spec.latency_time = GST_SECOND;
  if (!gst_audio_ring_buffer_parse_caps (&spec, caps))
    goto done;

  /* Make sure input is framed (one frame per buffer) and can be payloaded */
  switch (spec.type) {
    case GST_AUDIO_RING_BUFFER_FORMAT_TYPE_AC3:
    case GST_AUDIO_RING_BUFFER_FORMAT_TYPE_EAC3:
    case GST_AUDIO_RING_BUFFER_FORMAT_TYPE_DTS:
    case GST_AUDIO_RING_BUFFER_FORMAT_TYPE_MPEG:
    {
      gboolean framed = FALSE, parsed = FALSE;
      st = gst_caps_get_structure (caps, 0);

      gst_structure_get_boolean (st, "framed", &framed);
      gst_structure_get_boolean (st, "parsed", &parsed);
      if ((!framed && !parsed) || gst_audio_iec61937_frame_size (&spec) <= 0)
        goto done;
    }
    default:{
    }
  }
  ret = TRUE;

done:
  gst_caps_replace (&spec.caps, NULL);
  return ret;
}

static gboolean
gst_alsapassthroughsink_query (GstBaseSink * sink, GstQuery * query)
{
  GstAlsaPassthroughSink *alsa = GST_ALSA_PASSTHROUGH_SINK (sink);
  gboolean ret;

  switch (GST_QUERY_TYPE (query)) {
    case GST_QUERY_ACCEPT_CAPS:
    {
      GstCaps *caps;

      gst_query_parse_accept_caps (query, &caps);
      ret = gst_alsapassthroughsink_acceptcaps (alsa, caps);
      gst_query_set_accept_caps_result (query, ret);
      ret = TRUE;
      break;
    }
    default:
      ret = GST_BASE_SINK_CLASS (parent_class)->query (sink, query);
      break;
  }
  return ret;
}

static int
set_hwparams (GstAlsaPassthroughSink * self)
{
  guint rrate;
  gint err;
  snd_pcm_hw_params_t *params;
  guint period_time, buffer_time;

  snd_pcm_hw_params_malloc (&params);

  GST_DEBUG_OBJECT (self, "Negotiating to %d channels @ %d Hz (format = %s) "
      "SPDIF (%d)", self->channels, self->rate,
      snd_pcm_format_name (self->format), self->passthrough);

  /* start with requested values, if we cannot configure alsa for those values,
   * we set these values to -1, which will leave the default alsa values */
  buffer_time = self->buffer_time;
  period_time = self->period_time;

retry:
  /* choose all parameters */
  CHECK (snd_pcm_hw_params_any (self->handle, params), no_config);
  /* set the interleaved read/write format */
  CHECK (snd_pcm_hw_params_set_access (self->handle, params, self->access),
      wrong_access);
  /* set the sample format */
  if (self->passthrough) {
    /* Try to use big endian first else fallback to le and swap bytes */
    if (snd_pcm_hw_params_set_format (self->handle, params, self->format) < 0) {
      self->format = SND_PCM_FORMAT_S16_LE;
      self->need_swap = TRUE;
      GST_DEBUG_OBJECT (self, "falling back to little endian with swapping");
    } else {
      self->need_swap = FALSE;
    }
  }
  CHECK (snd_pcm_hw_params_set_format (self->handle, params, self->format),
      no_sample_format);
  /* set the count of channels */
  CHECK (snd_pcm_hw_params_set_channels (self->handle, params, self->channels),
      no_channels);
  /* set the stream rate */
  rrate = self->rate;
  CHECK (snd_pcm_hw_params_set_rate_near (self->handle, params, &rrate, NULL),
      no_rate);

#ifndef GST_DISABLE_GST_DEBUG
  /* get and dump some limits */
  {
    guint min, max;

    snd_pcm_hw_params_get_buffer_time_min (params, &min, NULL);
    snd_pcm_hw_params_get_buffer_time_max (params, &max, NULL);

    GST_DEBUG_OBJECT (self, "buffer time %u, min %u, max %u",
        self->buffer_time, min, max);

    snd_pcm_hw_params_get_period_time_min (params, &min, NULL);
    snd_pcm_hw_params_get_period_time_max (params, &max, NULL);

    GST_DEBUG_OBJECT (self, "period time %u, min %u, max %u",
        self->period_time, min, max);

    snd_pcm_hw_params_get_periods_min (params, &min, NULL);
    snd_pcm_hw_params_get_periods_max (params, &max, NULL);

    GST_DEBUG_OBJECT (self, "periods min %u, max %u", min, max);
  }
#endif

  /* now try to configure the buffer time and period time, if one
   * of those fail, we fall back to the defaults and emit a warning. */
  if (buffer_time != -1 && !self->passthrough) {
    /* set the buffer time */
    if ((err = snd_pcm_hw_params_set_buffer_time_near (self->handle, params,
                &buffer_time, NULL)) < 0) {
      GST_ELEMENT_WARNING (self, RESOURCE, SETTINGS, (NULL),
          ("Unable to set buffer time %i for playback: %s",
              buffer_time, snd_strerror (err)));
      /* disable buffer_time the next round */
      buffer_time = -1;
      goto retry;
    }
    GST_DEBUG_OBJECT (self, "buffer time %u", buffer_time);
    self->buffer_time = buffer_time;
  }
  if (period_time != -1 && !self->passthrough) {
    /* set the period time */
    if ((err = snd_pcm_hw_params_set_period_time_near (self->handle, params,
                &period_time, NULL)) < 0) {
      GST_ELEMENT_WARNING (self, RESOURCE, SETTINGS, (NULL),
          ("Unable to set period time %i for playback: %s",
              period_time, snd_strerror (err)));
      /* disable period_time the next round */
      period_time = -1;
      goto retry;
    }
    GST_DEBUG_OBJECT (self, "period time %u", period_time);
    self->period_time = period_time;
  }

  /* Set buffer size and period size manually for SPDIF */
  if (G_UNLIKELY (self->passthrough)) {
    snd_pcm_uframes_t buffer_size = SPDIF_BUFFER_SIZE;
    snd_pcm_uframes_t period_size = SPDIF_PERIOD_SIZE;

    CHECK (snd_pcm_hw_params_set_buffer_size_near (self->handle, params,
            &buffer_size), buffer_size);
    CHECK (snd_pcm_hw_params_set_period_size_near (self->handle, params,
            &period_size, NULL), period_size);
  }

  /* write the parameters to device */
  CHECK (snd_pcm_hw_params (self->handle, params), set_hw_params);

  /* now get the configured values */
  CHECK (snd_pcm_hw_params_get_buffer_size (params, &self->buffer_size),
      buffer_size);
  CHECK (snd_pcm_hw_params_get_period_size (params, &self->period_size, NULL),
      period_size);

  GST_DEBUG_OBJECT (self, "buffer size %lu, period size %lu", self->buffer_size,
      self->period_size);

  snd_pcm_hw_params_free (params);
  return 0;

  /* ERRORS */
no_config:
  {
    GST_ELEMENT_ERROR (self, RESOURCE, SETTINGS, (NULL),
        ("Broken configuration for playback: no configurations available: %s",
            snd_strerror (err)));
    snd_pcm_hw_params_free (params);
    return err;
  }
wrong_access:
  {
    GST_ELEMENT_ERROR (self, RESOURCE, SETTINGS, (NULL),
        ("Access type not available for playback: %s", snd_strerror (err)));
    snd_pcm_hw_params_free (params);
    return err;
  }
no_sample_format:
  {
    GST_ELEMENT_ERROR (self, RESOURCE, SETTINGS, (NULL),
        ("Sample format not available for playback: %s", snd_strerror (err)));
    snd_pcm_hw_params_free (params);
    return err;
  }
no_channels:
  {
    gchar *msg = g_strdup_printf (_("Could not open device for playback in %d-channel mode."), self->channels);

    GST_ELEMENT_ERROR (self, RESOURCE, SETTINGS, ("%s", msg),
        ("%s", snd_strerror (err)));
    g_free (msg);
    snd_pcm_hw_params_free (params);
    return err;
  }
no_rate:
  {
    GST_ELEMENT_ERROR (self, RESOURCE, SETTINGS, (NULL),
        ("Rate %iHz not available for playback: %s",
            self->rate, snd_strerror (err)));
    return err;
  }
buffer_size:
  {
    GST_ELEMENT_ERROR (self, RESOURCE, SETTINGS, (NULL),
        ("Unable to get buffer size for playback: %s", snd_strerror (err)));
    snd_pcm_hw_params_free (params);
    return err;
  }
period_size:
  {
    GST_ELEMENT_ERROR (self, RESOURCE, SETTINGS, (NULL),
        ("Unable to get period size for playback: %s", snd_strerror (err)));
    snd_pcm_hw_params_free (params);
    return err;
  }
set_hw_params:
  {
    GST_ELEMENT_ERROR (self, RESOURCE, SETTINGS, (NULL),
        ("Unable to set hw params for playback: %s", snd_strerror (err)));
    snd_pcm_hw_params_free (params);
    return err;
  }
}

static int
set_swparams (GstAlsaPassthroughSink * self)
{
  int err;
  snd_pcm_sw_params_t *params;

  snd_pcm_sw_params_malloc (&params);

  /* get the current swparams */
  CHECK (snd_pcm_sw_params_current (self->handle, params), no_config);
  /* start the transfer when the buffer is almost full: */
  /* (buffer_size / avail_min) * avail_min */
  CHECK (snd_pcm_sw_params_set_start_threshold (self->handle, params,
          (self->buffer_size / self->period_size) * self->period_size),
      start_threshold);

  /* allow the transfer when at least period_size samples can be processed */
  CHECK (snd_pcm_sw_params_set_avail_min (self->handle, params,
          self->period_size), set_avail);

  /* write the parameters to the playback device */
  CHECK (snd_pcm_sw_params (self->handle, params), set_sw_params);

  snd_pcm_sw_params_free (params);
  return 0;

  /* ERRORS */
no_config:
  {
    GST_ELEMENT_ERROR (self, RESOURCE, SETTINGS, (NULL),
        ("Unable to determine current swparams for playback: %s",
            snd_strerror (err)));
    snd_pcm_sw_params_free (params);
    return err;
  }
start_threshold:
  {
    GST_ELEMENT_ERROR (self, RESOURCE, SETTINGS, (NULL),
        ("Unable to set start threshold mode for playback: %s",
            snd_strerror (err)));
    snd_pcm_sw_params_free (params);
    return err;
  }
set_avail:
  {
    GST_ELEMENT_ERROR (self, RESOURCE, SETTINGS, (NULL),
        ("Unable to set avail min for playback: %s", snd_strerror (err)));
    snd_pcm_sw_params_free (params);
    return err;
  }

set_sw_params:
  {
    GST_ELEMENT_ERROR (self, RESOURCE, SETTINGS, (NULL),
        ("Unable to set sw params for playback: %s", snd_strerror (err)));
    snd_pcm_sw_params_free (params);
    return err;
  }
}

static gboolean
alsasink_parse_spec (GstAlsaPassthroughSink * self, GstAudioRingBufferSpec * spec)
{
  /* Initialize our boolean */
  self->passthrough = FALSE;

  switch (spec->type) {
    case GST_AUDIO_RING_BUFFER_FORMAT_TYPE_AC3:
    case GST_AUDIO_RING_BUFFER_FORMAT_TYPE_EAC3:
    case GST_AUDIO_RING_BUFFER_FORMAT_TYPE_DTS:
    case GST_AUDIO_RING_BUFFER_FORMAT_TYPE_MPEG:
      self->format = SND_PCM_FORMAT_S16_BE;
      self->passthrough = TRUE;
      break;
    default:
      return FALSE;
  }

  self->rate = GST_AUDIO_INFO_RATE (&spec->info);
  self->channels = 2;   /* always 2 channels for passthrough */
  self->buffer_time = spec->buffer_time;
  self->period_time = spec->latency_time;
  self->access = SND_PCM_ACCESS_RW_INTERLEAVED;

  return TRUE;
}

static gboolean
gst_alsapassthroughsink_open (GstAudioSink * asink)
{
  GstAlsaPassthroughSink *self = GST_ALSA_PASSTHROUGH_SINK (asink);
  gint err;

  /* open in non-blocking mode, we'll use snd_pcm_wait() for space to become
   * available. */
  CHECK (snd_pcm_open (&self->handle, self->device, SND_PCM_STREAM_PLAYBACK,
          SND_PCM_NONBLOCK), open_error);
  GST_LOG_OBJECT (self, "Opened device %s", self->device);

  return TRUE;

  /* ERRORS */
open_error:
  {
    if (err == -EBUSY) {
      GST_ELEMENT_ERROR (self, RESOURCE, BUSY,
          (_("Could not open audio device for playback. "
                  "Device is being used by another application.")),
          ("Device '%s' is busy", self->device));
    } else {
      GST_ELEMENT_ERROR (self, RESOURCE, OPEN_WRITE,
          (_("Could not open audio device for playback.")),
          ("Playback open error on device '%s': %s", self->device,
              snd_strerror (err)));
    }
    return FALSE;
  }
}

static gboolean
gst_alsapassthroughsink_prepare (GstAudioSink * asink, GstAudioRingBufferSpec * spec)
{
  GstAlsaPassthroughSink *self = GST_ALSA_PASSTHROUGH_SINK (asink);
  gint err;

  if (self->passthrough) {
    snd_pcm_close (self->handle);
    self->handle = gst_alsapassthroughsink_open_spdif (GST_OBJECT (self), self->device, spec);
    if (G_UNLIKELY (!self->handle)) {
      goto no_iec958;
    }
  }

  if (!alsasink_parse_spec (self, spec))
    goto spec_parse;

  CHECK (set_hwparams (self), hw_params_failed);
  CHECK (set_swparams (self), sw_params_failed);

  self->bpf = GST_AUDIO_INFO_BPF (&spec->info);
  spec->segsize = self->period_size * self->bpf;
  spec->segtotal = self->buffer_size / self->period_size;

  {
    snd_output_t *out_buf = NULL;
    char *msg = NULL;

    snd_output_buffer_open (&out_buf);
    snd_pcm_dump_hw_setup (self->handle, out_buf);
    snd_output_buffer_string (out_buf, &msg);
    GST_DEBUG_OBJECT (self, "Hardware setup: \n%s", msg);
    snd_output_close (out_buf);
    snd_output_buffer_open (&out_buf);
    snd_pcm_dump_sw_setup (self->handle, out_buf);
    snd_output_buffer_string (out_buf, &msg);
    GST_DEBUG_OBJECT (self, "Software setup: \n%s", msg);
    snd_output_close (out_buf);
  }

  return TRUE;

  /* ERRORS */
no_iec958:
  {
    GST_ELEMENT_ERROR (self, RESOURCE, OPEN_WRITE, (NULL),
        ("Could not open IEC958 (SPDIF) device for playback"));
    return FALSE;
  }
spec_parse:
  {
    GST_ELEMENT_ERROR (self, RESOURCE, SETTINGS, (NULL),
        ("Error parsing spec"));
    return FALSE;
  }
hw_params_failed:
  {
    GST_ELEMENT_ERROR (self, RESOURCE, SETTINGS, (NULL),
        ("Setting of hwparams failed: %s", snd_strerror (err)));
    return FALSE;
  }
sw_params_failed:
  {
    GST_ELEMENT_ERROR (self, RESOURCE, SETTINGS, (NULL),
        ("Setting of swparams failed: %s", snd_strerror (err)));
    return FALSE;
  }
}

static gboolean
gst_alsapassthroughsink_unprepare (GstAudioSink * asink)
{
  GstAlsaPassthroughSink *self = GST_ALSA_PASSTHROUGH_SINK (asink);

  snd_pcm_drop (self->handle);
  snd_pcm_hw_free (self->handle);

  return TRUE;
}

static gboolean
gst_alsapassthroughsink_close (GstAudioSink * asink)
{
  GstAlsaPassthroughSink *self = GST_ALSA_PASSTHROUGH_SINK (asink);

  GST_OBJECT_LOCK (asink);
  if (self->handle) {
    snd_pcm_close (self->handle);
    self->handle = NULL;
  }
  GST_OBJECT_UNLOCK (asink);

  return TRUE;
}


/*
 *   Underrun and suspend recovery
 */
static gint
xrun_recovery (GstAlsaPassthroughSink * alsa, snd_pcm_t * handle, gint err)
{
  GST_WARNING_OBJECT (alsa, "xrun recovery %d: %s", err, g_strerror (-err));

  if (err == -EPIPE) {          /* under-run */
    err = snd_pcm_prepare (handle);
    if (err < 0)
      GST_WARNING_OBJECT (alsa,
          "Can't recover from underrun, prepare failed: %s",
          snd_strerror (err));
    gst_audio_base_sink_report_device_failure (GST_AUDIO_BASE_SINK (alsa));
    return 0;
  } else if (err == -ESTRPIPE) {
    while ((err = snd_pcm_resume (handle)) == -EAGAIN)
      g_usleep (100);           /* wait until the suspend flag is released */

    if (err < 0) {
      err = snd_pcm_prepare (handle);
      if (err < 0)
        GST_WARNING_OBJECT (alsa,
            "Can't recover from suspend, prepare failed: %s",
            snd_strerror (err));
    }
    if (err == 0)
      gst_audio_base_sink_report_device_failure (GST_AUDIO_BASE_SINK (alsa));
    return 0;
  }
  return err;
}

static gint
gst_alsapassthroughsink_write (GstAudioSink * asink, gpointer data, guint length)
{
  GstAlsaPassthroughSink *self = GST_ALSA_PASSTHROUGH_SINK (asink);
  gint err;
  gint cptr;
  guint8 *ptr = data;

  GST_LOG_OBJECT (asink, "received audio samples buffer of %u bytes", length);

  cptr = length / self->bpf;

  GST_ALSA_PASSTHROUGH_SINK_LOCK (asink);
  while (cptr > 0) {
    /* start by doing a blocking wait for free space. Set the timeout
     * to 4 times the period time */
    err = snd_pcm_wait (self->handle, (4 * self->period_time / 1000));
    if (err < 0) {
      GST_DEBUG_OBJECT (asink, "wait error, %d", err);
    } else {
      GST_DELAY_SINK_LOCK (asink);
      err = snd_pcm_writei (self->handle, ptr, cptr);
      GST_DELAY_SINK_UNLOCK (asink);
    }

    GST_DEBUG_OBJECT (asink, "written %d frames out of %d", err, cptr);
    if (err < 0) {
      GST_DEBUG_OBJECT (asink, "Write error: %s", snd_strerror (err));
      if (err == -EAGAIN) {
        continue;
      } else if (err == -ENODEV) {
        goto device_disappeared;
      } else if (xrun_recovery (self, self->handle, err) < 0) {
        goto write_error;
      }
      continue;
    }

    ptr += snd_pcm_frames_to_bytes (self->handle, err);
    cptr -= err;
  }
  GST_ALSA_PASSTHROUGH_SINK_UNLOCK (asink);

  return length - (cptr * self->bpf);

write_error:
  {
    GST_ALSA_PASSTHROUGH_SINK_UNLOCK (asink);
    return length;              /* skip one period */
  }
device_disappeared:
  {
    GST_ELEMENT_ERROR (asink, RESOURCE, WRITE,
        (_("Error outputting to audio device. "
                "The device has been disconnected.")), (NULL));
    goto write_error;
  }
}

static guint
gst_alsapassthroughsink_delay (GstAudioSink * asink)
{
  GstAlsaPassthroughSink *self = GST_ALSA_PASSTHROUGH_SINK (asink);
  snd_pcm_sframes_t delay;
  int res;

  GST_DELAY_SINK_LOCK (asink);
  res = snd_pcm_delay (self->handle, &delay);
  GST_DELAY_SINK_UNLOCK (asink);
  if (G_UNLIKELY (res < 0)) {
    /* on errors, report 0 delay */
    GST_DEBUG_OBJECT (self, "snd_pcm_delay returned %d", res);
    delay = 0;
  }
  if (G_UNLIKELY (delay < 0)) {
    /* make sure we never return a negative delay */
    GST_WARNING_OBJECT (self, "snd_pcm_delay returned negative delay");
    delay = 0;
  }

  return delay;
}

static void
gst_alsapassthroughsink_reset (GstAudioSink * asink)
{
  GstAlsaPassthroughSink *self = GST_ALSA_PASSTHROUGH_SINK (asink);
  gint err;

  GST_ALSA_PASSTHROUGH_SINK_LOCK (asink);
  GST_DEBUG_OBJECT (self, "drop");
  CHECK (snd_pcm_drop (self->handle), drop_error);
  GST_DEBUG_OBJECT (self, "prepare");
  CHECK (snd_pcm_prepare (self->handle), prepare_error);
  GST_DEBUG_OBJECT (self, "reset done");
  GST_ALSA_PASSTHROUGH_SINK_UNLOCK (asink);

  return;

  /* ERRORS */
drop_error:
  {
    GST_ERROR_OBJECT (self, "alsa-reset: pcm drop error: %s",
        snd_strerror (err));
    GST_ALSA_PASSTHROUGH_SINK_UNLOCK (asink);
    return;
  }
prepare_error:
  {
    GST_ERROR_OBJECT (self, "alsa-reset: pcm prepare error: %s",
        snd_strerror (err));
    GST_ALSA_PASSTHROUGH_SINK_UNLOCK (asink);
    return;
  }
}

static GstBuffer *
gst_alsapassthroughsink_payload (GstAudioBaseSink * sink, GstBuffer * buf)
{
  GstAlsaPassthroughSink *alsa;

  alsa = GST_ALSA_PASSTHROUGH_SINK (sink);

  if (alsa->passthrough) {
    GstBuffer *out;
    gint framesize;
    GstMapInfo iinfo, oinfo;

    framesize = gst_audio_iec61937_frame_size (&sink->ringbuffer->spec);
    if (framesize <= 0)
      return NULL;

    out = gst_buffer_new_and_alloc (framesize);

    gst_buffer_map (buf, &iinfo, GST_MAP_READ);
    gst_buffer_map (out, &oinfo, GST_MAP_WRITE);

    if (!gst_audio_iec61937_payload (iinfo.data, iinfo.size,
            oinfo.data, oinfo.size, &sink->ringbuffer->spec, G_BIG_ENDIAN)) {
      gst_buffer_unmap (buf, &iinfo);
      gst_buffer_unmap (out, &oinfo);
      gst_buffer_unref (out);
      return NULL;
    }

    gst_buffer_unmap (buf, &iinfo);
    gst_buffer_unmap (out, &oinfo);

    gst_buffer_copy_into (out, buf, GST_BUFFER_COPY_METADATA, 0, -1);
    return out;
  }

  return gst_buffer_ref (buf);
}

static snd_pcm_t *
gst_alsapassthroughsink_open_spdif (GstObject * obj, gchar * device, GstAudioRingBufferSpec * spec)
{
  char *iec958_pcm_name = NULL;
  snd_pcm_t *pcm = NULL;
  int res;
  char devstr[256];             /* Storage for local 'default' device string */

  int aes3 = IEC958_AES3_CON_FS_NOTID;
  switch (spec->info.rate) {
  case 44100:
      aes3 = IEC958_AES3_CON_FS_44100;
      break;
  case 48000:
      aes3 = IEC958_AES3_CON_FS_48000;
      break;
  default:
      break;
  }

  sprintf (devstr,
      "%s,AES0=0x%02x,AES1=0x%02x,AES2=0x%02x,AES3=0x%02x",
      device,
      IEC958_AES0_NONAUDIO,                                 /* AES0 */
      IEC958_AES1_CON_ORIGINAL | IEC958_AES1_CON_PCM_CODER, /* AES1 */
      0,                                                    /* AES2 */
      aes3);                                                /* AES3 */

  GST_DEBUG_OBJECT (obj, "Generated device string \"%s\"", devstr);
  iec958_pcm_name = devstr;

  res = snd_pcm_open (&pcm, iec958_pcm_name, SND_PCM_STREAM_PLAYBACK, 0);
  if (G_UNLIKELY (res < 0)) {
    GST_DEBUG_OBJECT (obj, "failed opening IEC958 device: %s",
        snd_strerror (res));
    pcm = NULL;
  }

  return pcm;
}