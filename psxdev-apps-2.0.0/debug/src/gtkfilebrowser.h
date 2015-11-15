/* $Id: gtkfilebrowser.h,v 1.1.1.1 2001/05/17 11:50:27 dbalster Exp $ */

/*
 * GtkFileBrowser - directory/file browser widget for gtk+
 * Copyright 1999 Daniel Balster <dbalster@poboxes.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __GTK_FILE_BROWSER_H__
#define __GTK_FILE_BROWSER_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define GTK_TYPE_FILE_BROWSER          (gtk_file_browser_get_type())
#define GTK_FILE_BROWSER(obj)          (GTK_CHECK_CAST ((obj), GTK_TYPE_FILE_BROWSER, GtkFileBrowser))
#define GTK_FILE_BROWSER_CLASS(klass)  (GTK_CHECK_CLASS_CAST (klass, GTK_TYPE_FILE_BROWSER, GtkFileBrowserClass))
#define GTK_IS_FILE_BROWSER(obj)       (GTK_CHECK_TYPE ((obj), GTK_TYPE_FILE_BROWSER))
#define GTK_IS_FILE_BROWSER_CLASS      (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_FILE_BROWSER))

typedef struct _GtkFileBrowser         GtkFileBrowser;
typedef struct _GtkFileBrowserClass    GtkFileBrowserClass;

struct _GtkFileBrowser
{
	GtkCList clist; /* parent object */

	gboolean use_icons;
	gchar *path;
};

struct _GtkFileBrowserClass
{
	GtkCListClass parent_class; /* parent class */

	void (* path_changed) (GtkFileBrowser *file_browser, const gchar *path);
	void (* clicked) (GtkFileBrowser *file_browser, const gchar *path);
	void (* busy) (GtkFileBrowser *file_browser, gboolean busy);
};

GtkType gtk_file_browser_get_type (void);
GtkFileBrowser* gtk_file_browser_new (void);

void gtk_file_browser_set_use_icons (GtkFileBrowser *file_browser, gboolean arg);
gboolean gtk_file_browser_get_use_icons (GtkFileBrowser *file_browser);

void gtk_file_browser_set_path (GtkFileBrowser *file_browser, const gchar *arg);
const gchar *gtk_file_browser_get_path (GtkFileBrowser *file_browser);
void gtk_file_browser_path_changed (GtkFileBrowser *file_browser);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __GTK_FILE_BROWSER_H__ */
