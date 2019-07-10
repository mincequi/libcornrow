/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2004-2010  Marcel Holtmann <marcel@holtmann.org>
 *  Copyright (C) 2012 Collabora Ltd.
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <stdint.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <bluetooth/bluetooth.h>

#include <gst/gst.h>
#include "gstavdtputil.h"

GST_DEBUG_CATEGORY (avdtp_debug);
//GST_DEBUG_CATEGORY_EXTERN (avdtp_debug);
#define GST_CAT_DEFAULT avdtp_debug

gboolean gst_avdtp_connection_acquire (GstAvdtpConnection * conn, int fd, uint16_t blocksize)
{
    conn->stream = g_io_channel_unix_new (fd);
    g_io_channel_set_encoding (conn->stream, NULL, NULL);
    g_io_channel_set_close_on_unref (conn->stream, TRUE);
    conn->blocksize = blocksize;

    return TRUE;
}

void gst_avdtp_connection_release (GstAvdtpConnection * conn)
{
    if (conn->stream) {
        g_io_channel_shutdown (conn->stream, TRUE, NULL);
        g_io_channel_unref (conn->stream);
        conn->stream = NULL;
    }
}

gboolean gst_avdtp_connection_conf_recv_stream_fd (GstAvdtpConnection * conn)
{
    GIOStatus status;
    GIOFlags flags;
    int fd;
    int priority;

    /* Proceed if stream was already acquired */
    if (conn->stream == NULL) {
        GST_ERROR ("Error while configuring device: could not acquire audio socket");
        return FALSE;
    }

    /* set stream socket to nonblock */
    flags = g_io_channel_get_flags (conn->stream);
    flags |= G_IO_FLAG_NONBLOCK;
    status = g_io_channel_set_flags (conn->stream, flags, NULL);
    if (status != G_IO_STATUS_NORMAL)
        GST_WARNING ("Error while setting server socket to nonblock");

    fd = g_io_channel_unix_get_fd (conn->stream);

    /* It is possible there is some outstanding data in the pipe - we have to empty it */
    while (1) {
        ssize_t bread = read (fd, conn->buffer, conn->blocksize);
        if (bread <= 0) {
            break;
        }
    }

    /* set stream socket to block */
    flags = g_io_channel_get_flags (conn->stream);
    flags &= ~G_IO_FLAG_NONBLOCK;
    status = g_io_channel_set_flags (conn->stream, flags, NULL);
    if (status != G_IO_STATUS_NORMAL) {
        GST_WARNING ("Error while setting server socket to block");
    }

    priority = 6;
    if (setsockopt (fd, SOL_SOCKET, SO_PRIORITY, (const void *) &priority, sizeof (priority)) < 0) {
        GST_WARNING ("Unable to set socket to low delay");
    }

    memset (conn->buffer, 0, sizeof (conn->buffer));

    return TRUE;
}
