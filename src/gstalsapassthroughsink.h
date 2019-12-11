/* GStreamer
 * Copyright (C)  2005 Wim Taymans <wim@fluendo.com>
 *
 * GstAlsaPassthroughSink.h:
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


#ifndef __GST_ALSA_PASSTHROUGH_SINK_H__
#define __GST_ALSA_PASSTHROUGH_SINK_H__

#include <gst/gst.h>
#include <gst/audio/audio.h>
#include <alsa/asoundlib.h>

G_BEGIN_DECLS

#define GST_TYPE_ALSA_PASSTHROUGH_SINK            (gst_alsapassthroughsink_get_type())
#define GST_ALSA_PASSTHROUGH_SINK(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_ALSA_PASSTHROUGH_SINK,GstAlsaPassthroughSink))
#define GST_ALSA_PASSTHROUGH_SINK_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_ALSA_PASSTHROUGH_SINK,GstAlsaPassthroughSinkClass))
#define GST_IS_ALSA_PASSTHROUGH_SINK(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_ALSA_PASSTHROUGH_SINK))
#define GST_IS_ALSA_PASSTHROUGH_SINK_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_ALSA_PASSTHROUGH_SINK))
#define GST_ALSA_PASSTHROUGH_SINK_CAST(obj)       ((GstAlsaPassthroughSink *) (obj))

typedef struct _GstAlsaPassthroughSink GstAlsaPassthroughSink;
typedef struct _GstAlsaPassthroughSinkClass GstAlsaPassthroughSinkClass;

#define GST_ALSA_PASSTHROUGH_SINK_GET_LOCK(obj)	(&GST_ALSA_PASSTHROUGH_SINK_CAST (obj)->alsa_lock)
#define GST_ALSA_PASSTHROUGH_SINK_LOCK(obj)	    (g_mutex_lock (GST_ALSA_PASSTHROUGH_SINK_GET_LOCK (obj)))
#define GST_ALSA_PASSTHROUGH_SINK_UNLOCK(obj)   (g_mutex_unlock (GST_ALSA_PASSTHROUGH_SINK_GET_LOCK (obj)))

#define GST_DELAY_SINK_GET_LOCK(obj)	(&GST_ALSA_PASSTHROUGH_SINK_CAST (obj)->delay_lock)
#define GST_DELAY_SINK_LOCK(obj)	        (g_mutex_lock (GST_DELAY_SINK_GET_LOCK (obj)))
#define GST_DELAY_SINK_UNLOCK(obj)	(g_mutex_unlock (GST_DELAY_SINK_GET_LOCK (obj)))

/**
 * GstAlsaPassthroughSink:
 *
 * Opaque data structure
 */
struct _GstAlsaPassthroughSink {
  GstAudioSink    sink;

  gchar                 *device;

  snd_pcm_t             *handle;

  snd_pcm_access_t access;
  snd_pcm_format_t format;
  guint rate;
  guint channels;
  gint bpf;
  gboolean need_swap;

  guint buffer_time;
  guint period_time;
  snd_pcm_uframes_t buffer_size;
  snd_pcm_uframes_t period_size;

  GMutex alsa_lock;
  GMutex delay_lock;
};

struct _GstAlsaPassthroughSinkClass {
  GstAudioSinkClass parent_class;
};

GType gst_alsapassthroughsink_get_type(void);

G_END_DECLS

#endif /* __GST_ALSA_PASSTHROUGH_SINK_H__ */
