/*
 *  Copyright (c) 2013 matias <mati86dl@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA.
 */

/**
* SECTION:libmpris2client
* @short_description: Main interface to connect with mpris2 players
* @title: Mpris2Client
* @section_id:
* @stability: Unstable
* @include: mpris2client/mpris2client.h
*
* All functions used to connect with mpris2 players is located here.
*/

#include <gio/gio.h>

#include "libmpris2client.h"
#include "mpris2-metadata.h"

/**
 * Libmpri2client:
 * It is a generic library for controlling any mpris2 compatible player
 */

struct _Mpris2Client
{
	GObject parent_instance;

	/* Priv */
	GDBusConnection *gconnection;
	GDBusProxy      *props_proxy;
	GDBusProxy      *player_proxy;
	gchar			*dbus_name;
	guint            watch_id;
	guint            playback_timer_id;

	/* Settings. */
	gchar           *player;
	gboolean         strict_mode;

	/* Status */
	gboolean         connected;

	/* Interface MediaPlayer2 */
	gboolean         can_quit;
	gboolean         can_raise;
	gboolean         has_tracklist;
	gchar           *identity;
	gchar          **supported_uri_schemes;
	gchar          **supported_mime_types;

	/* Optionals Interface MediaPlayer2 */
	gboolean         fullscreen;
	gboolean         can_set_fullscreen;
	gchar           *desktop_entry;

	/* Interface MediaPlayer2.Player */
	PlaybackStatus   playback_status;
	gdouble          rate;
	Mpris2Metadata  *metadata;
	gdouble          volume;
	gint             position;
	gdouble          minimum_rate;
	gdouble          maximum_rate;
	gboolean         can_go_next;
	gboolean         can_go_previous;
	gboolean         can_play;
	gboolean         can_pause;
	gboolean         can_seek;
	gboolean         can_control;

	/* Optionals Interface MediaPlayer2.Player */
	gboolean         has_loop_status;
	LoopStatus       loop_status;

	gboolean         has_shuffle;
	gboolean         shuffle;
};

enum
{
	CONNECTION,
	PLAYBACK_STATUS,
	PLAYBACK_TICK,
	METADATA,
	VOLUME,
	LOOP_STATUS,
	SHUFFLE,
	LAST_SIGNAL
};
static int signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (Mpris2Client, mpris2_client, G_TYPE_OBJECT)

/*
 * Prototypes
 */
static void      mpris2_client_call_player_method              (Mpris2Client *mpris2, const char *method);
static void      mpris2_client_call_media_player_method        (Mpris2Client *mpris2, const char *method);

static void      mpris2_client_connect_dbus                    (Mpris2Client *mpris2);

static GVariant *mpris2_client_get_all_player_properties       (Mpris2Client *mpris2);
static GVariant *mpris2_client_get_player_properties           (Mpris2Client *mpris2, const gchar *prop);
static void      mpris2_client_set_player_properties           (Mpris2Client *mpris2, const gchar *prop, GVariant *vprop);

static GVariant *mpris2_client_get_all_media_player_properties (Mpris2Client *mpris2);
static void      mpris2_client_set_media_player_properties     (Mpris2Client *mpris2, const gchar *prop, GVariant *vprop);

/**
 * mpris2_client_new:
 *
 * Returns: (transfer full): a new instance of mpris2client.
 */

Mpris2Client *
mpris2_client_new (void)
{
	return g_object_new(MPRIS2_TYPE_CLIENT, NULL);
}

gboolean
mpris2_client_get_strict_mode (Mpris2Client *mpris2)
{
	return mpris2->strict_mode;
}

void
mpris2_client_set_strict_mode (Mpris2Client *mpris2, gboolean strict_mode)
{
	mpris2->strict_mode = strict_mode;
}

/*
 *  Interface MediaPlayer2.Player Methods
 */

void
mpris2_client_prev (Mpris2Client *mpris2)
{
	if (!mpris2->connected)
		return;

	if (!mpris2->can_control)
		return;

	if (mpris2->strict_mode && !mpris2->can_go_previous)
		return;

	mpris2_client_call_player_method (mpris2, "Previous");
}

void
mpris2_client_next (Mpris2Client *mpris2)
{
	if (!mpris2->connected)
		return;

	if (!mpris2->can_control)
		return;

	if (mpris2->strict_mode && !mpris2->can_go_next)
		return;

	mpris2_client_call_player_method (mpris2, "Next");
}

void
mpris2_client_pause (Mpris2Client *mpris2)
{
	if (!mpris2->connected)
		return;

	if (!mpris2->can_control)
		return;

	if (mpris2->strict_mode && !mpris2->can_pause)
		return;

	mpris2_client_call_player_method (mpris2, "Pause");
}

void
mpris2_client_play_pause (Mpris2Client *mpris2)
{
	if (!mpris2->connected)
		return;

	if (!mpris2->can_control)
		return;

	if (mpris2->strict_mode && !mpris2->can_pause)
		return;

	mpris2_client_call_player_method (mpris2, "PlayPause");
}

void
mpris2_client_stop (Mpris2Client *mpris2)
{
	if (!mpris2->connected)
		return;

	if (!mpris2->can_control)
		return;

	if (mpris2->strict_mode && !mpris2->can_control)
		return;

	mpris2_client_call_player_method (mpris2, "Stop");
}

void
mpris2_client_play (Mpris2Client *mpris2)
{
	if (!mpris2->connected)
		return;

	if (!mpris2->can_control)
		return;

	if (mpris2->strict_mode && !mpris2->can_play)
		return;

	mpris2_client_call_player_method (mpris2, "Play");
}

void
mpris2_client_seek (Mpris2Client *mpris2, gint offset)
{
	GDBusMessage *message;
	GError       *error = NULL;

	message = g_dbus_message_new_method_call (mpris2->dbus_name,
	                                          "/org/mpris/MediaPlayer2",
	                                          "org.mpris.MediaPlayer2.Player",
	                                          "Seek");
	g_dbus_message_set_body (message, g_variant_new ("(x)", offset));

	g_dbus_connection_send_message (mpris2->gconnection,
	                                message,
	                                G_DBUS_SEND_MESSAGE_FLAGS_NONE,
	                                NULL,
	                                &error);
	if (error != NULL) {
		g_warning ("unable to send message: %s", error->message);
		g_clear_error (&error);
		error = NULL;
	}

	g_dbus_connection_flush_sync (mpris2->gconnection, NULL, &error);
	if (error != NULL) {
		g_warning ("unable to flush message queue: %s", error->message);
		g_clear_error (&error);
	}

	g_object_unref (message);
}

void
mpris2_client_set_position (Mpris2Client *mpris2, const gchar *track_id, gint position)
{
	GDBusMessage *message;
	GError       *error = NULL;

	message = g_dbus_message_new_method_call (mpris2->dbus_name,
	                                          "/org/mpris/MediaPlayer2",
	                                          "org.mpris.MediaPlayer2.Player",
	                                          "SetPosition");
	g_dbus_message_set_body (message, g_variant_new ("(ox)", track_id, position));

	g_dbus_connection_send_message (mpris2->gconnection,
	                                message,
	                                G_DBUS_SEND_MESSAGE_FLAGS_NONE,
	                                NULL,
	                                &error);
	if (error != NULL) {
		g_warning ("unable to send message: %s", error->message);
		g_clear_error (&error);
		error = NULL;
	}

	g_dbus_connection_flush_sync (mpris2->gconnection, NULL, &error);
	if (error != NULL) {
		g_warning ("unable to flush message queue: %s", error->message);
		g_clear_error (&error);
	}

	g_object_unref (message);
}

void
mpris2_client_open_uri (Mpris2Client *mpris2, const gchar *uri)
{
	GDBusMessage *message;
	GError       *error = NULL;

	message = g_dbus_message_new_method_call (mpris2->dbus_name,
	                                          "/org/mpris/MediaPlayer2",
	                                          "org.mpris.MediaPlayer2.Player",
	                                          "OpenUri");
	g_dbus_message_set_body (message, g_variant_new ("(s)", uri));

	g_dbus_connection_send_message (mpris2->gconnection,
	                                message,
	                                G_DBUS_SEND_MESSAGE_FLAGS_NONE,
	                                NULL,
	                                &error);
	if (error != NULL) {
		g_warning ("unable to send message: %s", error->message);
		g_clear_error (&error);
		error = NULL;
	}

	g_dbus_connection_flush_sync (mpris2->gconnection, NULL, &error);
	if (error != NULL) {
		g_warning ("unable to flush message queue: %s", error->message);
		g_clear_error (&error);
	}

	g_object_unref (message);
}

/*
 *  Interface MediaPlayer2 Methods
 */

void
mpris2_client_raise_player (Mpris2Client *mpris2)
{
	if (!mpris2->can_raise)
		return;

	mpris2_client_call_media_player_method (mpris2, "Raise");
}

void
mpris2_client_quit_player (Mpris2Client *mpris2)
{
	if (!mpris2->can_quit)
		return;

	mpris2_client_call_media_player_method (mpris2, "Quit");
}

void
mpris2_client_set_fullscreen_player (Mpris2Client *mpris2, gboolean fullscreen)
{
	if (!mpris2->can_set_fullscreen)
		return;

	mpris2_client_set_media_player_properties (mpris2, "Fullscreen", g_variant_new_boolean(fullscreen));
}

/*
 * Interface MediaPlayer2.Player properties.
 */

PlaybackStatus
mpris2_client_get_playback_status (Mpris2Client *mpris2)
{
	return mpris2->playback_status;
}

gdouble
mpris2_client_get_playback_rate (Mpris2Client *mpris2)
{
	return mpris2->rate;
}

Mpris2Metadata *
mpris2_client_get_metadata (Mpris2Client *mpris2)
{
	return mpris2->metadata;
}

gdouble
mpris2_client_get_volume (Mpris2Client *mpris2)
{
	return mpris2->volume;
}

void
mpris2_client_set_volume (Mpris2Client *mpris2, gdouble volume)
{
	mpris2_client_set_player_properties (mpris2, "Volume", g_variant_new_double(volume));
}

gint
mpris2_client_get_position (Mpris2Client *mpris2)
{
	return mpris2->position;
}

gint
mpris2_client_get_accurate_position (Mpris2Client *mpris2)
{
	GVariant *value;
	value = mpris2_client_get_player_properties (mpris2, "Position");

	return (gint) g_variant_get_int64 (value);
}

gdouble
mpris2_client_get_minimum_rate (Mpris2Client *mpris2)
{
	return mpris2->minimum_rate;
}

gdouble
mpris2_client_get_maximum_rate (Mpris2Client *mpris2)
{
	return mpris2->maximum_rate;
}

gboolean
mpris2_client_get_can_go_next (Mpris2Client *mpris2)
{
	return mpris2->can_go_next;
}

gboolean
mpris2_client_get_can_go_previous (Mpris2Client *mpris2)
{
	return mpris2->can_go_previous;
}

gboolean
mpris2_client_get_can_play (Mpris2Client *mpris2)
{
	return mpris2->can_play;
}

gboolean
mpris2_client_get_can_pause (Mpris2Client *mpris2)
{
	return mpris2->can_pause;
}

gboolean
mpris2_client_get_can_seek (Mpris2Client *mpris2)
{
	return mpris2->can_seek;
}

gboolean
mpris2_client_get_can_control (Mpris2Client *mpris2)
{
	return mpris2->can_control;
}

/*
 * Optionals Interface MediaPlayer2.Player properties.
 */

gboolean
mpris2_client_player_has_loop_status (Mpris2Client *mpris2)
{
	return mpris2->has_loop_status;
}

LoopStatus
mpris2_client_get_loop_status (Mpris2Client *mpris2)
{
	return mpris2->loop_status;
}

void
mpris2_client_set_loop_status (Mpris2Client *mpris2, LoopStatus loop_status)
{
	if (!mpris2->has_loop_status)
		return;

	switch (loop_status) {
		case TRACK:
			mpris2_client_set_player_properties (mpris2, "LoopStatus", g_variant_new_string("Track"));
			break;
		case PLAYLIST:
			mpris2_client_set_player_properties (mpris2, "LoopStatus", g_variant_new_string("Playlist"));
			break;
		case NONE:
		default:
			mpris2_client_set_player_properties (mpris2, "LoopStatus", g_variant_new_string("None"));
			break;
	}
}

gboolean
mpris2_client_player_has_shuffle (Mpris2Client *mpris2)
{
	return mpris2->has_shuffle;
}

gboolean
mpris2_client_get_shuffle (Mpris2Client *mpris2)
{
	return mpris2->shuffle;
}

void
mpris2_client_set_shuffle (Mpris2Client *mpris2, gboolean shuffle)
{
	if (!mpris2->has_shuffle)
		return;

	mpris2_client_set_player_properties (mpris2, "Shuffle", g_variant_new_boolean(shuffle));
}

/*
 * Interface MediaPlayer2 Properties.
 */

gboolean
mpris2_client_can_quit (Mpris2Client *mpris2)
{
	return mpris2->can_quit;
}

gboolean
mpris2_client_can_set_fullscreen (Mpris2Client *mpris2)
{
	return mpris2->can_set_fullscreen;
}

gboolean
mpris2_client_can_raise (Mpris2Client *mpris2)
{
	return mpris2->can_raise;
}

gboolean
mpris2_client_has_tracklist_support (Mpris2Client *mpris2)
{
	return mpris2->has_tracklist;
}

const gchar *
mpris2_client_get_player_identity (Mpris2Client *mpris2)
{
	return mpris2->identity;
}

const gchar *
mpris2_client_get_player_desktop_entry (Mpris2Client *mpris2)
{
	return mpris2->desktop_entry;
}

gchar **
mpris2_client_get_supported_uri_schemes (Mpris2Client *mpris2)
{
	return mpris2->supported_uri_schemes;
}

gchar **
mpris2_client_get_supported_mime_types (Mpris2Client *mpris2)
{
	return mpris2->supported_mime_types;
}

const gchar *
mpris2_client_get_player (Mpris2Client *mpris2)
{
	return mpris2->player;
}

void
mpris2_client_set_player (Mpris2Client *mpris2, const gchar *player)
{
	/* Disconnect dbus */
	if (mpris2->watch_id) {
		g_bus_unwatch_name (mpris2->watch_id);
		mpris2->watch_id = 0;
	}
	if (mpris2->props_proxy != NULL) {
		g_object_unref (mpris2->props_proxy);
		mpris2->props_proxy = NULL;
	}
	if (mpris2->player_proxy != NULL) {
		g_object_unref (mpris2->player_proxy);
		mpris2->player_proxy = NULL;
	}

	/* Clean player */
	if (mpris2->player != NULL) {
		g_free (mpris2->player);
		mpris2->player = NULL;
	}

	/* Set new player and connect again */
	if (player != NULL) {
		mpris2->player = g_strdup(player);

		mpris2_client_connect_dbus (mpris2);
	}
}

gboolean
mpris2_client_auto_connect (Mpris2Client *mpris2)
{
	gboolean ret = FALSE;
	gchar **players = mpris2_client_get_available_players (mpris2);

	if (players != NULL) {
		mpris2_client_set_player (mpris2, players[0]);
		g_strfreev (players);
		ret = TRUE;
	}

	return ret;
}

gboolean
mpris2_client_is_connected (Mpris2Client *mpris2)
{
	return mpris2->connected;
}

/*
 * Position handlers.
 */
static gboolean
playback_tick_emit_cb (gpointer user_data)
{
	Mpris2Client *mpris2 = user_data;

	mpris2->position += (mpris2->rate)*1000000;

	g_signal_emit (mpris2, signals[PLAYBACK_TICK], 0, mpris2->position);

	return TRUE;
}

/*
 * SoundmenuDbus.
 */

/* Send mesages to use methods of org.mpris.MediaPlayer2.Player interfase. */

static void
mpris2_client_call_player_method (Mpris2Client *mpris2, const char *method)
{
	GDBusMessage *message;
	GError       *error = NULL;

	message = g_dbus_message_new_method_call (mpris2->dbus_name,
	                                          "/org/mpris/MediaPlayer2",
	                                          "org.mpris.MediaPlayer2.Player",
	                                          method);

	g_dbus_connection_send_message (mpris2->gconnection,
	                                message,
	                                G_DBUS_SEND_MESSAGE_FLAGS_NONE,
	                                NULL,
	                                &error);
	if (error != NULL) {
		g_warning ("unable to send message: %s", error->message);
		g_clear_error (&error);
		error = NULL;
	}

	g_dbus_connection_flush_sync (mpris2->gconnection, NULL, &error);
	if (error != NULL) {
		g_warning ("unable to flush message queue: %s", error->message);
		g_clear_error (&error);
	}

	g_object_unref (message);
}

/* Send mesages to use methods of org.mpris.MediaPlayer2 interfase. */

static void
mpris2_client_call_media_player_method (Mpris2Client *mpris2, const char *method)
{
	GDBusMessage *message;
	GError       *error = NULL;

	message = g_dbus_message_new_method_call (mpris2->dbus_name,
	                                          "/org/mpris/MediaPlayer2",
	                                          "org.mpris.MediaPlayer2",
	                                          method);

	g_dbus_connection_send_message (mpris2->gconnection,
	                                message,
	                                G_DBUS_SEND_MESSAGE_FLAGS_NONE,
	                                NULL,
	                                &error);
	if (error != NULL) {
		g_warning ("unable to send message: %s", error->message);
		g_clear_error (&error);
		error = NULL;
	}

	g_dbus_connection_flush_sync (mpris2->gconnection, NULL, &error);
	if (error != NULL) {
		g_warning ("unable to flush message queue: %s", error->message);
		g_clear_error (&error);
	}

	g_object_unref (message);
}

/* Returns the first player name that compliant to mpris2 on dbus.  */

gchar **
mpris2_client_get_available_players (Mpris2Client *mpris2)
{
	GError *error = NULL;
	GVariant *v;
	GVariantIter *iter;
	const gchar *str = NULL;
	gchar **res = NULL;
	guint items = 0;

	v = g_dbus_connection_call_sync (mpris2->gconnection,
	                                 "org.freedesktop.DBus",
	                                 "/org/freedesktop/DBus",
	                                 "org.freedesktop.DBus",
	                                 "ListNames",
	                                 NULL,
	                                 G_VARIANT_TYPE ("(as)"),
	                                 G_DBUS_CALL_FLAGS_NONE,
	                                 -1,
	                                 NULL,
	                                 &error);
	if (error) {
		g_critical ("Could not get a list of names registered on the session bus, %s",
		            error ? error->message : "no error given");
		g_clear_error (&error);
		return NULL;
	}

	g_variant_get (v, "(as)", &iter);
	while (g_variant_iter_loop (iter, "&s", &str)) {
		if (g_str_has_prefix(str, "org.mpris.MediaPlayer2.")) {
			res = (gchar**)g_realloc(res, (items + 1) * sizeof(gchar*));
			res[items] = g_strdup(str + 23);
			items++;
		}
	}

	/* Add NULL termination to the res vector */
	if (items > 0) {
		res = g_realloc(res, (items + 1) * sizeof(gchar*));
		res[items] = NULL;
	}

	g_variant_iter_free (iter);
	g_variant_unref (v);

	return res;
}

/* Get all properties using org.freedesktop.DBus.Properties interface.  */

static GVariant *
mpris2_client_get_all_player_properties (Mpris2Client *mpris2)
{
	GVariantIter iter;
	GVariant *result, *child = NULL;

	result = g_dbus_connection_call_sync (mpris2->gconnection,
	                                      mpris2->dbus_name,
	                                      "/org/mpris/MediaPlayer2",
	                                      "org.freedesktop.DBus.Properties",
	                                      "GetAll",
	                                      g_variant_new ("(s)", "org.mpris.MediaPlayer2.Player"),
	                                      G_VARIANT_TYPE ("(a{sv})"),
	                                      G_DBUS_CALL_FLAGS_NONE,
	                                      -1,
	                                      NULL,
	                                      NULL);

	if(result) {
		g_variant_iter_init (&iter, result);
		child = g_variant_iter_next_value (&iter);
	}

	return child;
}

/* Change any player propertie using org.freedesktop.DBus.Properties interfase. */

static void
mpris2_client_set_media_player_properties (Mpris2Client *mpris2, const gchar *prop, GVariant *vprop)
{
	GVariant *reply;
	GError   *error = NULL;

	reply = g_dbus_connection_call_sync (mpris2->gconnection,
	                                     mpris2->dbus_name,
	                                     "/org/mpris/MediaPlayer2",
	                                     "org.freedesktop.DBus.Properties",
	                                     "Set",
	                                     g_variant_new ("(ssv)",
	                                                    "org.mpris.MediaPlayer2",
	                                                    prop,
	                                                    vprop),
	                                     NULL,
	                                     G_DBUS_CALL_FLAGS_NONE,
	                                     -1,
	                                     NULL,
	                                     NULL);
	if (reply == NULL) {
		g_warning ("Unable to set session: %s", error->message);
		g_error_free (error);
		return;
	}
	g_variant_unref(reply);
}

/* Get all properties using org.freedesktop.DBus.Properties interface.  */

static GVariant *
mpris2_client_get_all_media_player_properties (Mpris2Client *mpris2)
{
	GVariantIter iter;
	GVariant *result, *child = NULL;

	result = g_dbus_connection_call_sync (mpris2->gconnection,
	                                      mpris2->dbus_name,
	                                      "/org/mpris/MediaPlayer2",
	                                      "org.freedesktop.DBus.Properties",
	                                      "GetAll",
	                                      g_variant_new ("(s)", "org.mpris.MediaPlayer2"),
	                                      G_VARIANT_TYPE ("(a{sv})"),
	                                      G_DBUS_CALL_FLAGS_NONE,
	                                      -1,
	                                      NULL,
	                                      NULL);

	if(result) {
		g_variant_iter_init (&iter, result);
		child = g_variant_iter_next_value (&iter);
	}

	return child;
}


/* Get any player propertie using org.freedesktop.DBus.Properties interfase. */

static GVariant *
mpris2_client_get_player_properties (Mpris2Client *mpris2, const gchar *prop)
{
	GVariant *v, *iter;
	GError *error = NULL;

	v = g_dbus_connection_call_sync (mpris2->gconnection,
	                                 mpris2->dbus_name,
	                                 "/org/mpris/MediaPlayer2",
	                                 "org.freedesktop.DBus.Properties",
	                                 "Get",
	                                  g_variant_new ("(ss)",
                                                     "org.mpris.MediaPlayer2.Player",
                                                     prop),
	                                 G_VARIANT_TYPE ("(v)"),
	                                 G_DBUS_CALL_FLAGS_NONE,
	                                 -1,
	                                 NULL,
	                                 &error);
	if (error) {
		g_critical ("Could not get properties on org.mpris.MediaPlayer2, %s",
		            error ? error->message : "no error given");
		g_clear_error (&error);
		return NULL;
	}

	g_variant_get (v, "(v)", &iter);

	return iter;
}

/* Change any player propertie using org.freedesktop.DBus.Properties interfase. */

static void
mpris2_client_set_player_properties (Mpris2Client *mpris2, const gchar *prop, GVariant *vprop)
{
	GVariant *reply;
	GError   *error = NULL;

	reply = g_dbus_connection_call_sync (mpris2->gconnection,
	                                     mpris2->dbus_name,
	                                     "/org/mpris/MediaPlayer2",
	                                     "org.freedesktop.DBus.Properties",
	                                     "Set",
	                                     g_variant_new ("(ssv)",
	                                                    "org.mpris.MediaPlayer2.Player",
	                                                    prop,
	                                                    vprop),
	                                     NULL,
	                                     G_DBUS_CALL_FLAGS_NONE,
	                                     -1,
	                                     NULL,
	                                     NULL);
	if (reply == NULL) {
		g_warning ("Unable to set session: %s", error->message);
		g_error_free (error);
		return;
	}
	g_variant_unref(reply);
}

/* These function intercepts the messages from the player. */

static const gchar *
g_avariant_get_string(GVariant * variant)
{
	const gchar **strv = NULL;
	const gchar *string = NULL;
	gsize len;

	strv = g_variant_get_strv (variant, &len);
	if (len > 0) {
		string = strv[0];
		g_free (strv);
	}
	else {
		string = "";
	}

	return string;
}

static Mpris2Metadata *
mpris2_metadata_new_from_variant (GVariant *dictionary)
{
	GVariantIter iter;
	GVariant *value;
	gchar *key;

	gint64 length = 0;

	Mpris2Metadata *metadata;

	metadata = mpris2_metadata_new ();

	g_variant_iter_init (&iter, dictionary);
	while (g_variant_iter_loop (&iter, "{sv}", &key, &value)) {
		if (0 == g_ascii_strcasecmp (key, "mpris:trackid"))
			mpris2_metadata_set_trackid (metadata, g_variant_get_string(value, NULL));
		else if (0 == g_ascii_strcasecmp (key, "xesam:url"))
			mpris2_metadata_set_url (metadata, g_variant_get_string(value, NULL));
		else if (0 == g_ascii_strcasecmp (key, "xesam:title"))
			mpris2_metadata_set_title (metadata, g_variant_get_string(value, NULL));
		else if (0 == g_ascii_strcasecmp (key, "xesam:artist"))
			mpris2_metadata_set_artist(metadata, g_avariant_get_string(value));
		else if (0 == g_ascii_strcasecmp (key, "xesam:album"))
			mpris2_metadata_set_album (metadata, g_variant_get_string(value, NULL));
		else if (0 == g_ascii_strcasecmp (key, "xesam:genre"));
			/* (List of Strings.) Not use genre */
		else if (0 == g_ascii_strcasecmp (key, "xesam:albumArtist"));
			// List of Strings.
		else if (0 == g_ascii_strcasecmp (key, "xesam:comment"));
			/* (List of Strings) Not use comment */
		else if (0 == g_ascii_strcasecmp (key, "xesam:audioBitrate"));
			/* (uint32) Not use audioBitrate */
		else if (0 == g_ascii_strcasecmp (key, "mpris:length"))
			length = g_variant_get_int64 (value);
		else if (0 == g_ascii_strcasecmp (key, "xesam:trackNumber"))
			mpris2_metadata_set_track_no(metadata, g_variant_get_int32 (value));
		else if (0 == g_ascii_strcasecmp (key, "xesam:useCount"));
			/* (Integer) Not use useCount */
		else if (0 == g_ascii_strcasecmp (key, "xesam:userRating"));
			/* (Float) Not use userRating */
		else if (0 == g_ascii_strcasecmp (key, "mpris:artUrl"))
			mpris2_metadata_set_arturl (metadata, g_variant_get_string(value, NULL));
		else if (0 == g_ascii_strcasecmp (key, "xesam:contentCreated"));
			/* has type 's' */
		else if (0 == g_ascii_strcasecmp (key, "audio-bitrate"));
			/* has type 'i' */
		else if (0 == g_ascii_strcasecmp (key, "audio-channels"));
			/* has type 'i' */
		else if (0 == g_ascii_strcasecmp (key, "audio-samplerate"));
			/* has type 'i' */
		else if (0 == g_ascii_strcasecmp (key, "xesam:contentCreated"));
			/* has type 's' */
		else if (0 == g_ascii_strcasecmp (key, "audio-bitrate"));
			/* has type 'i' */
		else if (0 == g_ascii_strcasecmp (key, "audio-channels"));
			/* has type 'i' */
		else if (0 == g_ascii_strcasecmp (key, "audio-samplerate"));
			/* has type 'i'*/
		else
			g_print ("Variant '%s' has type '%s'\n", key,
				     g_variant_get_type_string (value));
	}

	mpris2_metadata_set_length (metadata, length / 1000000l);

	return metadata;
}

static void
mpris2_client_parse_playback_status (Mpris2Client *mpris2, const gchar *playback_status)
{
	GVariant *value;

	if (0 == g_ascii_strcasecmp(playback_status, "Playing")) {
		mpris2->playback_status = PLAYING;
	}
	else if (0 == g_ascii_strcasecmp(playback_status, "Paused")) {
		mpris2->playback_status = PAUSED;
	}
	else {
		mpris2->playback_status = STOPPED;
	}

	if (mpris2->playback_status == PLAYING) {
		value = mpris2_client_get_player_properties (mpris2, "Position");
		mpris2->position = (gint) g_variant_get_int64 (value);

		if (mpris2->playback_timer_id == 0)
			mpris2->playback_timer_id = g_timeout_add_seconds (1, playback_tick_emit_cb, mpris2);
	}
	else {
		if (mpris2->playback_timer_id > 0) {
			g_source_remove (mpris2->playback_timer_id);
			mpris2->playback_timer_id = 0;
		}
	}
}

static void
mpris2_client_parse_player_properties (Mpris2Client *mpris2, GVariant *properties)
{
	GVariantIter iter;
	GVariant *value;
	const gchar *key;
	const gchar *playback_status = NULL;
	const gchar *loop_status = NULL;
	Mpris2Metadata *metadata = NULL;
	gdouble volume = -1;
	gboolean shuffle = FALSE;
	gboolean loop_status_changed = FALSE;
	gboolean shuffle_changed = FALSE;

	g_variant_iter_init (&iter, properties);

	while (g_variant_iter_loop (&iter, "{sv}", &key, &value)) {
		if (0 == g_ascii_strcasecmp (key, "PlaybackStatus")) {
			playback_status = g_variant_get_string(value, NULL);
		}
		else if (0 == g_ascii_strcasecmp (key, "Rate")) {
			mpris2->rate = g_variant_get_double(value);
		}
		else if (0 == g_ascii_strcasecmp (key, "Metadata")) {
			metadata = mpris2_metadata_new_from_variant (value);
		}
		else if (0 == g_ascii_strcasecmp (key, "Volume")) {
			volume = g_variant_get_double(value);
		}
		else if (0 == g_ascii_strcasecmp (key, "MinimumRate")) {
			mpris2->minimum_rate = g_variant_get_double(value);
		}
		else if (0 == g_ascii_strcasecmp (key, "MaximumRate")) {
			mpris2->maximum_rate = g_variant_get_double(value);
		}
		else if (0 == g_ascii_strcasecmp (key, "CanGoNext")) {
			mpris2->can_go_next = g_variant_get_boolean(value);
		}
		else if (0 == g_ascii_strcasecmp (key, "CanGoPrevious")) {
			mpris2->can_go_previous = g_variant_get_boolean(value);
		}
		else if (0 == g_ascii_strcasecmp (key, "CanPlay")) {
			mpris2->can_play = g_variant_get_boolean(value);
		}
		else if (0 == g_ascii_strcasecmp (key, "CanPause")) {
			mpris2->can_pause = g_variant_get_boolean(value);
		}
		else if (0 == g_ascii_strcasecmp (key, "CanSeek")) {
			mpris2->can_seek = g_variant_get_boolean(value);
		}
		else if (0 == g_ascii_strcasecmp (key, "CanControl")) {
			mpris2->can_control = g_variant_get_boolean(value);
		}
		/* Optionals */
		else if (0 == g_ascii_strcasecmp (key, "LoopStatus")) {
			loop_status_changed = TRUE;
			loop_status = g_variant_get_string(value, NULL);
		}
		else if (0 == g_ascii_strcasecmp (key, "Shuffle")) {
			shuffle_changed = TRUE;
			shuffle = g_variant_get_boolean(value);
		}
	}

	if (metadata != NULL) {
		if (mpris2->metadata != NULL)
			mpris2_metadata_free (mpris2->metadata);
		mpris2->metadata = metadata;

		g_signal_emit (mpris2, signals[METADATA], 0, metadata);
	}

	if (playback_status != NULL) {
		mpris2_client_parse_playback_status (mpris2, playback_status);
		g_signal_emit (mpris2, signals[PLAYBACK_STATUS], 0, mpris2->playback_status);
	}

	if (volume != -1) {
		mpris2->volume = volume;
		g_signal_emit (mpris2, signals[VOLUME], 0, volume);
	}

	if (loop_status_changed) {
		mpris2->has_loop_status = TRUE;

		if (0 == g_ascii_strcasecmp(loop_status, "Track")) {
			mpris2->loop_status = TRACK;
		}
		else if (0 == g_ascii_strcasecmp(loop_status, "Playlist")) {
			mpris2->loop_status = PLAYLIST;
		}
		else {
			mpris2->loop_status = NONE;
		}
		g_signal_emit (mpris2, signals[LOOP_STATUS], 0, mpris2->loop_status);
	}
	if (shuffle_changed) {
		mpris2->has_shuffle = TRUE;

		mpris2->shuffle = shuffle;
		g_signal_emit (mpris2, signals[SHUFFLE], 0, shuffle);
	}
}

static void
mpris2_client_parse_media_player_properties (Mpris2Client *mpris2, GVariant *properties)
{
	GVariantIter iter;
	GVariant *value;
	const gchar *key;

	g_variant_iter_init (&iter, properties);

	while (g_variant_iter_loop (&iter, "{sv}", &key, &value)) {
		if (0 == g_ascii_strcasecmp (key, "CanQuit")) {
			mpris2->can_quit = g_variant_get_boolean (value);
		}
		else if (0 == g_ascii_strcasecmp (key, "Fullscreen")) {
			mpris2->fullscreen = g_variant_get_boolean (value);
		}
		else if (0 == g_ascii_strcasecmp (key, "CanSetFullscreen")) {
			mpris2->can_set_fullscreen = g_variant_get_boolean (value);
		}
		else if (0 == g_ascii_strcasecmp (key, "CanRaise")) {
			mpris2->can_raise = g_variant_get_boolean (value);
		}
		else if (0 == g_ascii_strcasecmp (key, "HasTrackList")) {
			mpris2->has_tracklist = g_variant_get_boolean (value);
		}
		else if (0 == g_ascii_strcasecmp (key, "Identity")) {
			if (mpris2->identity)
				g_free (mpris2->identity);
			mpris2->identity = g_variant_dup_string (value, NULL);
		}
		else if (0 == g_ascii_strcasecmp (key, "DesktopEntry")) {
			if (mpris2->desktop_entry)
				g_free (mpris2->desktop_entry);
			mpris2->desktop_entry = g_variant_dup_string (value, NULL);
		}
		else if (0 == g_ascii_strcasecmp (key, "SupportedUriSchemes")) {
			if (mpris2->supported_uri_schemes)
				g_strfreev (mpris2->supported_uri_schemes);
			mpris2->supported_uri_schemes = g_variant_dup_strv (value, NULL);
		}
		else if (0 == g_ascii_strcasecmp (key, "SupportedMimeTypes")) {
			if (mpris2->supported_mime_types)
				g_strfreev (mpris2->supported_mime_types);
			mpris2->supported_mime_types = g_variant_dup_strv (value, NULL);
		}
	}
}

static void
mpris2_client_on_dbus_props_signal (GDBusProxy *proxy,
                                    gchar      *sender_name,
                                    gchar      *signal_name,
                                    GVariant   *parameters,
                                    gpointer    user_data)
{
	GVariantIter iter;
	GVariant *child;

	Mpris2Client *mpris2 = user_data;

	if (g_ascii_strcasecmp (signal_name, "PropertiesChanged"))
		return;

	g_variant_iter_init (&iter, parameters);

	child = g_variant_iter_next_value (&iter); /* Interface name. */
	g_variant_unref (child);

	child = g_variant_iter_next_value (&iter); /* Property name. */
	mpris2_client_parse_player_properties (mpris2, child);
	g_variant_unref (child);
}

static void
mpris2_client_on_dbus_player_signal (GDBusProxy *proxy,
                                     gchar      *sender_name,
                                     gchar      *signal_name,
                                     GVariant   *parameters,
                                     gpointer    user_data)
{
	GVariantIter iter;
	GVariant *child;

	Mpris2Client *mpris2 = user_data;

	if (g_ascii_strcasecmp (signal_name, "Seeked"))
		return;

	g_variant_iter_init (&iter, parameters);

	child = g_variant_iter_next_value (&iter);

	mpris2->position = g_variant_get_int64 (child);
	g_signal_emit (mpris2, signals[PLAYBACK_TICK], 0, mpris2->position);

	g_variant_unref (child);
}

/* Functions that detect when the player is connected to mpris2 */

static void
mpris2_client_connected_dbus (GDBusConnection *connection,
                              const gchar *name,
                              const gchar *name_owner,
                              gpointer user_data)
{
	GVariant *reply;

	Mpris2Client *mpris2 = user_data;

	mpris2->connected = TRUE;

	/* First check basic props of the player as identify, uris, etc. */
	reply = mpris2_client_get_all_media_player_properties (mpris2);
	mpris2_client_parse_media_player_properties (mpris2, reply);
	g_variant_unref (reply);

	/* Notify that connect to a player.*/
	g_signal_emit (mpris2, signals[CONNECTION], 0, mpris2->connected);

	/* And informs the current status of the player */
	reply = mpris2_client_get_all_player_properties (mpris2);
	mpris2_client_parse_player_properties (mpris2, reply);
	g_variant_unref (reply);
}

static void
mpris2_client_lose_dbus (GDBusConnection *connection,
                         const gchar *name,
                         gpointer user_data)
{
	Mpris2Client *mpris2 = user_data;

	/* Interface MediaPlayer2 */

	mpris2->can_quit        = FALSE;
	mpris2->can_raise       = FALSE;
	mpris2->has_tracklist   = FALSE;
	if (mpris2->identity) {
		g_free (mpris2->identity);
		mpris2->identity = NULL;
	}
	if (mpris2->supported_uri_schemes) {
		g_strfreev(mpris2->supported_uri_schemes);
		mpris2->supported_uri_schemes = NULL;
	}
	if (mpris2->supported_mime_types) {
		g_strfreev(mpris2->supported_mime_types);
		mpris2->supported_mime_types = NULL;
	}

	/* Optionals Interface MediaPlayer2 */
	mpris2->can_set_fullscreen = FALSE;
	if (mpris2->desktop_entry) {
		g_free (mpris2->desktop_entry);
		mpris2->desktop_entry = NULL;
	}

	/* Interface MediaPlayer2.Player */
	mpris2->playback_status = STOPPED;
	mpris2->rate            = 1.0;
	if (mpris2->metadata != NULL) {
		mpris2_metadata_free (mpris2->metadata);
		mpris2->metadata = NULL;
	}
	mpris2->volume          = -1;
	mpris2->position        = 0;
	mpris2->minimum_rate    = 1.0;
	mpris2->maximum_rate    = 1.0;
	mpris2->can_go_next     = FALSE;
	mpris2->can_go_previous = FALSE;
	mpris2->can_play        = FALSE;
	mpris2->can_pause       = FALSE;
	mpris2->can_seek        = FALSE;
	mpris2->can_control     = FALSE;

	/* Optionals Interface MediaPlayer2.Player */
	mpris2->has_loop_status = FALSE;
	mpris2->has_shuffle     = FALSE;

	mpris2->connected = FALSE;
	g_signal_emit (mpris2, signals[CONNECTION], 0, mpris2->connected);
}

static void
mpris2_client_connect_dbus (Mpris2Client *mpris2)
{
	GDBusProxy *proxy;
	GError     *gerror = NULL;
	guint       watch_id;

	if (mpris2->player == NULL)
		return;

	g_free(mpris2->dbus_name);
	mpris2->dbus_name = g_strdup_printf("org.mpris.MediaPlayer2.%s", mpris2->player);

	watch_id = g_bus_watch_name_on_connection(mpris2->gconnection,
	                                          mpris2->dbus_name,
	                                          G_BUS_NAME_OWNER_FLAGS_REPLACE,
	                                          mpris2_client_connected_dbus,
	                                          mpris2_client_lose_dbus,
	                                          mpris2,
	                                          NULL);

	/* interface=org.freedesktop.DBus.Properties */
	proxy = g_dbus_proxy_new_sync (mpris2->gconnection,
	                               G_DBUS_PROXY_FLAGS_NONE,
	                               NULL,
	                               mpris2->dbus_name,
	                               "/org/mpris/MediaPlayer2",
	                               "org.freedesktop.DBus.Properties",
	                               NULL, /* GCancellable */
	                               &gerror);

	if (proxy == NULL) {
		g_printerr ("Error creating proxy: %s\n", gerror->message);
		g_error_free (gerror);
		gerror = NULL;
    }
    else {
		g_signal_connect (proxy, "g-signal",
			              G_CALLBACK (mpris2_client_on_dbus_props_signal), mpris2);
		mpris2->props_proxy = proxy;
	}

	/* interface=org.mpris.MediaPlayer2.Player */
	proxy = g_dbus_proxy_new_sync (mpris2->gconnection,
	                               G_DBUS_PROXY_FLAGS_NONE,
	                               NULL,
	                               mpris2->dbus_name,
	                               "/org/mpris/MediaPlayer2",
	                               "org.mpris.MediaPlayer2.Player",
	                               NULL, /* GCancellable */
	                               &gerror);

	if (proxy == NULL) {
		g_printerr ("Error creating proxy: %s\n", gerror->message);
		g_error_free (gerror);
		gerror = NULL;
    }
    else {
		g_signal_connect (proxy, "g-signal",
			              G_CALLBACK (mpris2_client_on_dbus_player_signal), mpris2);
		mpris2->player_proxy = proxy;
	}

	mpris2->watch_id = watch_id;
}

static void
mpris2_client_finalize (GObject *object)
{
	Mpris2Client *mpris2 = MPRIS2_CLIENT (object);

	if (mpris2->player != NULL) {
		g_free (mpris2->player);
		mpris2->player = NULL;
	}
	if (mpris2->dbus_name) {
		g_free (mpris2->dbus_name);
		mpris2->dbus_name = NULL;
	}

	if (mpris2->identity) {
		g_free (mpris2->identity);
		mpris2->identity    = NULL;
	}
	if (mpris2->desktop_entry) {
		g_free (mpris2->desktop_entry);
		mpris2->desktop_entry = NULL;
	}
	if (mpris2->supported_uri_schemes) {
		g_strfreev(mpris2->supported_uri_schemes);
		mpris2->supported_uri_schemes = NULL;
	}
	if (mpris2->supported_mime_types) {
		g_strfreev(mpris2->supported_mime_types);
		mpris2->supported_mime_types = NULL;
	}

	if (mpris2->metadata != NULL) {
		mpris2_metadata_free (mpris2->metadata);
		mpris2->metadata = NULL;
	}

	(*G_OBJECT_CLASS (mpris2_client_parent_class)->finalize) (object);
}


static void
mpris2_client_class_init (Mpris2ClientClass *klass)
{
	GObjectClass  *gobject_class;

	gobject_class = G_OBJECT_CLASS (klass);
	gobject_class->finalize = mpris2_client_finalize;

	/**
	 * Mpris2Client::connection:
	 * @client: the object which received the signal
	 * @connection: the new connection to mpris2 interface
	 *
	 * The ::connection signal is emitted each time that connecction changed.
	 */
	signals[CONNECTION] =
		g_signal_new ("connection",
	                  G_TYPE_FROM_CLASS (gobject_class),
	                  G_SIGNAL_RUN_LAST,
	                  G_STRUCT_OFFSET (Mpris2ClientClass, connection),
	                  NULL, NULL,
                      g_cclosure_marshal_VOID__BOOLEAN,
                      G_TYPE_NONE, 1, G_TYPE_BOOLEAN);

	signals[PLAYBACK_STATUS] =
		g_signal_new ("playback-status",
		              G_TYPE_FROM_CLASS (gobject_class),
		              G_SIGNAL_RUN_LAST,
		              G_STRUCT_OFFSET (Mpris2ClientClass, playback_status),
		              NULL, NULL,
	                  g_cclosure_marshal_VOID__ENUM,
	                  G_TYPE_NONE, 1, G_TYPE_INT);

	signals[PLAYBACK_TICK] =
		g_signal_new ("playback-tick",
		              G_TYPE_FROM_CLASS (gobject_class),
		              G_SIGNAL_RUN_LAST,
		              G_STRUCT_OFFSET (Mpris2ClientClass, playback_tick),
		              NULL, NULL,
	                  g_cclosure_marshal_VOID__INT,
	                  G_TYPE_NONE, 1, G_TYPE_INT);

	signals[METADATA] =
		g_signal_new ("metadata",
		              G_TYPE_FROM_CLASS (gobject_class),
		              G_SIGNAL_RUN_LAST,
		              G_STRUCT_OFFSET (Mpris2ClientClass, metadata),
		              NULL, NULL,
		              g_cclosure_marshal_VOID__POINTER,
		              G_TYPE_NONE, 1, G_TYPE_POINTER);

	signals[VOLUME] =
		g_signal_new ("volume",
		              G_TYPE_FROM_CLASS (gobject_class),
		              G_SIGNAL_RUN_LAST,
		              G_STRUCT_OFFSET (Mpris2ClientClass, volume),
		              NULL, NULL,
	                  g_cclosure_marshal_VOID__DOUBLE,
	                  G_TYPE_NONE, 1, G_TYPE_DOUBLE);

	signals[LOOP_STATUS] =
		g_signal_new ("loop-status",
		              G_TYPE_FROM_CLASS (gobject_class),
		              G_SIGNAL_RUN_LAST,
		              G_STRUCT_OFFSET (Mpris2ClientClass, loop_status),
		              NULL, NULL,
	                  g_cclosure_marshal_VOID__ENUM,
	                  G_TYPE_NONE, 1, G_TYPE_INT);

	signals[SHUFFLE] =
		g_signal_new ("shuffle",
		              G_TYPE_FROM_CLASS (gobject_class),
		              G_SIGNAL_RUN_LAST,
		              G_STRUCT_OFFSET (Mpris2ClientClass, shuffle),
		              NULL, NULL,
	                  g_cclosure_marshal_VOID__BOOLEAN,
	                  G_TYPE_NONE, 1, G_TYPE_BOOLEAN);
}

static void
mpris2_client_init (Mpris2Client *mpris2)
{
	GDBusConnection *gconnection;
	GError          *gerror = NULL;

	gconnection = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, &gerror);
	if (gconnection == NULL) {
		g_message ("Failed to get session bus: %s", gerror->message);
		g_error_free (gerror);
		gerror = NULL;
	}

	mpris2->gconnection           = gconnection;
	mpris2->props_proxy           = NULL;
	mpris2->player_proxy          = NULL;
	mpris2->dbus_name             = NULL;
	mpris2->watch_id              = 0;

	mpris2->player                = NULL;

	mpris2->can_quit              = FALSE;
	mpris2->can_set_fullscreen    = FALSE;
	mpris2->can_raise             = FALSE;
	mpris2->has_tracklist         = FALSE;
	mpris2->identity              = NULL;
	mpris2->desktop_entry         = NULL;
	mpris2->supported_uri_schemes = NULL;
	mpris2->supported_mime_types  = NULL;

	mpris2->playback_status       = STOPPED;
	mpris2->rate                  = 1.0;
	mpris2->metadata              = NULL;
	mpris2->volume                = -1;
	mpris2->position              = 0;
	mpris2->minimum_rate          = 1.0;
	mpris2->maximum_rate          = 1.0;
	mpris2->can_go_next           = FALSE;
	mpris2->can_go_previous       = FALSE;
	mpris2->can_play              = FALSE;
	mpris2->can_pause             = FALSE;
	mpris2->can_seek              = FALSE;
	mpris2->can_control           = FALSE;

	mpris2->has_loop_status       = FALSE;
	mpris2->loop_status           = FALSE;
	mpris2->has_shuffle           = FALSE;
	mpris2->shuffle               = FALSE;

	mpris2->connected             = FALSE;
	mpris2->strict_mode           = FALSE;

	mpris2_client_connect_dbus (mpris2);
}