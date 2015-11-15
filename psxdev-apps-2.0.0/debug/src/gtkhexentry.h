/* $Id: gtkhexentry.h,v 1.1.1.1 2001/05/17 11:50:27 dbalster Exp $ */

/*
 * GtkHexEntry - 1..8 digit simple hex entry/display
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

#ifndef __GTK_HEX_ENTRY_H__
#define __GTK_HEX_ENTRY_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define GTK_TYPE_HEX_ENTRY          (gtk_hex_entry_get_type())
#define GTK_HEX_ENTRY(obj)          (GTK_CHECK_CAST ((obj), GTK_TYPE_HEX_ENTRY, GtkHexEntry))
#define GTK_HEX_ENTRY_CLASS(klass)  (GTK_CHECK_CLASS_CAST (klass, GTK_TYPE_HEX_ENTRY, GtkHexEntryClass))
#define GTK_IS_HEX_ENTRY(obj)       (GTK_CHECK_TYPE ((obj), GTK_TYPE_HEX_ENTRY))
#define GTK_IS_HEX_ENTRY_CLASS      (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_HEX_ENTRY))

typedef struct _GtkHexEntry			GtkHexEntry;
typedef struct _GtkHexEntryClass	GtkHexEntryClass;

struct _GtkHexEntry
{
	GtkWidget widget; /* parent object data */

	// everything is private, don't use it!
	GdkPixmap *pixmap;
	GdkCursor *cursor;
	GdkGC *gc;
	GdkFont *font;
	gint digits;
	gchar *buffer;
	guint32 timer;
	gint button;
	gint selection_start_pos;
	gint selection_end_pos;
	gboolean has_selection;
	GdkColor cursor_color;
	gint cursor_position;
	gboolean editable;
	gboolean selected;
	gboolean modified;
};

struct _GtkHexEntryClass
{
	GtkWidgetClass parent_class; /* parent class data */
	
	void (* value_changed) (GtkHexEntry *hex_entry);
	void (* activate) (GtkHexEntry *hex_entry);
};

GtkType    gtk_hex_entry_get_type   (void);
GtkWidget* gtk_hex_entry_new        (guint digits, gulong value);

void gtk_hex_entry_value_changed (GtkHexEntry *hex_entry);
void gtk_hex_entry_set_value (GtkHexEntry *hex_entry, gulong arg);
gulong gtk_hex_entry_get_value (GtkHexEntry *hex_entry);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __GTK_HEX_ENTRY_H__ */
