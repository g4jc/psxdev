/* $Id: gtkr3kdisasm.h,v 1.1.1.1 2001/05/17 11:50:27 dbalster Exp $ */

/*
 *	GTK+/PSXDEV debugger widgets
 *	
 *	Copyright (C) 1998,1999,2000 Daniel Balster (dbalster@psxdev.de)
 *	http://www.psxdev.de
 *	
 *	This is not free software.
 */

#ifndef __GTK_R3K_DISASM_H__
#define __GTK_R3K_DISASM_H__

#include <gtk/gtk.h>
#include <gtkmemory.h>
#include <gtkbreakpoints.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define GTK_TYPE_R3K_DISASM          (gtk_r3k_disasm_get_type())
#define GTK_R3K_DISASM(obj)          (GTK_CHECK_CAST ((obj), GTK_TYPE_R3K_DISASM, GtkR3kDisasm))
#define GTK_R3K_DISASM_CLASS(klass)  (GTK_CHECK_CLASS_CAST (klass, GTK_TYPE_R3K_DISASM, GtkR3kDisasmClass))
#define GTK_IS_R3K_DISASM(obj)       (GTK_CHECK_TYPE ((obj), GTK_TYPE_R3K_DISASM))
#define GTK_IS_R3K_DISASM_CLASS      (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_R3K_DISASM))

typedef struct _GtkR3kDisasm         GtkR3kDisasm;
typedef struct _GtkR3kDisasmClass    GtkR3kDisasmClass;

struct _GtkR3kDisasm
{
	GtkWidget widget; /* parent object data */

	GtkMemory *memory;
	GtkBreakpoints *breakpoints;

	GdkPixmap *pixmap;
//	GdkCursor *cursor;
	GdkGC *gc;
	GdkFont *font, *font_bold;
	guint32 timer;
	
	GdkColor red,blue,green;
	
	GdkCursor *branch_cursor;
	
	guint cw, ch;
	guint rows, cols;
	
	gulong cursor;
	
	gint position;
	
	gint button;
	
	gulong pointer;

	gulong *targets;
	
	GSList *stack;

	gboolean over_link;

	GdkPixmap *icons;
	GdkBitmap *masks;
};

struct _GtkR3kDisasmClass
{
	GtkWidgetClass parent_class; /* parent class data */

	void (* pointer_changed) (GtkR3kDisasm *r3k_disasm);
	void (* address_changed) (GtkR3kDisasm *r3k_disasm);
	void (* cursor_changed) (GtkR3kDisasm *r3k_disasm);

	void (* enter_branch_notify) (GtkR3kDisasm *r3k_disasm);
	void (* leave_branch_notify) (GtkR3kDisasm *r3k_disasm);

	void (* link_pressed) (GtkR3kDisasm *r3k_disasm);
	void (* link_released) (GtkR3kDisasm *r3k_disasm);
	void (* link_clicked) (GtkR3kDisasm *r3k_disasm);
};

GtkType gtk_r3k_disasm_get_type (void);
GtkWidget *gtk_r3k_disasm_new (GtkMemory *memory, GtkBreakpoints *breakpoints);

void gtk_r3k_disasm_address_changed (GtkR3kDisasm *r3k_disasm);
void gtk_r3k_disasm_set_address (GtkR3kDisasm *r3k_disasm, gulong addr);
gulong gtk_r3k_disasm_get_address (GtkR3kDisasm *r3k_disasm);

void gtk_r3k_disasm_pointer_changed (GtkR3kDisasm *r3k_disasm);
void gtk_r3k_disasm_set_pointer (GtkR3kDisasm *r3k_disasm, gulong addr);
gulong gtk_r3k_disasm_get_pointer (GtkR3kDisasm *r3k_disasm);

void gtk_r3k_disasm_cursor_changed (GtkR3kDisasm *r3k_disasm);
void gtk_r3k_disasm_set_cursor (GtkR3kDisasm *r3k_disasm, gulong addr);
gulong gtk_r3k_disasm_get_cursor (GtkR3kDisasm *r3k_disasm);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __GTK_R3K_DISASM_H__ */
