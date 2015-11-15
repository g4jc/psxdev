/* $Id: gtkdirbrowser.h,v 1.1.1.1 2001/05/17 11:50:27 dbalster Exp $ */

/*
 * GtkDirBrowser - directory browser widget for gtk+
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

#ifndef __GTK_DIR_BROWSER_H__
#define __GTK_DIR_BROWSER_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define GTK_TYPE_DIR_BROWSER          (gtk_dir_browser_get_type())
#define GTK_DIR_BROWSER(obj)          (GTK_CHECK_CAST ((obj), GTK_TYPE_DIR_BROWSER, GtkDirBrowser))
#define GTK_DIR_BROWSER_CLASS(klass)  (GTK_CHECK_CLASS_CAST (klass, GTK_TYPE_DIR_BROWSER, GtkDirBrowserClass))
#define GTK_IS_DIR_BROWSER(obj)       (GTK_CHECK_TYPE ((obj), GTK_TYPE_DIR_BROWSER))
#define GTK_IS_DIR_BROWSER_CLASS      (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_DIR_BROWSER))

typedef struct _GtkDirBrowser         GtkDirBrowser;
typedef struct _GtkDirBrowserClass    GtkDirBrowserClass;

struct _GtkDirBrowser
{
	GtkCTree ctree; /* parent object */

	// whether or not to use icons. default is YES
	gboolean use_icons;
	gboolean hide_dotdirs;

	// the currently selected path, read-only. can be NULL!
	// access with gtk_dir_browser_get_path(),
	// set with gtk_dir_browser_set_path()
	gchar *path;
	
	// a list of GtkCTreeNodes representing the root points.
	// access with the appropriate functions !
	GList *roots;
};

struct _GtkDirBrowserClass
{
	GtkCTreeClass parent_class; /* parent class */

	void (* path_changed) (GtkDirBrowser *dir_browser, const gchar *path);
	void (* busy) (GtkDirBrowser *dir_browser, gboolean busy);
};

GtkType gtk_dir_browser_get_type (void);
GtkDirBrowser* gtk_dir_browser_new (void);

void gtk_dir_browser_set_use_icons (GtkDirBrowser *dir_browser, gboolean arg);
gboolean gtk_dir_browser_get_use_icons (GtkDirBrowser *dir_browser);

void gtk_dir_browser_add_root (GtkDirBrowser *dir_browser, const gchar *label, const gchar *root);
const GList *gtk_dir_browser_get_roots (GtkDirBrowser *dir_browser);

gboolean gtk_dir_browser_set_path (GtkDirBrowser *dir_browser, const gchar *arg);
const gchar *gtk_dir_browser_get_path (GtkDirBrowser *dir_browser);
void gtk_dir_browser_path_changed (GtkDirBrowser *dir_browser);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __GTK_DIR_BROWSER_H__ */
