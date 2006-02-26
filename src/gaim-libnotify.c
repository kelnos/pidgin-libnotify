/*
 * Gaim-libnotify - Provides a libnotify interface for Gaim
 * Copyright (C) 2005 Duarte Henriques
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gln_intl.h"

#ifndef GAIM_PLUGINS
#define GAIM_PLUGINS
#endif

#include "gaim.h"
#include "version.h"
#include "debug.h"
#include "util.h"

/* TODO: file a bug with gaim and remove this */
#include "gtkconv.h"

#include <libnotify/notify.h>

#include <string.h>

#define PLUGIN_ID "gaim-libnotify"

static GHashTable *buddy_hash;

/* Imported from gtkblist.c +4928, do not touch!
 * bug filed, this should be core API
 * Note: code that calls this must g_free() the uri */
static gchar *
get_prpl_icon_uri (GaimAccount *account)
{
	GaimPlugin *prpl;
	GaimPluginProtocolInfo *prpl_info = NULL;
	const char *protoname = NULL;
	char buf[256];

	g_return_val_if_fail (account != NULL, NULL);

	prpl = gaim_find_prpl (gaim_account_get_protocol_id(account));

	if (prpl != NULL) {
		prpl_info = GAIM_PLUGIN_PROTOCOL_INFO(prpl);

		if (prpl_info->list_icon != NULL)
			protoname = prpl_info->list_icon(account, NULL);
	}

	if (protoname == NULL)
		return NULL;

	g_snprintf(buf, sizeof(buf), "%s.png", protoname);

	return g_build_filename(DATADIR, "pixmaps", "gaim", "status",
								"default", buf, NULL);
}

/* Signon flood be gone! - thanks to the guifications devs */
static GList *just_signed_on_accounts = NULL;

static gboolean
event_connection_throttle_cb (gpointer data)
{
	GaimAccount *account;

	account = (GaimAccount *)data;

	if (!account)
		return FALSE;

	if (!gaim_account_get_connection (account)) {
		just_signed_on_accounts = g_list_remove (just_signed_on_accounts, account);
		return FALSE;
	}

	if (!gaim_account_is_connected (account))
		return TRUE;

	just_signed_on_accounts = g_list_remove (just_signed_on_accounts, account);
	return FALSE;
}

static void
event_connection_throttle (GaimConnection *gc, gpointer data)
{
	GaimAccount *account;

	if (!gc)
		return;

	account = gaim_connection_get_account(gc);
	if (!account)
		return;

	just_signed_on_accounts = g_list_prepend (just_signed_on_accounts, account);
	g_timeout_add (5000, event_connection_throttle_cb, (gpointer)account);
}

static gchar *
best_name (GaimBuddy *buddy)
{
	if (buddy->alias) {
		return buddy->alias;
	} else if (buddy->server_alias) {
		return buddy->server_alias;
	} else {
		return buddy->name;
	}
}

static GdkPixbuf *
best_icon (GaimBuddy *buddy)
{
	GdkPixbuf *icon;
#if 0
	GaimBuddyIcon *buddy_icon;

	buddy_icon = gaim_buddy_icons_find (buddy->account, buddy->name);
	if (buddy_icon) {
		const guchar *data;
		size_t len;
		GdkPixbufLoader *loader;
		const char *type;
		
		/*g_signal_connect (loader, "area-prepared", G_CALLBACK(whatever));*/
		
		type = gaim_buddy_icon_get_type (buddy_icon);
		g_print ("BUDDYICON 1\n");
		data = gaim_buddy_icon_get_data (buddy_icon, &len);
		g_print ("BUDDYICON 2\n");
		loader = gdk_pixbuf_loader_new_with_type (type, NULL);
		g_print ("BUDDYICON 3\n");
		gdk_pixbuf_loader_write (loader, data, len, NULL);
		g_print ("BUDDYICON 4\n");
		gdk_pixbuf_loader_close (loader, NULL);
		g_print ("BUDDYICON 5\n");
		icon = gdk_pixbuf_loader_get_pixbuf (loader);
		g_print ("BUDDYICON 6\n");
		g_object_ref (icon);
		g_print ("BUDDYICON 7\n");
		g_free (loader);
		/*icon = gdk_pixbuf_new_from_data (data, cs, TRUE, 24, 96, 96, rs);*/
	} else {
		gchar *icon_uri;
		icon_uri = get_prpl_icon_uri (buddy->account);
		if (icon_uri) {
			icon = gdk_pixbuf_new_from_file (icon_uri, NULL);
		} else {
			icon = NULL;
		}
		g_free (icon_uri);
	}
#endif

	gchar *icon_uri;
	icon_uri = g_build_filename (DATADIR, "pixmaps", "gaim", "status",
								"default", "msn.png", NULL);
	icon = gdk_pixbuf_new_from_file (icon_uri, NULL);

	return icon;
}

static void
action_cb (NotifyNotification *notification,
		   gchar *action, gpointer user_data)
{
	GaimBuddy *buddy = NULL;
	GaimConversation *conv = NULL;
	/*GaimConvWindow *win = NULL;*/

	gaim_debug_info (PLUGIN_ID, "action_cb(), "
					"notification: 0x%x, action: '%s'", notification, action);

#if 0 /* old libnotify */
	buddy = (GaimBuddy *)notify_notification_get_user_data (notification);
#else
	buddy = (GaimBuddy *)user_data;
#endif

	if (!buddy) {
		gaim_debug_warning (PLUGIN_ID, "Got no buddy!");
		return;
	}

	conv = gaim_find_conversation_with_account (GAIM_CONV_TYPE_ANY, buddy->name, buddy->account);

	if (!conv) {
		conv = gaim_conversation_new (GAIM_CONV_TYPE_IM,
									  buddy->account,
									  buddy->name);
	}
	conv->ui_ops->present(conv);
#if 0
	win = gaim_conversation_get_window (conv);

	if (win) {
		gaim_conv_window_raise (win);
		gaim_conv_window_switch_conversation (win, gaim_conversation_get_index(conv));

		if (GAIM_IS_GTK_WINDOW (win))
			gtk_window_present (GTK_WINDOW (GAIM_GTK_WINDOW (win)->window));
	} else {
		gaim_debug_warning (PLUGIN_ID, "No window found for conversation!");
		return;
	}
#endif

	notify_notification_close (notification, NULL);
}

static gboolean
closed_cb (NotifyNotification *notification)
{
	/*GaimBuddy *buddy;*/

	gaim_debug_info (PLUGIN_ID, "closed_cb(), notification: 0x%x\n", notification);

	/* BUGGY notification
	buddy = (GaimBuddy *)notify_notification_get_user_data (notification);
	g_hash_table_remove (buddy_hash, buddy);
	*/
	g_object_unref(G_OBJECT(notification));

	return FALSE;
}

static void
notify (const gchar *title,
		const gchar *body,
		GaimBuddy *buddy)
{
	NotifyNotification *notification = NULL;
	GdkPixbuf *icon;gchar *icon_uri;
	gchar *text;
	/*GError **error;*/

	if (strlen (body) > 60) {
		gchar *str;
		str = g_strndup (body, 58);
		text = g_strdup_printf ("%s..", str);
		g_free (str);
	} else {
		text = g_strdup (body);
	}

	/* BUGGY notification
	notification = g_hash_table_lookup (buddy_hash, buddy);
	
	if (notification != NULL) {
		notify_notification_update (notification, title, body, NULL);
		return;
	}
	*/

	icon_uri = get_prpl_icon_uri (buddy->account);

	gaim_debug_info (PLUGIN_ID, "notify(), "
					 "title: '%s', body: '%s', buddy: '%s', icon_uri: '%s'\n",
					 title, text, best_name(buddy), icon_uri);

	notification = notify_notification_new (title, text, icon_uri, NULL);
	g_free (text);
	if (icon_uri)
		g_free (icon_uri);

	g_hash_table_insert (buddy_hash, buddy, notification);

	/* BUGGY notification
	notify_notification_set_user_data (notification, &buddy, NULL);
	*/

	g_signal_connect (notification, "closed", G_CALLBACK(closed_cb), NULL);

	icon = best_icon (buddy);
	notify_notification_set_icon_from_pixbuf (notification, icon);
	g_object_unref (icon);

	notify_notification_set_urgency (notification, NOTIFY_URGENCY_NORMAL);

	notify_notification_add_action (notification, "show", _("Show"), action_cb, buddy, NULL);

	if (!notify_notification_show (notification, NULL)) {
		gaim_debug_error (PLUGIN_ID, "notify(), failed to send notification\n");
	}

}

static void
notify_event_buddy_cb (GaimBuddy *buddy, gpointer data)
{
	gchar *title;

	g_return_if_fail (buddy);

	if (g_list_find (just_signed_on_accounts, buddy->account))
		return;

	title = g_strdup_printf (_("%s signed on"), best_name (buddy));

	notify (title, "", buddy);

	g_free (title);
}

static void
notify_msg_sent (GaimAccount *account,
				 const gchar *sender,
				 const gchar *message)
{
	GaimBuddy *buddy;
	gchar *title, *body, *name;

	buddy = gaim_find_buddy (account, sender);
	if (!buddy)
		return;

	name = best_name (buddy);

	title = g_strdup_printf (_("%s says:"), name);
	body = gaim_markup_strip_html (message);

	notify (title, body, buddy);

	g_free (title);
	g_free (body);
}

static void
notify_new_message_cb (GaimAccount *account,
					   const gchar *sender,
					   const gchar *message,
					   int flags,
					   gpointer data)
{
	GaimConversation *conv;

	conv = gaim_find_conversation_with_account (GAIM_CONV_TYPE_IM, sender, account);

	/* XXX commented for debug */
	/*if (conv && gaim_conversation_has_focus (conv))
		return;*/

	notify_msg_sent (account, sender, message);
}

static void
notify_chat_nick (GaimAccount *account,
				  const gchar *sender,
				  const gchar *message,
				  GaimConversation *conv,
				  gpointer data)
{
	gchar *nick;

	nick = (gchar *)gaim_conv_chat_get_nick (GAIM_CONV_CHAT(conv));
	if (nick && !strcmp (sender, nick))
		return;

	if (!g_strstr_len (message, strlen(message), nick))
		return;

	notify_msg_sent (account, sender, message);
}

void *gaim_dbus_get_handle(void);
static gboolean
plugin_load (GaimPlugin *plugin)
{
	void *conv_handle, *blist_handle, *conn_handle;

	if (!notify_is_initted () && !notify_init ("Gaim")) {
		gaim_debug_error (PLUGIN_ID, "libnotify not running!\n");
		return FALSE;
	}

	conv_handle = gaim_conversations_get_handle ();
	blist_handle = gaim_blist_get_handle ();
	conn_handle = gaim_connections_get_handle();

	buddy_hash = g_hash_table_new (NULL, NULL);

	gaim_signal_connect (blist_handle, "buddy-signed-on", plugin,
						GAIM_CALLBACK(notify_event_buddy_cb), NULL);
	
	gaim_signal_connect (conv_handle, "received-im-msg", plugin,
						GAIM_CALLBACK(notify_new_message_cb), NULL);

	gaim_signal_connect (conv_handle, "received-chat-msg", plugin,
						GAIM_CALLBACK(notify_chat_nick), NULL);

	/* used just to not display the flood of guifications we'd get */
	gaim_signal_connect (conn_handle, "signed-on", plugin,
						GAIM_CALLBACK(event_connection_throttle), NULL);

	return TRUE;
}

static gboolean
plugin_unload (GaimPlugin *plugin)
{
	void *conv_handle, *blist_handle, *conn_handle;

	conv_handle = gaim_conversations_get_handle ();
	blist_handle = gaim_blist_get_handle ();
	conn_handle = gaim_connections_get_handle();

	gaim_signal_disconnect (blist_handle, "buddy-signed-on", plugin,
							GAIM_CALLBACK(notify_event_buddy_cb));

	gaim_signal_disconnect (conv_handle, "received-im-msg", plugin,
							GAIM_CALLBACK(notify_new_message_cb));

	gaim_signal_disconnect (conv_handle, "received-chat-msg", plugin,
							GAIM_CALLBACK(notify_chat_nick));

	gaim_signal_disconnect (conn_handle, "signed-on", plugin,
						GAIM_CALLBACK(event_connection_throttle));

	g_hash_table_destroy (buddy_hash);

	notify_uninit ();

	return TRUE;
}

static GaimPluginInfo info = {
    GAIM_PLUGIN_MAGIC,										/* api version */
    GAIM_MAJOR_VERSION,
    GAIM_MINOR_VERSION,
    GAIM_PLUGIN_STANDARD,									/* type */
    NULL,													/* ui requirement */
    0,														/* flags */
    NULL,													/* dependencies */
    GAIM_PRIORITY_DEFAULT,									/* priority */
    
    PLUGIN_ID,												/* id */
    NULL,													/* name */
    VERSION,												/* version */
    NULL,													/* summary */
    NULL,													/* description */
    
    "Duarte Henriques <duarte.henriques@gmail.com>",		/* author */
    "http://sf.net/projects/gaim-libnotify/",				/* homepage */
    
    plugin_load,			/* load */
    plugin_unload,			/* unload */
    NULL,					/* destroy */
    NULL,					/* ui info */
    NULL,					/* extra info */
    NULL					/* actions info */
};

static void
init_plugin (GaimPlugin *plugin)
{
	bindtextdomain (PACKAGE, LOCALEDIR);
	bind_textdomain_codeset (PACKAGE, "UTF-8");

	info.name = _("Libnotify Interface");
	info.summary = _("Displays popups via libnotify.");
	info.description = _("Displays popups via libnotify.");
}

GAIM_INIT_PLUGIN(notify, init_plugin, info)

