/*************************************************************************/
/* Copyright (C) 2012-2014 matias <mati86dl@gmail.com>                   */
/*                                                                       */
/* This program is free software: you can redistribute it and/or modify  */
/* it under the terms of the GNU General Public License as published by  */
/* the Free Software Foundation, either version 3 of the License, or     */
/* (at your option) any later version.                                   */
/*                                                                       */
/* This program is distributed in the hope that it will be useful,       */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of        */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         */
/* GNU General Public License for more details.                          */
/*                                                                       */
/* You should have received a copy of the GNU General Public License     */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */
/*************************************************************************/

#include "mpris2-album-art.h"

G_DEFINE_TYPE(Mpris2AlbumArt, mpris2_album_art, GTK_TYPE_IMAGE)

struct _Mpris2AlbumArtPrivate
{
	gchar *path;
	guint size;
};

enum
{
	PROP_0,
	PROP_PATH,
	PROP_SIZE,
	LAST_PROP
};

static GParamSpec *gParamSpecs[LAST_PROP];

Mpris2AlbumArt *
mpris2_album_art_new (void)
{
	return g_object_new(MPRIS2_TYPE_ALBUM_ART, NULL);
}

/**
 * mpris2_album_art_update_image:
 *
 */

static void
mpris2_album_art_update_image (Mpris2AlbumArt *albumart)
{
	Mpris2AlbumArtPrivate *priv;
	GdkPixbuf *pixbuf, *album_art, *frame;
	GError *error = NULL;

	g_return_if_fail(MPRIS2_IS_ALBUM_ART(albumart));

	priv = albumart->priv;

	frame = gdk_pixbuf_new_from_file (BASEICONDIR"/128x128/apps/mpris2-status-icon.png", &error);

	if(priv->path != NULL) {
		album_art = gdk_pixbuf_new_from_file_at_scale (priv->path,
		                                               112, 112, FALSE, &error);
		if (album_art) {
			gdk_pixbuf_copy_area(album_art, 0, 0, 112, 112, frame, 12, 8);
			g_object_unref(G_OBJECT(album_art));
		}
		else {
			g_critical("Unable to open image file: %s\n", priv->path);
			g_error_free(error);
		}
	}

	pixbuf = gdk_pixbuf_scale_simple (frame,
	                                  priv->size, priv->size,
	                                  GDK_INTERP_BILINEAR);

	mpris2_album_art_set_pixbuf (albumart, pixbuf);

	g_object_unref (G_OBJECT(pixbuf));
	g_object_unref (G_OBJECT(frame));
}

/**
 * album_art_get_path:
 *
 */
const gchar *
mpris2_album_art_get_path (Mpris2AlbumArt *albumart)
{
	g_return_val_if_fail (MPRIS2_IS_ALBUM_ART(albumart), NULL);

	return albumart->priv->path;
}

/**
 * album_art_set_path:
 *
 */
void
mpris2_album_art_set_path (Mpris2AlbumArt *albumart,
                           const gchar    *path)
{
	Mpris2AlbumArtPrivate *priv;

	g_return_if_fail (MPRIS2_IS_ALBUM_ART(albumart));

	priv = albumart->priv;

	g_free (priv->path);
	if (path)
		priv->path = g_filename_from_uri(path, NULL, NULL);
	else
		priv->path = NULL;

	mpris2_album_art_update_image (albumart);

	g_object_notify_by_pspec(G_OBJECT(albumart), gParamSpecs[PROP_PATH]);
}

/**
 * album_art_get_size:
 *
 */
guint
mpris2_album_art_get_size (Mpris2AlbumArt *albumart)
{
	g_return_val_if_fail (MPRIS2_IS_ALBUM_ART(albumart), 0);

	return albumart->priv->size;
}

/**
 * album_art_set_size:
 *
 */
void
mpris2_album_art_set_size (Mpris2AlbumArt *albumart,
                           guint           size)
{
	Mpris2AlbumArtPrivate *priv;

	g_return_if_fail(MPRIS2_IS_ALBUM_ART(albumart));

	priv = albumart->priv;

	priv->size = size;

	mpris2_album_art_update_image (albumart);

	g_object_notify_by_pspec(G_OBJECT(albumart), gParamSpecs[PROP_SIZE]);
}

/**
 * album_art_set_pixbuf:
 *
 */
void
mpris2_album_art_set_pixbuf (Mpris2AlbumArt *albumart, GdkPixbuf *pixbuf)
{
	g_return_if_fail (MPRIS2_IS_ALBUM_ART(albumart));

	gtk_image_clear (GTK_IMAGE(albumart));
	gtk_image_set_from_pixbuf (GTK_IMAGE(albumart), pixbuf);
}

/**
 * album_art_get_pixbuf:
 *
 */
GdkPixbuf *
mpris2_album_art_get_pixbuf (Mpris2AlbumArt *albumart)
{
	GdkPixbuf *pixbuf = NULL;

	g_return_val_if_fail (MPRIS2_IS_ALBUM_ART(albumart), NULL);

	if(gtk_image_get_storage_type(GTK_IMAGE(albumart)) == GTK_IMAGE_PIXBUF)
		pixbuf = gtk_image_get_pixbuf (GTK_IMAGE(albumart));

	return pixbuf;
}

static void
mpris2_album_art_finalize (GObject *object)
{
	Mpris2AlbumArtPrivate *priv;

	priv = MPRIS2_ALBUM_ART(object)->priv;

	g_free (priv->path);

	G_OBJECT_CLASS(mpris2_album_art_parent_class)->finalize(object);
}

static void
mpris2_album_art_get_property (GObject    *object,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
	Mpris2AlbumArt *albumart = MPRIS2_ALBUM_ART(object);

	switch (prop_id) {
	case PROP_PATH:
		g_value_set_string(value, mpris2_album_art_get_path(albumart));
		break;
	case PROP_SIZE:
		g_value_set_uint (value, mpris2_album_art_get_size(albumart));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

static void
mpris2_album_art_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
	Mpris2AlbumArt *albumart = MPRIS2_ALBUM_ART(object);

	switch (prop_id) {
	case PROP_PATH:
		mpris2_album_art_set_path(albumart, g_value_get_string(value));
		break;
	case PROP_SIZE:
		mpris2_album_art_set_size(albumart, g_value_get_uint(value));
		break;
	 default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

static void
mpris2_album_art_class_init (Mpris2AlbumArtClass *klass)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS(klass);
	object_class->finalize = mpris2_album_art_finalize;
	object_class->get_property = mpris2_album_art_get_property;
	object_class->set_property = mpris2_album_art_set_property;
	g_type_class_add_private(object_class, sizeof(Mpris2AlbumArtPrivate));

	/**
	 * Mpris2AlbumArt:path:
	 *
	 */
	gParamSpecs[PROP_PATH] =
		g_param_spec_string("path",
		                    "Path",
		                    "The album art path",
		                    BASEICONDIR"/128x128/apps/mpris2-status-icon.png",
		                    G_PARAM_READWRITE |G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);

	/**
	 * Mpris2AlbumArt:size:
	 *
	 */
	gParamSpecs[PROP_SIZE] =
		g_param_spec_uint("size",
		                  "Size",
		                  "The album art size",
		                  24, 512,
		                  48,
		                  G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(object_class, LAST_PROP, gParamSpecs);
}

static void
mpris2_album_art_init (Mpris2AlbumArt *albumart)
{
	albumart->priv = G_TYPE_INSTANCE_GET_PRIVATE (albumart,
	                                              MPRIS2_TYPE_ALBUM_ART,
	                                              Mpris2AlbumArtPrivate);
}
