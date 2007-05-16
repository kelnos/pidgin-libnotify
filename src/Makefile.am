gddir = $(LIBPURPLE_LIBDIR)/purple-2

gaim_libnotify_la_LDFLAGS = -module -avoid-version

if PLUGINS

gd_LTLIBRARIES = gaim-libnotify.la

gaim_libnotify_la_SOURCES = \
	gaim-libnotify.c \
	gln_intl.h

gaim_libnotify_la_LIBADD = $(LIBNOTIFY_LIBS) $(DBUS_LIBS) $(GTK_LIBS)

endif

AM_CPPFLAGS = \
	-DLIBDIR=\"$(LIBPURPLE_LIBDIR)/purple-2/\" \
	-DDATADIR=\"$(LIBPURPLE_DATADIR)\" \
	-DLOCALEDIR=\"$(LIBPURPLE_DATADIR)/locale\" \
	$(DEBUG_CFLAGS) \
	$(GTK_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(PIDGIN_CFLAGS) \
	$(LIBPURPLE_CFLAGS) \
	$(LIBNOTIFY_CFLAGS) \
	$(DBUS_CFLAGS) \
	$(GTK_CFLAGS)

