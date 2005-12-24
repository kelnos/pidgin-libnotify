gddir = $(GAIM_LIBDIR)/gaim

gaim_libnotify_la_LDFLAGS = -module -avoid-version

if PLUGINS

gd_LTLIBRARIES = gaim-libnotify.la

gaim_libnotify_la_SOURCES = \
	gaim-libnotify.c \
	gln_intl.h

gaim_libnotify_la_LIBADD = $(LIBNOTIFY_LIBS) $(DBUS_LIBS) $(GTK_LIBS)

endif

AM_CPPFLAGS = \
	-DLIBDIR=\"$(GAIM_LIBDIR)/gaim/\" \
	-DDATADIR=\"$(GAIM_DATADIR)\" \
	-DLOCALEDIR=\"$(GAIM_DATADIR)/locale\" \
	$(DEBUG_CFLAGS) \
	$(GTK_CFLAGS) \
	$(GLIB_CFLAGS) \
	$(GAIM_CFLAGS) \
	$(LIBNOTIFY_CFLAGS) \
	$(DBUS_CFLAGS) \
	$(GTK_CFLAGS)

