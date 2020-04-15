# pidgin-libnotify

## About

pidgin-libnotify displays Linux system notifications for events such as
receiving a message or a buddy signing on or off.

This is a (minimal) fork of the original pidgin-libnotify ([hosted at
SourceForge](https://sourceforge.net/projects/gaim-libnotify/)) that
updates the plugin for a modern version of libnotify and applies some
downstream patches.

## Installation

You'll need autoconf, automake, libtool, intltool, make, a C compiler,
and the development headers for libnotify installed.  Then run (replace
`$PIDGIN_PREFIX` with the prefix used to install Pidgin itself, usually
`/usr`):

```
./autogen.sh --prefix=$PIDGIN_PREFIX
make
make install
```
