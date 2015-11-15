/* $Id: gtkfilebrowser.c,v 1.1.1.1 2001/05/17 11:50:27 dbalster Exp $ */

/*
 * GtkFileBrowser - directory browser widget for gtk+
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
#include <gtkfilebrowser.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>

#include <pwd.h>
#include <grp.h>

enum {
	ARG_0,
	ARG_USE_ICONS,
	ARG_PATH,
};

enum {
	SIG_PATH_CHANGED,		// directory path changed
	SIG_BUSY,				// widget scans files
	SIG_CLICKED,			// doubleclicked something
	SIG_LAST
};

/* object class methods */
static void gtk_file_browser_class_init (GtkFileBrowserClass *klass);
static void gtk_file_browser_init (GtkFileBrowser *file_browser);
static void gtk_file_browser_destroy (GtkObject *object);
static void gtk_file_browser_set_arg (GtkObject *object, GtkArg *arg, guint arg_id);
static void gtk_file_browser_get_arg (GtkObject *object, GtkArg *arg, guint arg_id);

static GtkCListClass *parent_class = NULL;
static guint file_browser_signals[SIG_LAST] = { 0 };

/**************************************************************/

/** static icons **/

#include "open.xpm"
#include "close.xpm"
#include "link.xpm"
#include "lock.xpm"

typedef enum {
	ICON_OPEN=0, ICON_CLOSE, ICON_LINK, ICON_LOCK, ICON_COUNT
} GtkFileBrowserIconType;

static const gchar **folder_icons[ICON_COUNT] = {
	open_xpm, close_xpm, link_xpm, lock_xpm
};

static GdkPixmap *folder_pixmap[ICON_COUNT] = { 0 };
static GdkBitmap *folder_mask[ICON_COUNT];

/**************************************************************/

static const gchar *titles[] = {
"Type",
"Name",
"Size",
"Modes",
"Attributes",
"Access",
"Modify",
"Change",
"Links",
"User",
"Group",
"INode",
"Device"
};

/**
 * get_icon
 * 
 * @widget must be of type GTK_FILE_BROWSER
 * @name a complete path to the object to be icon fitted
 * @pixmaps ..pointers to pointers to be filled.
 */

static void
get_icon (GtkFileBrowser *file_browser, const gchar *name,
	GdkPixmap **close_pixmap, GdkBitmap **close_mask,
	GdkPixmap **open_pixmap, GdkBitmap **open_mask)
{
	GtkFileBrowserIconType type = ICON_CLOSE;
	gchar buf[1];
	
	g_return_if_fail (GTK_IS_FILE_BROWSER(file_browser));
	
	if (file_browser->use_icons==FALSE)
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
		GTK_WIDGET(file_browser)->window, &folder_mask[ICON_OPEN], NULL, folder_icons[ICON_OPEN]);
	}

	if (folder_pixmap[type]==NULL)
	{
		folder_pixmap[type] = gdk_pixmap_create_from_xpm_d (
		GTK_WIDGET(file_browser)->window, &folder_mask[type], NULL, folder_icons[type]);
	}
	
	*close_pixmap = folder_pixmap[type];
	*close_mask = folder_mask[type];
	*open_pixmap = folder_pixmap[ICON_OPEN];
	*open_mask = folder_mask[ICON_OPEN];
}

static char *strmode (mode_t mode)
{
	static char str[16];

	strcpy (str,"----------");

	if (mode & S_IFLNK) str[0] = 'l';
	if (mode & S_IFBLK) str[0] = 'b';
	if (mode & S_IFDIR) str[0] = 'd';
	if (mode & S_IFCHR) str[0] = 'c';
	if (mode & S_IFIFO) str[0] = 'p';
	if (mode & S_ISVTX) str[0] = 's';

	if (mode & S_IRUSR) str[1] = 'r';
	if (mode & S_IWUSR) str[2] = 'w';
	if (mode & S_IXUSR) str[3] = 'x';

	if (mode & S_IRGRP) str[4] = 'r';
	if (mode & S_IWGRP) str[5] = 'w';
	if (mode & S_IXGRP) str[6] = 'x';

	if (mode & S_IROTH) str[7] = 'r';
	if (mode & S_IWOTH) str[8] = 'w';
	if (mode & S_IXOTH) str[9] = 'x';

	if (mode & S_ISUID) str[3] = 's';
	if (mode & S_ISGID) str[6] = 's';

	return str;
}

#define HAVE_EXT2_FS_H

#ifdef HAVE_EXT2_FS_H
#include <linux/ext2_fs.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#endif

unsigned long getflags (char *name)
{
#ifdef HAVE_EXT2_FS_H
	unsigned long a = 0;
	int fd;
	fd = open (name, O_RDONLY|O_NONBLOCK);
	if (fd == -1) return 0;
	ioctl (fd, EXT2_IOC_GETFLAGS, &a);
	close (fd);
	return a;
#else
	return 0;
#endif
}

static char *strattr (unsigned long a)
{
	static char str[16];

	strcpy (str,"--------");

#ifdef HAVE_EXT2_FS_H
	if (a & EXT2_SECRM_FL) str[0] = 's';
	if (a & EXT2_UNRM_FL) str[1] = 'u';
	if (a & EXT2_COMPR_FL) str[2] = 'c';
	if (a & EXT2_SYNC_FL) str[3] = 'S';
	if (a & EXT2_IMMUTABLE_FL) str[4] = 'i';
	if (a & EXT2_APPEND_FL) str[5] = 'a';
	if (a & EXT2_NODUMP_FL) str[6] = 'd';
	if (a & EXT2_NOATIME_FL) str[7] = 'A';
#endif
	return str;
}

static void
on_select_row (GtkCList *clist, gint row, gint column, GdkEvent *event, gpointer data)
{
	GtkFileBrowser *file_browser = GTK_FILE_BROWSER(clist);
	gchar *label, *path;
	
	if (file_browser->path==NULL) return;

	if (gtk_clist_get_text (clist,row,1,&label))
	{
		path = g_strconcat (file_browser->path,G_DIR_SEPARATOR_S,label,NULL);

		if (event) switch (event->type)
		{
			case GDK_2BUTTON_PRESS:
				gtk_signal_emit (GTK_OBJECT (clist),
					file_browser_signals[SIG_CLICKED], path, NULL);
				break;
		}

		g_free (path);
	}
}

static void rescan (GtkFileBrowser *file_browser)
{
	gchar *path;
	gint row;
	GtkCTreeNode *child;
	DIR *dir = NULL;
	struct stat st;
	struct dirent *dirent;
	gchar *name;
	gchar *label[14];

	gtk_clist_freeze (GTK_CLIST(file_browser));
	gtk_clist_clear (GTK_CLIST(file_browser));

	if (file_browser->path != NULL) dir = opendir(file_browser->path);
	if (dir)
	{
		while (dirent = readdir(dir))
		{
			GdkPixmap *icon1,*icon2;
			GdkBitmap *mask1,*mask2;
			gchar buf_size[10];
			gchar buf_nlink[10];
			
			label[0] = NULL;
			label[1] = dirent->d_name;
			label[2] = buf_size;
			label[8] = buf_nlink;

			if ((dirent->d_name[0] == '.') && (dirent->d_name[1]==0)) continue;
	
			name = g_strconcat (file_browser->path, G_DIR_SEPARATOR_S, dirent->d_name, NULL);
			if (stat(name, &st)==0)
			{
				struct group *gr;
				struct passwd *pw;
			
// TYPE
				if (S_ISDIR(st.st_mode))
				{
					label[0] = "<DIR>";
				}
// NAME
// SIZE
				g_snprintf (buf_size,10,"%d",st.st_size);
// FLAGS
				label[3] = strmode(st.st_mode);
// ATTR
				label[4] = strattr(getflags(name));
// access,modify,change
				label[5] = label[6] = label[7] = "10.10.1999 12:30";
// N LINKS
				g_snprintf (buf_nlink,10,"%d",st.st_nlink);

				pw = getpwuid (st.st_uid);
				gr = getgrgid (st.st_gid);
				
				label[9]  = pw ? pw->pw_name : "?";
				label[10] = gr ?  gr->gr_name : "?";
//"INode",
//"Device"

				label[11] = NULL;
				label[12] = NULL;
				
				row = gtk_clist_append (GTK_CLIST(file_browser),label);
			}
			g_free (name);
		}

		closedir(dir);
	//	gtk_ctree_sort_node (GTK_CTREE(file_browser), GTK_CTREE_NODE(node));
	//	gtk_ctree_select (GTK_CTREE(ctree), GTK_CTREE_NODE(node));
	}

	gtk_clist_thaw (GTK_CLIST(file_browser));
}

/**************************************************************/

GtkType
gtk_file_browser_get_type (void)
{
	static GtkType file_browser_type = 0;

	if (!file_browser_type)
	{
		GtkTypeInfo file_browser_info =
		{
			(gchar*)"GtkFileBrowser",
			sizeof (GtkFileBrowser),
			sizeof (GtkFileBrowserClass),
			(GtkClassInitFunc) gtk_file_browser_class_init,
			(GtkObjectInitFunc) gtk_file_browser_init,
			/* reserved_1 */ NULL,
			/* reserved_2 */ NULL,
			(GtkClassInitFunc) NULL
		};
		file_browser_type = gtk_type_unique (gtk_clist_get_type (), &file_browser_info);
	}

	return file_browser_type;
}

static void
gtk_file_browser_class_init (GtkFileBrowserClass *klass)
{
	GtkObjectClass *object_class;

	parent_class = gtk_type_class(gtk_clist_get_type());
	object_class = (GtkObjectClass*) klass;

	object_class->destroy = gtk_file_browser_destroy;
	object_class->set_arg = gtk_file_browser_set_arg;
	object_class->get_arg = gtk_file_browser_get_arg;

	/* register arguments */

	gtk_object_add_arg_type ("GtkFileBrowser::use_icons",
		GTK_TYPE_BOOL,
		GTK_ARG_READWRITE,
		ARG_USE_ICONS);

	gtk_object_add_arg_type ("GtkFileBrowser::path",
		GTK_TYPE_STRING,
		GTK_ARG_READWRITE,
		ARG_PATH);

	/* register signals */

	file_browser_signals[SIG_PATH_CHANGED] =
	gtk_signal_new ("path_changed",
		GTK_RUN_FIRST | GTK_RUN_NO_RECURSE,
		object_class->type,
		GTK_SIGNAL_OFFSET (GtkFileBrowserClass, path_changed),
		gtk_marshal_NONE__STRING,
		GTK_TYPE_NONE,
		1, /* parameters */
		GTK_TYPE_STRING);

	file_browser_signals[SIG_CLICKED] =
	gtk_signal_new ("clicked",
		GTK_RUN_FIRST | GTK_RUN_NO_RECURSE,
		object_class->type,
		GTK_SIGNAL_OFFSET (GtkFileBrowserClass, clicked),
		gtk_marshal_NONE__STRING,
		GTK_TYPE_NONE,
		1, /* parameters */
		GTK_TYPE_STRING);

	file_browser_signals[SIG_BUSY] =
	gtk_signal_new ("busy",
		GTK_RUN_FIRST | GTK_RUN_NO_RECURSE,
		object_class->type,
		GTK_SIGNAL_OFFSET (GtkFileBrowserClass, busy),
		gtk_marshal_NONE__BOOL,
		GTK_TYPE_NONE,
		1, /* parameters */
		GTK_TYPE_BOOL);

	gtk_object_class_add_signals (object_class, file_browser_signals, SIG_LAST);

	klass->path_changed = NULL;
	klass->busy = NULL;
	klass->clicked = NULL;
}

static void
gtk_file_browser_init (GtkFileBrowser *file_browser)
{
	file_browser->use_icons = TRUE;
	file_browser->path = NULL;
}

GtkFileBrowser*
gtk_file_browser_new (void)
{
	GtkFileBrowser *file_browser;

	file_browser = gtk_type_new (gtk_file_browser_get_type());

	/*
		Type:Name:Size:Flags:Attr:Access,Modify,Change,Links,Uid,Gid,INode,Device
	*/

	gtk_clist_construct (GTK_CLIST(file_browser),13,titles);
	gtk_clist_set_selection_mode (GTK_CLIST(file_browser),GTK_SELECTION_EXTENDED);

	gtk_clist_set_column_justification (GTK_CLIST(file_browser),2,GTK_JUSTIFY_RIGHT);
 
	gtk_clist_set_column_width (GTK_CLIST(file_browser),0,30); // type
	gtk_clist_set_column_width (GTK_CLIST(file_browser),1,200);
	gtk_clist_set_column_width (GTK_CLIST(file_browser),2,70);
	gtk_clist_set_column_width (GTK_CLIST(file_browser),3,120);
	gtk_clist_set_column_width (GTK_CLIST(file_browser),4,120);
	gtk_clist_set_column_width (GTK_CLIST(file_browser),5,4);
	gtk_clist_set_column_width (GTK_CLIST(file_browser),6,4);
	gtk_clist_set_column_width (GTK_CLIST(file_browser),7,4);
	gtk_clist_set_column_width (GTK_CLIST(file_browser),8,4);
	gtk_clist_set_column_width (GTK_CLIST(file_browser),9,90);
	gtk_clist_set_column_width (GTK_CLIST(file_browser),10,90);

	file_browser->use_icons = TRUE;

	gtk_signal_connect (GTK_OBJECT(GTK_CLIST(file_browser)), "select_row",
		GTK_SIGNAL_FUNC (on_select_row), NULL);

	return GTK_FILE_BROWSER(file_browser);
}

static void
gtk_file_browser_destroy (GtkObject *object)
{
	GtkFileBrowser *file_browser;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GTK_IS_FILE_BROWSER(object));
    
	file_browser = GTK_FILE_BROWSER(object);

	g_free (file_browser->path);

	if (GTK_OBJECT_CLASS (parent_class)->destroy)
	 (* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}

static void
gtk_file_browser_set_arg (GtkObject *object,
	GtkArg *arg,
	guint arg_id)
{
	GtkFileBrowser *file_browser;

	file_browser = GTK_FILE_BROWSER (object);
  
	switch (arg_id)
	{
		case ARG_USE_ICONS:
			gtk_file_browser_set_use_icons (file_browser, GTK_VALUE_BOOL (*arg));
			break;
		case ARG_PATH:
			gtk_file_browser_set_path (file_browser, GTK_VALUE_STRING (*arg));
			break;
		default:
			break;
	}
}

static void
gtk_file_browser_get_arg (GtkObject *object,
	GtkArg *arg,
	guint arg_id)
{
	GtkFileBrowser *file_browser;

	file_browser = GTK_FILE_BROWSER (object);
  
	switch (arg_id)
	{
		case ARG_USE_ICONS:
			GTK_VALUE_BOOL (*arg) = file_browser->use_icons;
			break;
		case ARG_PATH:
			GTK_VALUE_STRING (*arg) = file_browser->path;
			break;
		default:
			arg->type = GTK_TYPE_INVALID;
			break;
	}
}

/* public interface */

void 
gtk_file_browser_set_use_icons (GtkFileBrowser *file_browser, gboolean arg)
{
	g_return_if_fail (file_browser != NULL);
	g_return_if_fail (GTK_IS_FILE_BROWSER (file_browser));

	file_browser->use_icons = arg;
}

gboolean
gtk_file_browser_get_use_icons (GtkFileBrowser *file_browser)
{
	g_return_val_if_fail (file_browser != NULL, FALSE);
	g_return_val_if_fail (GTK_IS_FILE_BROWSER (file_browser), FALSE);

	return file_browser->use_icons;
}

void
gtk_file_browser_path_changed (GtkFileBrowser *file_browser)
{
	g_return_if_fail (file_browser != NULL);
	g_return_if_fail (GTK_IS_FILE_BROWSER (file_browser));

	rescan (file_browser);

	gtk_signal_emit (GTK_OBJECT (file_browser),
		file_browser_signals[SIG_PATH_CHANGED], file_browser->path, NULL);
}

void
gtk_file_browser_set_path (GtkFileBrowser *file_browser, const gchar *pathname)
{
	g_return_val_if_fail (file_browser != NULL, FALSE);
	g_return_val_if_fail (GTK_IS_FILE_BROWSER (file_browser), FALSE);

	if (access(pathname,X_OK)) return;

	{
		int old = chdir (pathname);
		gchar *path;
		
		if (old!=0) return;

		path = g_get_current_dir ();
			
		g_free (file_browser->path);
		file_browser->path = g_strconcat (path,G_DIR_SEPARATOR_S,NULL);
		g_free (path);
		
		fchdir (old);
	}

	gtk_file_browser_path_changed (file_browser);
}

const gchar *
gtk_file_browser_get_path (GtkFileBrowser *file_browser)
{
	g_return_val_if_fail (file_browser != NULL, NULL);
	g_return_val_if_fail (GTK_IS_FILE_BROWSER (file_browser), NULL);

	return file_browser->path;
}
