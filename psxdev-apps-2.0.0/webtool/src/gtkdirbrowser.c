/* $Id: gtkdirbrowser.c,v 1.1.1.1 2001/05/17 11:50:27 dbalster Exp $ */

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

#include <gtk/gtk.h>
#include <gtkdirbrowser.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>

enum {
	ARG_0,
	ARG_USE_ICONS,
	ARG_USER_ROOT,
	ARG_PATH,
};

enum {
	SIG_PATH_CHANGED,
	SIG_BUSY,
	SIG_LAST
};

/* object class methods */
static void gtk_dir_browser_class_init (GtkDirBrowserClass *klass);
static void gtk_dir_browser_init (GtkDirBrowser *dir_browser);
static void gtk_dir_browser_destroy (GtkObject *object);
static void gtk_dir_browser_set_arg (GtkObject *object, GtkArg *arg, guint arg_id);
static void gtk_dir_browser_get_arg (GtkObject *object, GtkArg *arg, guint arg_id);

static GtkCTreeClass *parent_class = NULL;
static guint dir_browser_signals[SIG_LAST] = { 0 };

/**************************************************************/

/** static icons **/

#include "open.xpm"
#include "close.xpm"
#include "link.xpm"
#include "lock.xpm"

typedef enum {
	ICON_OPEN=0, ICON_CLOSE, ICON_LINK, ICON_LOCK, ICON_COUNT
} GtkDirBrowserIconType;

static const gchar **folder_icons[ICON_COUNT] = {
	open_xpm, close_xpm, link_xpm, lock_xpm
};

static GdkPixmap *folder_pixmap[ICON_COUNT] = { 0 };
static GdkBitmap *folder_mask[ICON_COUNT];

/**************************************************************/

/**
 * get_icon
 * 
 * @widget must be of type GTK_DIR_BROWSER
 * @name a complete path to the object to be icon fitted
 * @pixmaps ..pointers to pointers to be filled.
 */

static void
get_icon (GtkDirBrowser *dir_browser, const gchar *name,
	GdkPixmap **close_pixmap, GdkBitmap **close_mask,
	GdkPixmap **open_pixmap, GdkBitmap **open_mask)
{
	GtkDirBrowserIconType type = ICON_CLOSE;
	gchar buf[1];
	
	g_return_if_fail (GTK_IS_DIR_BROWSER(dir_browser));
	
	if (dir_browser->use_icons==FALSE)
	{
		*close_pixmap = *close_mask = *open_pixmap = *open_mask = NULL;
		return;
	}

	// symbolic link to directory
	if (readlink(name,buf,1)>0) type = ICON_LINK;

	// directory is accessible
	if (access(name,X_OK)) type = ICON_LOCK;

	if (folder_pixmap[ICON_OPEN]==NULL)
	{
		folder_pixmap[ICON_OPEN] = gdk_pixmap_create_from_xpm_d (
		GTK_WIDGET(dir_browser)->window, &folder_mask[ICON_OPEN], NULL, folder_icons[ICON_OPEN]);
	}

	if (folder_pixmap[type]==NULL)
	{
		folder_pixmap[type] = gdk_pixmap_create_from_xpm_d (
		GTK_WIDGET(dir_browser)->window, &folder_mask[type], NULL, folder_icons[type]);
	}
	
	*close_pixmap = folder_pixmap[type];
	*close_mask = folder_mask[type];
	*open_pixmap = folder_pixmap[ICON_OPEN];
	*open_mask = folder_mask[ICON_OPEN];
}

static gboolean is_dir_empty (const gchar *path)
{
	DIR *dir;
	struct stat st;
	struct dirent *dirent;
	gboolean empty = TRUE;

	dir = opendir (path);
	if (dir)
	{
		while (dirent = readdir(dir))
		{
			gchar *name;
			
			if ((dirent->d_name[0] == '.') && (dirent->d_name[1]==0)) continue;
			if ((dirent->d_name[0] == '.') && (dirent->d_name[1] == '.') && (dirent->d_name[2]==0)) continue;

			name = g_strconcat(path, G_DIR_SEPARATOR_S, dirent->d_name, NULL);
			if (stat(name, &st)==0)
			{
				g_free (name);
				if (S_ISDIR(st.st_mode))
				{
					empty = FALSE;
					break;
				}
			}
		}
		closedir (dir);
	}
	
	return empty;
}

static
gchar *gtk_ctree_get_text_path (GtkCTree *ctree, GtkCTreeNode *node, gchar separator)
{
	GtkCTreeNode *child = node;
	gchar *text, *path;
	GString *string;

	string = g_string_new (NULL);
	string = g_string_insert_c (string,0,separator);

	while (GTK_CTREE_ROW (child)->parent)
	{
		gchar *pixtext = GTK_CELL_PIXTEXT (GTK_CTREE_ROW (child)->row.cell[0])->text;
	
		string = g_string_insert (string,0,pixtext);
		string = g_string_insert_c (string,0,separator);
		child = GTK_CTREE_ROW (child)->parent;
	}

	path = gtk_ctree_node_get_row_data (ctree,child);
	if (path != NULL)
	{
		if (strlen(path)>1)
		string = g_string_insert (string, 0, path);
	}

	text = g_strdup (string->str);
	g_string_free (string,TRUE);
	
	return text;
}

static void
on_tree_select_row (GtkCTree *ctree, GList *node, gint column, gpointer user_data)
{
	gchar *name, *path;

	path = gtk_ctree_get_text_path (ctree,GTK_CTREE_NODE(node),G_DIR_SEPARATOR);

	g_free (GTK_DIR_BROWSER(ctree)->path);
	GTK_DIR_BROWSER(ctree)->path = path;

	gtk_dir_browser_path_changed (GTK_DIR_BROWSER(ctree));
}

static void
on_tree_collapse (GtkCTree *ctree, GList *node, gpointer user_data)
{
	if (node == NULL) return;

	gtk_clist_freeze (GTK_CLIST(ctree));

	while (GTK_CTREE_ROW (node)->children)
	{
		// the last node will not be removed to allow re-expanding
		if (GTK_CTREE_NODE_NEXT(GTK_CTREE_ROW (node)->children)) break;
	
		gtk_ctree_remove_node (ctree,GTK_CTREE_ROW (node)->children);
	}

	gtk_clist_thaw (GTK_CLIST(ctree));
}

static void
on_tree_expand (GtkCTree *ctree, GList *node, gpointer user_data)
{
	gchar *path;
	gint row;
	GtkCTreeNode *child;
	DIR *dir;
	struct stat st;
	struct dirent *dirent;
	gchar *name;

	if (node == NULL) return;

	gtk_signal_emit (GTK_OBJECT (ctree),
		dir_browser_signals[SIG_BUSY], TRUE, NULL);

	gtk_clist_freeze (GTK_CLIST(ctree));

	while (GTK_CTREE_ROW (node)->children)
	{
		gtk_ctree_remove_node (ctree,GTK_CTREE_ROW (node)->children);
	}

	path = gtk_ctree_get_text_path (ctree,GTK_CTREE_NODE(node),G_DIR_SEPARATOR);

	dir = opendir(path);
	if (dir)
	{
		while (dirent = readdir(dir))
		{
			GdkPixmap *icon1,*icon2;
			GdkBitmap *mask1,*mask2;
	
			if ((dirent->d_name[0] == '.') && (dirent->d_name[1]==0)) continue;
			if ((dirent->d_name[0] == '.') && (dirent->d_name[1] == '.') && (dirent->d_name[2]==0)) continue;

			// path always ends with G_DIR_SEPARATOR
			name = g_strconcat(path, dirent->d_name, NULL);
			if (stat(name, &st)==0)
			{
				if (GTK_DIR_BROWSER(ctree)->hide_dotdirs && dirent->d_name[0] == '.')
				{
				}
				else if (S_ISDIR(st.st_mode))
				{
					gchar *label[1];
					label[0] = dirent->d_name;
					
					
					get_icon (GTK_DIR_BROWSER(ctree),name,&icon1,&mask1,&icon2,&mask2);
					
					child = gtk_ctree_insert_node (GTK_CTREE(ctree), GTK_CTREE_NODE(node), NULL, label, 0, icon1,mask1,icon2,mask2, FALSE, FALSE);

					// expansion only works with at least one child node
					if (!is_dir_empty(name))
						gtk_ctree_insert_node (GTK_CTREE(ctree), child, NULL, label, 0, NULL, NULL, NULL, NULL, FALSE, FALSE);
				}
			}
			g_free (name);
		}

		closedir(dir);
		gtk_ctree_sort_node (GTK_CTREE(ctree), GTK_CTREE_NODE(node));
		gtk_ctree_select (GTK_CTREE(ctree), GTK_CTREE_NODE(node));
	}

	gtk_clist_thaw (GTK_CLIST(ctree));

	gtk_signal_emit (GTK_OBJECT (ctree),
		dir_browser_signals[SIG_BUSY], FALSE, NULL);
}

/********/

GtkType
gtk_dir_browser_get_type (void)
{
	static GtkType dir_browser_type = 0;

	if (!dir_browser_type)
	{
		GtkTypeInfo dir_browser_info =
		{
			(gchar*)"GtkDirBrowser",
			sizeof (GtkDirBrowser),
			sizeof (GtkDirBrowserClass),
			(GtkClassInitFunc) gtk_dir_browser_class_init,
			(GtkObjectInitFunc) gtk_dir_browser_init,
			/* reserved_1 */ NULL,
			/* reserved_2 */ NULL,
			(GtkClassInitFunc) NULL
		};
		dir_browser_type = gtk_type_unique (gtk_ctree_get_type (), &dir_browser_info);
	}

	return dir_browser_type;
}

static void
gtk_dir_browser_class_init (GtkDirBrowserClass *klass)
{
	GtkObjectClass *object_class;

	parent_class = gtk_type_class(gtk_ctree_get_type());
	object_class = (GtkObjectClass*) klass;

	object_class->destroy = gtk_dir_browser_destroy;
	object_class->set_arg = gtk_dir_browser_set_arg;
	object_class->get_arg = gtk_dir_browser_get_arg;

	/* register arguments */

	gtk_object_add_arg_type ("GtkDirBrowser::use_icons",
		GTK_TYPE_BOOL,
		GTK_ARG_READWRITE,
		ARG_USE_ICONS);

	gtk_object_add_arg_type ("GtkDirBrowser::path",
		GTK_TYPE_STRING,
		GTK_ARG_READWRITE,
		ARG_PATH);

	/* register signals */

	dir_browser_signals[SIG_PATH_CHANGED] =
	gtk_signal_new ("path_changed",
		GTK_RUN_FIRST | GTK_RUN_NO_RECURSE,
		object_class->type,
		GTK_SIGNAL_OFFSET (GtkDirBrowserClass, path_changed),
		gtk_marshal_NONE__STRING,
		GTK_TYPE_NONE,
		1, /* parameters */
		GTK_TYPE_STRING);

	dir_browser_signals[SIG_BUSY] =
	gtk_signal_new ("busy",
		GTK_RUN_FIRST | GTK_RUN_NO_RECURSE,
		object_class->type,
		GTK_SIGNAL_OFFSET (GtkDirBrowserClass, busy),
		gtk_marshal_NONE__BOOL,
		GTK_TYPE_NONE,
		1, /* parameters */
		GTK_TYPE_BOOL);

	gtk_object_class_add_signals (object_class, dir_browser_signals, SIG_LAST);

	klass->path_changed = NULL;
	klass->busy = NULL;
}

static void
gtk_dir_browser_init (GtkDirBrowser *dir_browser)
{
	dir_browser->use_icons = TRUE;
	dir_browser->hide_dotdirs = TRUE;
	dir_browser->path = NULL;
}

GtkDirBrowser*
gtk_dir_browser_new (void)
{
	GtkDirBrowser *dir_browser;

	dir_browser = gtk_type_new (gtk_dir_browser_get_type());

	gtk_ctree_construct (GTK_CTREE(dir_browser),1,0,NULL);
	gtk_clist_set_selection_mode (GTK_CLIST(dir_browser),GTK_SELECTION_BROWSE);

	gtk_clist_set_column_auto_resize (GTK_CLIST(dir_browser),0,TRUE);

	dir_browser->use_icons = TRUE;
	dir_browser->roots = NULL;

	gtk_signal_connect (GTK_OBJECT(GTK_CTREE(dir_browser)), "tree_select_row",
		GTK_SIGNAL_FUNC (on_tree_select_row), NULL);

	gtk_signal_connect (GTK_OBJECT(GTK_CTREE(dir_browser)), "tree_expand",
		GTK_SIGNAL_FUNC (on_tree_expand), NULL);

	gtk_signal_connect (GTK_OBJECT(GTK_CTREE(dir_browser)), "tree_collapse",
		GTK_SIGNAL_FUNC (on_tree_collapse), NULL);

	return GTK_DIR_BROWSER(dir_browser);
}

static void
gtk_dir_browser_destroy (GtkObject *object)
{
	GtkDirBrowser *dir_browser;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GTK_IS_DIR_BROWSER(object));
    
	dir_browser = GTK_DIR_BROWSER(object);

	g_free (dir_browser->path);

	gtk_clist_freeze (GTK_CLIST(object));
	gtk_clist_clear (GTK_CLIST(object));

	g_list_free (dir_browser->roots);

	if (GTK_OBJECT_CLASS (parent_class)->destroy)
	 (* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}

static void
gtk_dir_browser_set_arg (GtkObject *object,
	GtkArg *arg,
	guint arg_id)
{
	GtkDirBrowser *dir_browser;

	dir_browser = GTK_DIR_BROWSER (object);
  
	switch (arg_id)
	{
		case ARG_USE_ICONS:
			gtk_dir_browser_set_use_icons (dir_browser, GTK_VALUE_BOOL (*arg));
			break;
		case ARG_PATH:
			gtk_dir_browser_set_path (dir_browser, GTK_VALUE_STRING (*arg));
			break;
		default:
			break;
	}
}

static void
gtk_dir_browser_get_arg (GtkObject *object,
	GtkArg *arg,
	guint arg_id)
{
	GtkDirBrowser *dir_browser;

	dir_browser = GTK_DIR_BROWSER (object);
  
	switch (arg_id)
	{
		case ARG_USE_ICONS:
			GTK_VALUE_BOOL (*arg) = dir_browser->use_icons;
			break;
		case ARG_PATH:
			GTK_VALUE_STRING (*arg) = dir_browser->path;
			break;
		default:
			arg->type = GTK_TYPE_INVALID;
			break;
	}
}

/* public interface */

void 
gtk_dir_browser_set_use_icons (GtkDirBrowser *dir_browser, gboolean arg)
{
	g_return_if_fail (dir_browser != NULL);
	g_return_if_fail (GTK_IS_DIR_BROWSER (dir_browser));

	dir_browser->use_icons = arg;
}

gboolean
gtk_dir_browser_get_use_icons (GtkDirBrowser *dir_browser)
{
	g_return_val_if_fail (dir_browser != NULL, FALSE);
	g_return_val_if_fail (GTK_IS_DIR_BROWSER (dir_browser), FALSE);

	return dir_browser->use_icons;
}

void
gtk_dir_browser_path_changed (GtkDirBrowser *dir_browser)
{
	g_return_if_fail (dir_browser != NULL);
	g_return_if_fail (GTK_IS_DIR_BROWSER (dir_browser));

	gtk_signal_emit (GTK_OBJECT (dir_browser),
		dir_browser_signals[SIG_PATH_CHANGED], dir_browser->path, NULL);
}

gboolean
gtk_dir_browser_set_path (GtkDirBrowser *dir_browser, const gchar *pathname)
{
	gchar **token, **iter;
	GtkWidget *ctree, *clist, *entry;
	GtkCTreeNode *node, *parent;
	GtkCTreeRow *row;
	GtkCTreeNode *child;
	gchar *label[10], *path = NULL;
	gint level;

	g_return_val_if_fail (dir_browser != NULL, FALSE);
	g_return_val_if_fail (GTK_IS_DIR_BROWSER (dir_browser), FALSE);

// "compress" the pathname

	if (access(pathname,X_OK)) return FALSE;

	{
		int old = chdir (pathname);
		
		if (old==0)
		{
			gchar *tpath = g_get_current_dir ();
			
			path = g_strconcat (tpath,G_DIR_SEPARATOR_S,NULL);
			g_free (tpath);
		
			fchdir (old);
		}
		else return (FALSE);
	}

	gtk_clist_freeze (GTK_CLIST(dir_browser));

	token = g_strsplit (path, G_DIR_SEPARATOR_S, -1);
	if (token)
	{
		GList *list = dir_browser->roots;
		iter = token;

		while (list)
		{
			gchar *txt;
			node = list->data;

			txt = gtk_ctree_node_get_row_data (GTK_CTREE(dir_browser),node);

			if (**iter == '.') dir_browser->hide_dotdirs = FALSE;

			if (txt)
			{
				if (strlen(txt+1)==0 && strlen(*iter)==0)
				{
					iter++;
					break;
				}

				if (strcmp(txt,*iter)==0)
				{
					break;
				}
			}

			list = list->next;
			node = NULL;
		}

		gtk_signal_handler_block_by_func (GTK_OBJECT(dir_browser),
			GTK_SIGNAL_FUNC(on_tree_select_row),NULL);

		while (*iter)
		{
			gboolean found = FALSE;

			if (**iter == '.') dir_browser->hide_dotdirs = FALSE;

			if (node==NULL) break;
			gtk_ctree_expand (GTK_CTREE(dir_browser),node);

			if (GTK_CTREE_ROW (node)->children)
			{
				for (child = GTK_CTREE_ROW (node)->children; child;
					child = GTK_CTREE_ROW (child)->sibling)
				{
					gchar *label = GTK_CELL_PIXTEXT(GTK_CTREE_ROW(child)->row.cell[0])->text;
					
					if (strcmp(label,*iter)==0)
					{
						found = TRUE;
						break;
					}
				}
			}

			if (!found)
			{
		//		g_print ("BAD BAD BAD!\n");
				break;
			}
			
			node = child;

			iter++;
		}
		gtk_signal_handler_unblock_by_func (GTK_OBJECT(dir_browser),
			GTK_SIGNAL_FUNC(on_tree_select_row),NULL);
		
		if (node != NULL) gtk_ctree_expand (GTK_CTREE(dir_browser),node);
	
		g_strfreev (token);
	}

	gtk_clist_thaw (GTK_CLIST(dir_browser));

//	g_free (dir_browser->path);
//	dir_browser->path = path;

//	gtk_dir_browser_path_changed (dir_browser);
}

const gchar *
gtk_dir_browser_get_path (GtkDirBrowser *dir_browser)
{
	g_return_val_if_fail (dir_browser != NULL, NULL);
	g_return_val_if_fail (GTK_IS_DIR_BROWSER (dir_browser), NULL);

	return dir_browser->path;
}

void gtk_dir_browser_add_root (GtkDirBrowser *dir_browser, const gchar *text, const gchar *root)
{
	GtkCTreeNode *node;
	gchar const *label[1];
	GdkPixmap *icon1,*icon2;
	GdkBitmap *mask1,*mask2;

	g_return_if_fail (dir_browser != NULL);
	g_return_if_fail (GTK_IS_DIR_BROWSER (dir_browser));

	if (!g_path_is_absolute(root)) return;

	gtk_clist_freeze (GTK_CLIST(dir_browser));

	if (root != NULL)
	{
		label[0] = text;

		get_icon (dir_browser,root,&icon1,&mask1,&icon2,&mask2);

		node = gtk_ctree_insert_node (GTK_CTREE(dir_browser),NULL,NULL,(gchar**)label,0,icon1,mask1,icon2,mask2,FALSE,FALSE);
		if (!is_dir_empty(root))
			gtk_ctree_insert_node (GTK_CTREE(dir_browser), node, NULL, (gchar**)label, 0, NULL, NULL, NULL, NULL, FALSE, FALSE);
		gtk_ctree_node_set_row_data_full (GTK_CTREE(dir_browser),node,g_strdup(root),g_free);

		dir_browser->roots = g_list_append (dir_browser->roots, node);
	}

	gtk_clist_thaw (GTK_CLIST(dir_browser));
}

const GList *gtk_dir_browser_get_roots (GtkDirBrowser *dir_browser)
{
	g_return_val_if_fail (dir_browser != NULL, NULL);
	g_return_val_if_fail (GTK_IS_DIR_BROWSER (dir_browser), NULL);

	return dir_browser->roots;
}
