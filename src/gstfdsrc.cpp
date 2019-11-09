#include <sys/types.h>

#include <sys/stat.h>
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#include "gstfdsrc.h"

static GstStaticPadTemplate srctemplate = GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS_ANY);

GST_DEBUG_CATEGORY_STATIC (gst_fd_src_debug);
#define GST_CAT_DEFAULT gst_fd_src_debug

#define DEFAULT_FD              0
#define DEFAULT_TIMEOUT         0

enum
{
  PROP_0,

  PROP_FD,
  PROP_TIMEOUT,

  PROP_LAST
};

#define _do_init \
  GST_DEBUG_CATEGORY_INIT (gst_fd_src_debug, "fdsrc", 0, "fdsrc element");
#define gst_fd_src_parent_class parent_class
G_DEFINE_TYPE_WITH_CODE (GstFd2Src, gst_fd_src, GST_TYPE_PUSH_SRC, _do_init);

static gboolean gst_fd_src_start (GstBaseSrc * bsrc);
static gboolean gst_fd_src_stop (GstBaseSrc * bsrc);
static gboolean gst_fd_src_unlock (GstBaseSrc * bsrc);
static gboolean gst_fd_src_unlock_stop (GstBaseSrc * bsrc);

static GstFlowReturn gst_fd_src_create (GstPushSrc * psrc, GstBuffer ** outbuf);

static void
gst_fd_src_class_init (GstFd2SrcClass * klass)
{
  // GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GstElementClass *gstelement_class;
  GstBaseSrcClass *gstbasesrc_class;
  GstPushSrcClass *gstpush_src_class;

  gstelement_class = GST_ELEMENT_CLASS (klass);
  gstbasesrc_class = GST_BASE_SRC_CLASS (klass);
  gstpush_src_class = GST_PUSH_SRC_CLASS (klass);

  gst_element_class_set_static_metadata (gstelement_class,
      "Filedescriptor Source",
      "Source/File",
      "Read from a file descriptor", "Erik Walthinsen <omega@cse.ogi.edu>");
  gst_element_class_add_static_pad_template (gstelement_class, &srctemplate);

  gstbasesrc_class->start = GST_DEBUG_FUNCPTR (gst_fd_src_start);
  gstbasesrc_class->stop = GST_DEBUG_FUNCPTR (gst_fd_src_stop);
  gstbasesrc_class->unlock = GST_DEBUG_FUNCPTR (gst_fd_src_unlock);
  gstbasesrc_class->unlock_stop = GST_DEBUG_FUNCPTR (gst_fd_src_unlock_stop);

  gstpush_src_class->create = GST_DEBUG_FUNCPTR (gst_fd_src_create);
}

static void
gst_fd_src_init (GstFd2Src * fdsrc)
{
  fdsrc->fd = -1;
  fdsrc->size = -1;

  gst_base_src_set_format (GST_BASE_SRC (fdsrc), GST_FORMAT_TIME);
  gst_base_src_set_live (GST_BASE_SRC (fdsrc), TRUE);
  gst_base_src_set_do_timestamp (GST_BASE_SRC (fdsrc), TRUE);
}

static void
gst_fd_src_update_fd (GstFd2Src * src, guint64 size)
{
  /* we need to always update the fdset since it may not have existed when
   * gst_fd_src_update_fd () was called earlier */
  if (src->fdset != NULL) {
    GstPollFD fd = GST_POLL_FD_INIT;

    if (src->fd >= 0) {
      fd.fd = src->fd;
      /* this will log a harmless warning, if it was never added */
      gst_poll_remove_fd (src->fdset, &fd);
    }

    gst_poll_add_fd (src->fdset, &fd);
    gst_poll_fd_ctl_read (src->fdset, &fd, TRUE);
  }
}

static gboolean
gst_fd_src_start (GstBaseSrc * bsrc)
{
  GstFd2Src *src = GST_FD_SRC (bsrc);

  src->fdset = gst_poll_new (TRUE);
  gst_fd_src_update_fd (src, -1);

  return TRUE;
}

static gboolean
gst_fd_src_stop (GstBaseSrc * bsrc)
{
  GstFd2Src *src = GST_FD_SRC (bsrc);

  if (src->fdset) {
    gst_poll_free (src->fdset);
    src->fdset = NULL;
  }

  return TRUE;
}

static gboolean
gst_fd_src_unlock (GstBaseSrc * bsrc)
{
  GstFd2Src *src = GST_FD_SRC (bsrc);

  GST_LOG_OBJECT (src, "Flushing");
  GST_OBJECT_LOCK (src);
  gst_poll_set_flushing (src->fdset, TRUE);
  GST_OBJECT_UNLOCK (src);

  return TRUE;
}

static gboolean
gst_fd_src_unlock_stop (GstBaseSrc * bsrc)
{
  GstFd2Src *src = GST_FD_SRC (bsrc);

  GST_LOG_OBJECT (src, "No longer flushing");
  GST_OBJECT_LOCK (src);
  gst_poll_set_flushing (src->fdset, FALSE);
  GST_OBJECT_UNLOCK (src);

  return TRUE;
}

static GstFlowReturn gst_fd_src_create (GstPushSrc * psrc, GstBuffer ** outbuf)
{
  GstFd2Src *src;
  GstBuffer *buf;
  gssize readbytes;
  guint blocksize;
  GstMapInfo info;

  gboolean try_again;
  gint retval;

  src = GST_FD_SRC (psrc);

#ifndef HAVE_WIN32
  do {
    try_again = FALSE;

    retval = gst_poll_wait (src->fdset, GST_CLOCK_TIME_NONE);
    GST_LOG_OBJECT (src, "poll returned %d", retval);

    if (G_UNLIKELY (retval == -1)) {
      if (errno == EINTR || errno == EAGAIN) {
        /* retry if interrupted */
        try_again = TRUE;
      } else if (errno == EBUSY) {
        goto stopped;
      } else {
        goto poll_error;
      }
    } else if (G_UNLIKELY (retval == 0)) {
      try_again = TRUE;
      /* timeout, post element message */
    }
  } while (G_UNLIKELY (try_again));     /* retry if interrupted or timeout */
#endif

  blocksize = GST_BASE_SRC (src)->blocksize;

  /* create the buffer */
  buf = gst_buffer_new_allocate (NULL, blocksize, NULL);
  if (G_UNLIKELY (buf == NULL))
    goto alloc_failed;

  if (!gst_buffer_map (buf, &info, GST_MAP_WRITE))
    goto buffer_read_error;

  do {
    readbytes = read (src->fd, info.data, blocksize);
    GST_LOG_OBJECT (src, "read %" G_GSSIZE_FORMAT, readbytes);
  } while (readbytes == -1 && errno == EINTR);  /* retry if interrupted */

  if (readbytes < 0)
    goto read_error;

  gst_buffer_unmap (buf, &info);
  gst_buffer_resize (buf, 0, readbytes);

  if (readbytes == 0)
    goto eos;

  GST_BUFFER_TIMESTAMP (buf) = GST_CLOCK_TIME_NONE;

  GST_LOG_OBJECT (psrc, "Read buffer of size %" G_GSSIZE_FORMAT, readbytes);

  /* we're done, return the buffer */
  *outbuf = buf;

  return GST_FLOW_OK;

  /* ERRORS */
#ifndef HAVE_WIN32
poll_error:
  {
    GST_ELEMENT_ERROR (src, RESOURCE, READ, (NULL),
        ("poll on file descriptor: %s.", g_strerror (errno)));
    GST_DEBUG_OBJECT (psrc, "Error during poll");
    return GST_FLOW_ERROR;
  }
stopped:
  {
    GST_DEBUG_OBJECT (psrc, "Poll stopped");
    return GST_FLOW_FLUSHING;
  }
#endif
alloc_failed:
  {
    GST_ERROR_OBJECT (src, "Failed to allocate %u bytes", blocksize);
    return GST_FLOW_ERROR;
  }
eos:
  {
    GST_DEBUG_OBJECT (psrc, "Read 0 bytes. EOS.");
    gst_buffer_unref (buf);
    return GST_FLOW_EOS;
  }
read_error:
  {
    GST_ELEMENT_ERROR (src, RESOURCE, READ, (NULL),
        ("read on file descriptor: %s.", g_strerror (errno)));
    GST_DEBUG_OBJECT (psrc, "Error reading from fd");
    gst_buffer_unmap (buf, &info);
    gst_buffer_unref (buf);
    return GST_FLOW_ERROR;
  }
buffer_read_error:
  {
    GST_ELEMENT_ERROR (src, RESOURCE, WRITE, (NULL), ("Can't write to buffer"));
    gst_buffer_unref (buf);
    return GST_FLOW_ERROR;
  }
}

