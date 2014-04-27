AM_CPPFLAGS = \
	-DG_LOG_DOMAIN=\"mpris2-status-icon\" \
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\" \
	-DBASEICONDIR=\"$(datadir)/icons/hicolor\" \
	$(PLATFORM_CPPFLAGS)

#
# Status Icon
#
bin_PROGRAMS = mpris2-status-icon

mpris2_status_icon_SOURCES = 	\
	mpris2-album-art.c 			\
	mpris2-album-art.h			\
	mpris2-status-icon.c

mpris2_status_icon_CFLAGS =		\
	$(GIO_CFLAGS)				\
	$(GTK_CFLAGS)

mpris2_status_icon_LDADD =		\
	$(GIO_LIBS)					\
	$(GTK_LIBS)					\
	../src/libmpris2client.la

iconsdir = $(datadir)/icons/hicolor/128x128/apps
icons_DATA = mpris2-status-icon.png

EXTRA_DIST = $(icons_DATA)

#
# Update icon cache
#
gtk_update_icon_cache = gtk-update-icon-cache -f -t $(datadir)/icons/hicolor

update-icon-cache:
	@-if test -z "$(DESTDIR)"; then	\
                echo "Updating Gtk icon cache."; \
                $(gtk_update_icon_cache); \
        else \
                echo "*** Icon cache not updated. Remember to run:"; \
		echo "***"; \
                echo "***   $(gtk_update_icon_cache)"; \
		echo "***"; \
        fi

install-data-hook: update-icon-cache
uninstall-hook: update-icon-cache
