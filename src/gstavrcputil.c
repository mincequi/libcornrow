/*
 * Copyright (C) 2015 Arun Raghavan <git@arunraghavan.net>
 *
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gstavrcputil.h"

#include <gio/gio.h>

struct _GstAvrcpConnection
{
    GMainContext *context;
    GMainLoop *mainloop;
    GThread *thread;

    GstAvrcpMetadataCb cb;
    gpointer user_data;
    GDestroyNotify user_data_free_cb;
};

GstAvrcpConnection * gst_avrcp_connection_new (GstAvrcpMetadataCb cb,
                                               gpointer user_data,
                                               GDestroyNotify user_data_free_cb)
{
    GstAvrcpConnection *avrcp;

    avrcp = g_new0 (GstAvrcpConnection, 1);

    avrcp->cb = cb;
    avrcp->user_data = user_data;
    avrcp->user_data_free_cb = user_data_free_cb;

    avrcp->context = g_main_context_new ();
    avrcp->mainloop = g_main_loop_new (avrcp->context, FALSE);

    g_main_context_push_thread_default (avrcp->context);
    g_main_context_pop_thread_default (avrcp->context);

    avrcp->thread = g_thread_new ("gstavrcp", (GThreadFunc) g_main_loop_run, avrcp->mainloop);

    return avrcp;
}

void gst_avrcp_connection_free (GstAvrcpConnection * avrcp)
{
  g_main_loop_quit (avrcp->mainloop);
  g_main_loop_unref (avrcp->mainloop);

  g_main_context_unref (avrcp->context);

  g_thread_join (avrcp->thread);

  if (avrcp->user_data_free_cb)
    avrcp->user_data_free_cb (avrcp->user_data);

  g_free (avrcp);
}
