#!/bin/sh

set -e

check_installed() {
    if ! $1 --version </dev/null >/dev/null 2>&1; then
        echo
        echo "You must have $1 installed to compile pidgin-libnotify"
        echo
        exit 1
    fi
}

[ "$GLIB_GETTEXTIZE" ] || GLIB_GETTEXTIZE=glib-gettextize
[ "$AUTORECONF" ] || AUTORECONF=autoreconf

check_installed $GLIB_GETTEXTIZE
check_installed $AUTORECONF

echo "Generating configuration files for pidgin-libnotify, please wait...."
echo;

# Backup po/ChangeLog because gettext likes to change it
cp -p po/ChangeLog po/ChangeLog.save
echo "Running gettextize, please ignore non-fatal messages...."
$GLIB_GETTEXTIZE --force --copy
#restore pl/ChangeLog
mv po/ChangeLog.save po/ChangeLog

$AUTORECONF -fiv

echo "Running ./configure $@"
echo;
./configure $@
