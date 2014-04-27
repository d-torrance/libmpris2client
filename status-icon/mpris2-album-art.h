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

#ifndef MPRIS2_ALBUM_ART_H
#define MPRIS2_ALBUM_ART_H

#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define MPRIS2_TYPE_ALBUM_ART (mpris2_album_art_get_type())
#define MPRIS2_ALBUM_ART(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), MPRIS2_TYPE_ALBUM_ART, Mpris2AlbumArt))
#define MPRIS2_ALBUM_ART_CONST(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), MPRIS2_TYPE_ALBUM_ART, Mpris2AlbumArt const))
#define MPRIS2_ALBUM_ART_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), MPRIS2_TYPE_ALBUM_ART, Mpris2AlbumArtClass))
#define MPRIS2_IS_ALBUM_ART(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MPRIS2_TYPE_ALBUM_ART))
#define MPRIS2_IS_ALBUM_ART_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), MPRIS2_TYPE_ALBUM_ART))
#define MPRIS2_ALBUM_ART_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), MPRIS2_TYPE_ALBUM_ART, Mpris2AlbumArtClass))

typedef struct _Mpris2AlbumArt Mpris2AlbumArt;
typedef struct _Mpris2AlbumArtClass Mpris2AlbumArtClass;
typedef struct _Mpris2AlbumArtPrivate Mpris2AlbumArtPrivate;

struct _Mpris2AlbumArt
{
   GtkImage parent;

   /*< private >*/
   Mpris2AlbumArtPrivate *priv;
};

struct _Mpris2AlbumArtClass
{
   GtkImageClass parent_class;
};

GType mpris2_album_art_get_type (void) G_GNUC_CONST;

/*
 * Api.
 */
Mpris2AlbumArt *mpris2_album_art_new        (void);

const gchar    *mpris2_album_art_get_path   (Mpris2AlbumArt *albumart);
void            mpris2_album_art_set_path   (Mpris2AlbumArt *albumart, const char *path);

guint           mpris2_album_art_get_size   (Mpris2AlbumArt *albumart);
void            mpris2_album_art_set_size   (Mpris2AlbumArt *albumart, guint size);

GdkPixbuf      *mpris2_album_art_get_pixbuf (Mpris2AlbumArt *albumart);
void            mpris2_album_art_set_pixbuf (Mpris2AlbumArt *albumart, GdkPixbuf *pixbuf);

G_END_DECLS

#endif /* MPRIS2_ALBUM_ART_H */
