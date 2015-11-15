/* $Id: gtkr3kdisasm.c,v 1.1.1.1 2001/05/17 11:50:27 dbalster Exp $ */

/*
 *	GTK+/PSXDEV debugger widgets
 *	
 *	Copyright (C) 1998,1999,2000 Daniel Balster (dbalster@psxdev.de)
 *	http://www.psxdev.de
 *	
 *	This is not free software.
 */

/*
	TODO:
	
	- enter/leave link callbacks and painting

	on_branch_enter_notify (source, target)
	on_branch_leave_notify (source, target)
	on_branch_clicked (source, target)

	- enter/leave register and painting

	on_register_enter_notify (regnum, coprocessor)
	on_register_leave_notify (regnum, coprocessor)
	on_register_clicked  (regnum, coprocessor)

	- memory
	
	on_read_memory
	on_write_memory
		
	on_address_changed

---------------------------------------------------------------

	TODO:
	
	more connection with GtkMemory object!

	Cursors: pointer_over_link

	paint algorithm:
	
	
	show_address
	show_hex
	show_ascii
	show_icons
	show_insns
*/

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include <gtkr3kdisasm.h>
#include <r3k_disasm.h>

#include <ctype.h>
#include <stdio.h>

enum {
	ARG_0,
	ARG_ADDRESS,
	ARG_POINTER,
	ARG_CURSOR,
};

enum {
	SIG_POINTER_CHANGED,
	SIG_ADDRESS_CHANGED,
	SIG_CURSOR_CHANGED,
	SIG_ENTER_BRANCH_NOTIFY,
	SIG_LEAVE_BRANCH_NOTIFY,
	SIG_INSERT_BREAKPOINT,
	SIG_REMOVE_BREAKPOINT,
	SIG_LAST
};

#define BUFFER(xx) ((gulong*)(xx)->memory->buffer)

static void gtk_r3k_disasm_class_init (GtkR3kDisasmClass *klass);
static void gtk_r3k_disasm_init (GtkR3kDisasm *r3k_disasm);
static void gtk_r3k_disasm_finalize (GtkObject *object);
static void gtk_r3k_disasm_destroy (GtkObject *object);
static void gtk_r3k_disasm_set_arg (GtkObject *object, GtkArg *arg, guint arg_id);
static void gtk_r3k_disasm_get_arg (GtkObject *object, GtkArg *arg, guint arg_id);

static void gtk_r3k_disasm_realize (GtkWidget *widget);
static void gtk_r3k_disasm_unrealize (GtkWidget *widget);
static void gtk_r3k_disasm_draw (GtkWidget *widget, GdkRectangle *area);
static gint gtk_r3k_disasm_expose (GtkWidget *widget, GdkEventExpose *event);
static void gtk_r3k_disasm_paint (GtkWidget *widget, GdkRectangle *area);
static gint gtk_r3k_disasm_button_press (GtkWidget *widget, GdkEventButton *event);
static gint gtk_r3k_disasm_button_release (GtkWidget *widget, GdkEventButton *event);
static gint gtk_r3k_disasm_motion (GtkWidget *widget, GdkEventMotion *event);
static gint gtk_r3k_disasm_configure_event (GtkWidget *widget, GdkEventConfigure *event);
static void gtk_r3k_disasm_size_request (GtkWidget *widget, GtkRequisition *requisition);
static void gtk_r3k_disasm_size_allocate (GtkWidget *widget, GtkAllocation *allocation);
static gint gtk_r3k_disasm_key_press (GtkWidget *widget, GdkEventKey *event);
static gint gtk_r3k_disasm_focus_in (GtkWidget *widget, GdkEventFocus *event);
static gint gtk_r3k_disasm_focus_out (GtkWidget *widget, GdkEventFocus *event);
static gint gtk_r3k_disasm_enter_notify (GtkWidget *widget, GdkEventCrossing *event);
static gint gtk_r3k_disasm_leave_notify (GtkWidget *widget, GdkEventCrossing *event);

static GtkWidgetClass *parent_class = NULL;
static guint r3k_disasm_signals[SIG_LAST] = { 0 };

/* this is not very maintainable. should split them up into single icons */

#include "icons.xpm"

#define ICON_W	16
#define ICON_H	14

#define ICON_HERE		0
#define ICON_STOP		16
#define ICON_UP_TGT		32
#define ICON_DOWN_TGT	48
#define ICON_UP_SRC		64
#define ICON_DOWN_SRC	80

#define TWENTY	ICON_W
#define TWENTY2	(TWENTY-5)

#define CY  4
//((r3k_disasm->ch-ICON_H)/2)

GtkType
gtk_r3k_disasm_get_type (void)
{
	static GtkType r3k_disasm_type = 0;

	if (!r3k_disasm_type)
	{
		GtkTypeInfo r3k_disasm_info =
		{
			(gchar*)"GtkR3kDisasm",
			sizeof (GtkR3kDisasm),
			sizeof (GtkR3kDisasmClass),
			(GtkClassInitFunc) gtk_r3k_disasm_class_init,
			(GtkObjectInitFunc) gtk_r3k_disasm_init,
			/* reserved_1 */ NULL,
			/* reserved_2 */ NULL,
			(GtkClassInitFunc) NULL
		};
		r3k_disasm_type = gtk_type_unique (gtk_widget_get_type (), &r3k_disasm_info);
	}

	return r3k_disasm_type;
}

static void
gtk_r3k_disasm_class_init (GtkR3kDisasmClass *klass)
{
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;

	parent_class = gtk_type_class(gtk_widget_get_type());
	object_class = (GtkObjectClass*) klass;
	widget_class = (GtkWidgetClass*) klass;

	object_class->destroy = gtk_r3k_disasm_destroy;
	object_class->finalize = gtk_r3k_disasm_finalize;
	object_class->set_arg = gtk_r3k_disasm_set_arg;
	object_class->get_arg = gtk_r3k_disasm_get_arg;

	widget_class->realize = gtk_r3k_disasm_realize;
	widget_class->unrealize = gtk_r3k_disasm_unrealize;
	widget_class->draw = gtk_r3k_disasm_draw;
	widget_class->configure_event = gtk_r3k_disasm_configure_event;
	widget_class->expose_event = gtk_r3k_disasm_expose;
	widget_class->button_press_event = gtk_r3k_disasm_button_press;
	widget_class->button_release_event = gtk_r3k_disasm_button_release;
	widget_class->motion_notify_event = gtk_r3k_disasm_motion;
	widget_class->size_request = gtk_r3k_disasm_size_request;
	widget_class->size_allocate = gtk_r3k_disasm_size_allocate;
	widget_class->key_press_event = gtk_r3k_disasm_key_press;
	widget_class->focus_out_event = gtk_r3k_disasm_focus_out;
	widget_class->focus_in_event = gtk_r3k_disasm_focus_in;
	widget_class->enter_notify_event = gtk_r3k_disasm_enter_notify;
	widget_class->leave_notify_event = gtk_r3k_disasm_leave_notify;

	/* register arguments */

	gtk_object_add_arg_type ("GtkR3kDisasm::address",
		GTK_TYPE_ULONG,
		GTK_ARG_READWRITE,
		ARG_ADDRESS);

	gtk_object_add_arg_type ("GtkR3kDisasm::pointer",
		GTK_TYPE_ULONG,
		GTK_ARG_READWRITE,
		ARG_POINTER);

	gtk_object_add_arg_type ("GtkR3kDisasm::cursor",
		GTK_TYPE_ULONG,
		GTK_ARG_READWRITE,
		ARG_CURSOR);

	/* register signals */

	r3k_disasm_signals[SIG_ADDRESS_CHANGED] =
	gtk_signal_new ("address_changed",
		GTK_RUN_FIRST | GTK_RUN_NO_RECURSE,
		object_class->type,
		GTK_SIGNAL_OFFSET (GtkR3kDisasmClass, address_changed),
		gtk_marshal_NONE__NONE,
		GTK_TYPE_NONE, 0);

	r3k_disasm_signals[SIG_CURSOR_CHANGED] =
	gtk_signal_new ("cursor_changed",
		GTK_RUN_FIRST | GTK_RUN_NO_RECURSE,
		object_class->type,
		GTK_SIGNAL_OFFSET (GtkR3kDisasmClass, cursor_changed),
		gtk_marshal_NONE__NONE,
		GTK_TYPE_NONE, 0);

	r3k_disasm_signals[SIG_POINTER_CHANGED] =
	gtk_signal_new ("pointer_changed",
		GTK_RUN_FIRST | GTK_RUN_NO_RECURSE,
		object_class->type,
		GTK_SIGNAL_OFFSET (GtkR3kDisasmClass, pointer_changed),
		gtk_marshal_NONE__NONE,
		GTK_TYPE_NONE, 0);

	r3k_disasm_signals[SIG_ENTER_BRANCH_NOTIFY] =
	gtk_signal_new ("enter_branch_notify",
		GTK_RUN_FIRST | GTK_RUN_NO_RECURSE,
		object_class->type,
		GTK_SIGNAL_OFFSET (GtkR3kDisasmClass, enter_branch_notify),
		gtk_marshal_NONE__NONE,
		GTK_TYPE_NONE, 0);

	r3k_disasm_signals[SIG_LEAVE_BRANCH_NOTIFY] =
	gtk_signal_new ("leave_branch_notify",
		GTK_RUN_FIRST | GTK_RUN_NO_RECURSE,
		object_class->type,
		GTK_SIGNAL_OFFSET (GtkR3kDisasmClass, leave_branch_notify),
		gtk_marshal_NONE__NONE,
		GTK_TYPE_NONE, 0);

	gtk_object_class_add_signals (object_class, r3k_disasm_signals, SIG_LAST);

	klass->address_changed = NULL;
	klass->pointer_changed = NULL;
	klass->cursor_changed = NULL;
	klass->enter_branch_notify = NULL;
	klass->leave_branch_notify = NULL;
}

static gint
queue_draw_timeout (gpointer data)
{
	GtkR3kDisasm *r3k_disasm;

	GDK_THREADS_ENTER ();

	r3k_disasm = GTK_R3K_DISASM (data);
	r3k_disasm->timer = 0;
	gtk_r3k_disasm_paint (GTK_WIDGET(r3k_disasm),NULL);

	GDK_THREADS_LEAVE ();

	return FALSE;
}

static void
queue_draw (GtkR3kDisasm *r3k_disasm)
{
	g_return_if_fail (r3k_disasm != NULL);
	g_return_if_fail (GTK_IS_R3K_DISASM (r3k_disasm));

	if (!r3k_disasm->timer)
		r3k_disasm->timer = gtk_timeout_add (20, queue_draw_timeout, r3k_disasm);
}

/* draw masked pixmap */

static void
draw_pixmap (GdkDrawable *drawable,
	GdkGC *gc,
	GdkDrawable *src,
	GdkBitmap *mask,
	gint xsrc,
	gint ysrc,
	gint xdest,
	gint ydest,
	gint width,
	gint height)
{
	gdk_gc_set_clip_mask (gc, mask);
	gdk_gc_set_clip_origin (gc,xdest-xsrc,ydest);
	gdk_draw_pixmap (drawable,gc,src,xsrc,ysrc,xdest,ydest,width,height);
	gdk_gc_set_clip_mask (gc, NULL);
	gdk_gc_set_clip_origin (gc, 0, 0);
}

static void
make_pixmap (GtkR3kDisasm *r3k_disasm)
{
	GtkWidget *widget;

	g_return_if_fail (r3k_disasm != NULL);
	g_return_if_fail (GTK_IS_R3K_DISASM (r3k_disasm));

	if (GTK_WIDGET_REALIZED (r3k_disasm))
	{
		widget = GTK_WIDGET (r3k_disasm);

		if (r3k_disasm->pixmap)
			gdk_pixmap_unref (r3k_disasm->pixmap);

		r3k_disasm->pixmap = gdk_pixmap_new (widget->window,
			widget->allocation.width, widget->allocation.height, -1);
	}
}

static void
gtk_r3k_disasm_init (GtkR3kDisasm *r3k_disasm)
{
	GTK_WIDGET_SET_FLAGS (r3k_disasm, GTK_CAN_FOCUS);

	r3k_disasm->memory = NULL;
	r3k_disasm->breakpoints = NULL;

	r3k_disasm->gc = NULL;
	r3k_disasm->pixmap = NULL;
	r3k_disasm->timer = 0;

	r3k_disasm->stack = NULL;

//	r3k_disasm->font = gdk_font_load ("-b&h-lucidatypewriter-medium-r-normal-*-*-120-*-*-m-*-iso8859-1");
//	r3k_disasm->font_bold = gdk_font_load ("-b&h-lucidatypewriter-bold-r-normal-*-*-120-*-*-m-*-iso8859-1");

//	r3k_disasm->font = gdk_font_load ("unknown-fixedsys-normal-r-normal-*-*-110-*-*-m-*-iso8859-1");
//	r3k_disasm->font_bold = gdk_font_load ("unknown-fixedsys-normal-r-normal-*-*-110-*-*-m-*-iso8859-1");

//	if (r3k_disasm->font==NULL)
	{
		r3k_disasm->font = gdk_font_load ("-b&h-lucidatypewriter-medium-r-normal-*-*-140-*-*-m-*-iso8859-1");
		r3k_disasm->font_bold = gdk_font_load ("-b&h-lucidatypewriter-bold-r-normal-*-*-140-*-*-m-*-iso8859-1");
	}

	r3k_disasm->cw = gdk_char_width (r3k_disasm->font,'0');
	r3k_disasm->ch = r3k_disasm->font->ascent + r3k_disasm->font->descent;

	gdk_color_parse ("red",&r3k_disasm->red);
	gdk_color_parse ("blue",&r3k_disasm->blue);
	gdk_color_parse ("#555555",&r3k_disasm->green);

	r3k_disasm->branch_cursor = gdk_cursor_new(GDK_PLUS);
}

static void
gtk_r3k_disasm_finalize (GtkObject *object)
{
	g_return_if_fail (object != NULL);
	g_return_if_fail (GTK_IS_R3K_DISASM (object));

	if (GTK_R3K_DISASM (object)->pixmap)
		gdk_pixmap_unref (GTK_R3K_DISASM (object)->pixmap);

	GTK_OBJECT_CLASS (parent_class)->finalize (object);
}

static gint
gtk_r3k_disasm_configure_event (GtkWidget *widget, GdkEventConfigure *event)
{
	GtkR3kDisasm *r3k_disasm;

	g_return_val_if_fail (widget != NULL,FALSE);
	g_return_val_if_fail (GTK_IS_R3K_DISASM (widget),FALSE);

	r3k_disasm = GTK_R3K_DISASM (widget);

	if (r3k_disasm->pixmap) gdk_pixmap_unref (r3k_disasm->pixmap);
	r3k_disasm->pixmap = NULL;

	return FALSE;
}

static void
gtk_r3k_disasm_realize (GtkWidget *widget)
{
	GtkR3kDisasm *r3k_disasm;
	GdkWindowAttr attributes;
	gint attributes_mask;

	g_return_if_fail (widget != NULL);
	g_return_if_fail (GTK_IS_R3K_DISASM (widget));
  
	GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);
	r3k_disasm = GTK_R3K_DISASM (widget);

	attributes.x = widget->allocation.x;
	attributes.y = widget->allocation.y;
	attributes.width = widget->allocation.width;
	attributes.height = widget->allocation.height;
	attributes.window_type = GDK_WINDOW_CHILD;
	attributes.wclass = GDK_INPUT_OUTPUT;
	attributes.visual = gtk_widget_get_visual (widget);
	attributes.colormap = gtk_widget_get_colormap (widget);

	attributes.event_mask = gtk_widget_get_events (widget)
	| GDK_EXPOSURE_MASK | GDK_KEY_PRESS_MASK
	| GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK
	| GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK
	| GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK;

	attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;
  
	widget->window = gdk_window_new (gtk_widget_get_parent_window (widget),
		&attributes, attributes_mask);
	gdk_window_set_user_data (widget->window, r3k_disasm);

	widget->style = gtk_style_attach (widget->style, widget->window);

	gtk_style_set_background (widget->style, widget->window, GTK_STATE_NORMAL);
	gdk_window_set_background (widget->window, &widget->style->base[GTK_WIDGET_STATE (widget)]);
	gdk_window_set_back_pixmap (widget->window, NULL, TRUE);
	
	r3k_disasm->gc = gdk_gc_new (widget->window);

	gdk_color_alloc (gtk_widget_get_colormap (widget),&r3k_disasm->red);
	gdk_color_alloc (gtk_widget_get_colormap (widget),&r3k_disasm->blue);
	gdk_color_alloc (gtk_widget_get_colormap (widget),&r3k_disasm->green);

	gdk_gc_set_foreground (r3k_disasm->gc,&r3k_disasm->red);

	r3k_disasm->icons = gdk_pixmap_create_from_xpm_d (widget->window,
		&r3k_disasm->masks, NULL, icons_xpm);

	make_pixmap (r3k_disasm);
}

static void
gtk_r3k_disasm_size_request (GtkWidget *widget, GtkRequisition *requisition)
{
	GtkR3kDisasm *r3k_disasm;

	g_return_if_fail (widget != NULL);
	g_return_if_fail (GTK_IS_R3K_DISASM (widget));
	g_return_if_fail (requisition != NULL);

	r3k_disasm = GTK_R3K_DISASM (widget);

	r3k_disasm->cols = 10; //widget->allocation.width / r3k_disasm->cw;
	r3k_disasm->rows = widget->allocation.height / r3k_disasm->ch;

	requisition->width  = r3k_disasm->cols * r3k_disasm->cw;
	requisition->height = r3k_disasm->rows * r3k_disasm->ch;

	g_free (r3k_disasm->targets);
	r3k_disasm->targets = g_new0 (gulong,r3k_disasm->rows);

	if (r3k_disasm->pixmap) gdk_pixmap_unref (r3k_disasm->pixmap);
	r3k_disasm->pixmap = NULL;

	gtk_memory_set_size (r3k_disasm->memory, r3k_disasm->rows*4);

//	gtk_r3k_disasm_address_changed (r3k_disasm);
}

static void
gtk_r3k_disasm_size_allocate (GtkWidget *widget, GtkAllocation *allocation)
{
	GtkR3kDisasm *r3k_disasm;

	g_return_if_fail (widget != NULL);
	g_return_if_fail (GTK_IS_R3K_DISASM (widget));
	g_return_if_fail (allocation != NULL);

	widget->allocation = *allocation;
	r3k_disasm = GTK_R3K_DISASM (widget);

	r3k_disasm->cols = allocation->width / r3k_disasm->cw;
	r3k_disasm->rows = allocation->height / r3k_disasm->ch;

	allocation->width = r3k_disasm->cols * r3k_disasm->cw;
	allocation->height = r3k_disasm->rows * r3k_disasm->ch;

	g_free (r3k_disasm->targets);
	r3k_disasm->targets = g_new0 (gulong,r3k_disasm->rows);

	if (r3k_disasm->pixmap) gdk_pixmap_unref (r3k_disasm->pixmap);
	r3k_disasm->pixmap = NULL;

	gtk_memory_set_size (r3k_disasm->memory, r3k_disasm->rows*4);
//	gtk_r3k_disasm_address_changed (r3k_disasm);

	if (GTK_WIDGET_REALIZED (widget))
	{
		gdk_window_move_resize (widget->window,
			allocation->x, allocation->y,
			allocation->width, allocation->height);

		gtk_r3k_disasm_paint (GTK_WIDGET(r3k_disasm),NULL);
	}
}

static void
gtk_r3k_disasm_unrealize (GtkWidget *widget)
{
	GtkR3kDisasm *r3k_disasm;

	g_return_if_fail (widget != NULL);
	g_return_if_fail (GTK_IS_R3K_DISASM (widget));

	r3k_disasm = GTK_R3K_DISASM (widget);

	gdk_gc_destroy (r3k_disasm->gc);
	r3k_disasm->gc = NULL;

	if (GTK_WIDGET_CLASS (parent_class)->unrealize)
	(* GTK_WIDGET_CLASS (parent_class)->unrealize) (widget);
}

static gint
gtk_r3k_disasm_focus_in (GtkWidget *widget, GdkEventFocus *event)
{
	GtkR3kDisasm *r3k_disasm;

	g_return_val_if_fail (widget != NULL, FALSE);
	g_return_val_if_fail (GTK_IS_R3K_DISASM (widget), FALSE);
	g_return_val_if_fail (event != NULL, FALSE);

	r3k_disasm = GTK_R3K_DISASM (widget);

	GTK_WIDGET_SET_FLAGS (widget, GTK_HAS_FOCUS);

//	widget->state = GTK_STATE_ACTIVE;

	queue_draw (r3k_disasm);

	return FALSE;
}

static gint
gtk_r3k_disasm_focus_out (GtkWidget *widget, GdkEventFocus *event)
{
	GtkR3kDisasm *r3k_disasm;

	g_return_val_if_fail (widget != NULL, FALSE);
	g_return_val_if_fail (GTK_IS_R3K_DISASM (widget), FALSE);
	g_return_val_if_fail (event != NULL, FALSE);

	r3k_disasm = GTK_R3K_DISASM (widget);

	GTK_WIDGET_UNSET_FLAGS (widget, GTK_HAS_FOCUS);

//	widget->state = GTK_STATE_NORMAL;

	queue_draw (r3k_disasm);

	return FALSE;
}

static gint
gtk_r3k_disasm_enter_notify (GtkWidget *widget, GdkEventCrossing *event)
{
	GtkR3kDisasm *r3k_disasm;

	g_return_val_if_fail (widget != NULL, FALSE);
	g_return_val_if_fail (GTK_IS_R3K_DISASM (widget), FALSE);
	g_return_val_if_fail (event != NULL, FALSE);

	r3k_disasm = GTK_R3K_DISASM (widget);

//	widget->state = GTK_WIDGET_HAS_FOCUS(widget) ? GTK_STATE_ACTIVE : GTK_STATE_PRELIGHT;

	queue_draw (r3k_disasm);
	
	return FALSE;
}

static gint
gtk_r3k_disasm_leave_notify (GtkWidget *widget, GdkEventCrossing *event)
{
	GtkR3kDisasm *r3k_disasm;

	g_return_val_if_fail (widget != NULL, FALSE);
	g_return_val_if_fail (GTK_IS_R3K_DISASM (widget), FALSE);
	g_return_val_if_fail (event != NULL, FALSE);

	r3k_disasm = GTK_R3K_DISASM (widget);

	r3k_disasm->over_link = FALSE;
	r3k_disasm->position = -1;
//	widget->state = GTK_STATE_NORMAL;

	queue_draw (r3k_disasm);
	
	return FALSE;
}

static void
gtk_r3k_disasm_paint (GtkWidget *widget,
	GdkRectangle *area)
{
	GtkR3kDisasm *r3k_disasm;
	GdkRectangle def_area;

	g_return_if_fail (widget != NULL);
	g_return_if_fail (GTK_IS_R3K_DISASM (widget));

	r3k_disasm = GTK_R3K_DISASM (widget);

	if (area == NULL)
	{
		area = &def_area;
		area->x = area->y = 0;
		area->width = widget->allocation.width;
		area->height = widget->allocation.height;
	}

	if (r3k_disasm->pixmap==NULL)
		r3k_disasm->pixmap = gdk_pixmap_new (widget->window,
			widget->allocation.width,
			widget->allocation.height, -1);

	if (GTK_WIDGET_DRAWABLE (widget))
	{
		guint x,y;
		guint i;
		gulong addr;
		GtkBreakpoint *bkpt;
	
		gdk_draw_rectangle (r3k_disasm->pixmap,widget->style->bg_gc[GTK_WIDGET_STATE (widget)],TRUE,
		0,0,widget->allocation.width,widget->allocation.height);

		if (BUFFER(r3k_disasm))
		{
			x = 4;
			y = r3k_disasm->font->ascent;

			addr = gtk_memory_get_address (r3k_disasm->memory);
			for (i=0; i<r3k_disasm->rows; i++)
			{
				gchar buf[200];
				gint len;
				GdkGC *gc;
				GdkFont *font;
				opcode_t *opcode;
				gulong ab = BUFFER(r3k_disasm)[i];
				gchar *cd = (gchar*) &ab;
				insn_t insn;

				len = sprintf (buf,"%08lx:%08lx %c%c%c%c",addr,BUFFER(r3k_disasm)[i],
				isprint(cd[0])?cd[0]:'.',
				isprint(cd[1])?cd[1]:'.',
				isprint(cd[2])?cd[2]:'.',
				isprint(cd[3])?cd[3]:'.'
				);

				x = 4;

				if (r3k_disasm->cursor == addr)
				{
					font = r3k_disasm->font_bold;
					gc = widget->style->fg_gc[GTK_STATE_PRELIGHT];
				}
				else
				{
					font = r3k_disasm->font;
					gc = widget->style->fg_gc[GTK_WIDGET_STATE (widget)];
				}

		//		font = r3k_disasm->font_bold;
		//		gc = r3k_disasm->gc;
		//		gdk_gc_set_foreground (gc,&r3k_disasm->green);

				gdk_draw_string (r3k_disasm->pixmap,font,gc,x,y,buf);
				x += (len+5)*r3k_disasm->cw;

				if (r3k_disasm->pointer == addr)
				{
					draw_pixmap (r3k_disasm->pixmap,widget->style->black_gc,
						r3k_disasm->icons,r3k_disasm->masks,
						ICON_HERE,0,x-TWENTY,CY + y-r3k_disasm->ch,ICON_W,ICON_H);
				}

				insn = (insn_t)BUFFER(r3k_disasm)[i];

				bkpt = gtk_breakpoints_find (r3k_disasm->breakpoints,addr);
				
				if (bkpt)
				{
					insn = (insn_t) bkpt->safe;

					draw_pixmap (r3k_disasm->pixmap,widget->style->black_gc,
						r3k_disasm->icons,r3k_disasm->masks,
						ICON_STOP,0,x-TWENTY,CY + y-r3k_disasm->ch,ICON_W,ICON_H);
				}

				opcode = r3k_decode (buf,200,addr,insn);

				r3k_disasm->targets[i] = 0x00000000;
				if (opcode) if (opcode->has_branch_target)
				{
					r3k_disasm->targets[i] = opcode->branch_target;
					
					if (r3k_disasm->over_link && (i==r3k_disasm->position))
					{
						gshort y2,y3=y;

						draw_pixmap (r3k_disasm->pixmap,widget->style->black_gc,
							r3k_disasm->icons,r3k_disasm->masks,
							opcode->branch_target<addr ?
							ICON_UP_SRC : ICON_DOWN_SRC,
							0,x-TWENTY,CY + y-r3k_disasm->ch,ICON_W,ICON_H);

						if ((opcode->branch_target>=gtk_memory_get_address (r3k_disasm->memory)) &&
						(opcode->branch_target<=gtk_memory_get_address (r3k_disasm->memory)+(r3k_disasm->rows*4))
						)
						{
							y2 = 1+((opcode->branch_target-gtk_memory_get_address (r3k_disasm->memory))>>2);
							y2 *= r3k_disasm->ch;

							draw_pixmap (r3k_disasm->pixmap,widget->style->black_gc,
								r3k_disasm->icons,r3k_disasm->masks,
								opcode->branch_target<addr ?
								ICON_UP_TGT : ICON_DOWN_TGT,
								0,x-TWENTY,CY + y2,ICON_W,ICON_H);
						}
						else
						{
							if (opcode->branch_target>=gtk_memory_get_address (r3k_disasm->memory))
							{
								y2=widget->allocation.height;
							}
							else y2 = -r3k_disasm->ch;
						}
						if (opcode->branch_target<addr)
						{
							y2 += r3k_disasm->ch;
							y3 -= r3k_disasm->ch;
						}

						gdk_gc_set_foreground (r3k_disasm->gc,&r3k_disasm->blue);

						gdk_draw_line (r3k_disasm->pixmap,widget->style->black_gc,
							x-TWENTY2,y3,x-TWENTY2,CY + y2);

						gdk_draw_line (r3k_disasm->pixmap,widget->style->black_gc,
							x-TWENTY2+2,y3,x-TWENTY2+2,CY + y2);

						gdk_draw_line (r3k_disasm->pixmap,r3k_disasm->gc,
							x-TWENTY2+1,y3,x-TWENTY2+1,CY + y2);
						
					}
				}

		//		g_strup (buf);

				gc = widget->style->fg_gc[GTK_WIDGET_STATE (widget)];

				if (r3k_disasm->over_link) if (i==r3k_disasm->position)
				if (r3k_disasm->targets[i])
				{
					gc = r3k_disasm->gc;
					gdk_gc_set_foreground (gc,&r3k_disasm->blue);
				}

				if (bkpt)
				{
					font = r3k_disasm->font_bold;
				}
				else
				{
					font = r3k_disasm->font;
				}

				if (opcode) if (opcode->coprocessor==2)
				{
					gc = r3k_disasm->gc;
					gdk_gc_set_foreground (r3k_disasm->gc,&r3k_disasm->red);
				}

				gdk_draw_string (r3k_disasm->pixmap,font,gc,x,y,buf);

				if (opcode) if (opcode->has_branch_target)
				{
					gdk_draw_line (r3k_disasm->pixmap,gc,
						x,
						y+font->descent-1,
						x+gdk_string_width(font,buf),
						y+font->descent-1);
				}

				addr+=4;
				y += r3k_disasm->ch;
			}
		}
	}
	
	gdk_draw_pixmap (widget->window, widget->style->fg_gc[GTK_STATE_ACTIVE],
		r3k_disasm->pixmap, area->x, area->y, area->x, area->y, area->width, area->height);
}

static void
gtk_r3k_disasm_draw (GtkWidget *widget,
	GdkRectangle *area)
{
	g_return_if_fail (widget != NULL);
	g_return_if_fail (GTK_IS_R3K_DISASM (widget));
	g_return_if_fail (area != NULL);

	if (GTK_WIDGET_DRAWABLE (widget))
		gtk_r3k_disasm_paint (widget, area);
}

static gint
gtk_r3k_disasm_expose (GtkWidget *widget,
	GdkEventExpose *event)
{
	GtkR3kDisasm *r3k_disasm;

	g_return_val_if_fail (widget != NULL, FALSE);
	g_return_val_if_fail (GTK_IS_R3K_DISASM (widget), FALSE);
	g_return_val_if_fail (event != NULL, FALSE);

	r3k_disasm = GTK_R3K_DISASM (widget);
	
	if (GTK_WIDGET_DRAWABLE (widget))
		gtk_r3k_disasm_paint (widget, &event->area);

	return FALSE;
}

static gint
gtk_r3k_disasm_key_press (GtkWidget *widget, GdkEventKey *event)
{
	GtkR3kDisasm *r3k_disasm;
	int key;
	gulong addr;

	g_return_val_if_fail (widget != NULL, FALSE);
	g_return_val_if_fail (GTK_IS_R3K_DISASM (widget), FALSE);
	g_return_val_if_fail (event != NULL, FALSE);

	r3k_disasm = GTK_R3K_DISASM (widget);

	key = event->keyval;

	switch (key)
	{
		case GDK_Up:
			addr = gtk_memory_get_address (r3k_disasm->memory);
			addr -=4;
			gtk_memory_set_address (r3k_disasm->memory, addr);
			break;
		case GDK_Down:
			addr = gtk_memory_get_address (r3k_disasm->memory);
			addr +=4;
			gtk_memory_set_address (r3k_disasm->memory, addr);
			break;
		case GDK_Page_Down:
			addr = gtk_memory_get_address (r3k_disasm->memory);
			addr += r3k_disasm->rows*4;
			gtk_memory_set_address (r3k_disasm->memory, addr);
			break;
		case GDK_Page_Up:
			addr = gtk_memory_get_address (r3k_disasm->memory);
			addr -= r3k_disasm->rows*4;
			gtk_memory_set_address (r3k_disasm->memory, addr);
			break;
		
		default:
			break;
	}

	queue_draw (r3k_disasm);

	return TRUE;
}

static gint
gtk_r3k_disasm_button_press (GtkWidget *widget, GdkEventButton *event)
{
	GtkR3kDisasm *r3k_disasm;

	g_return_val_if_fail (widget != NULL, FALSE);
	g_return_val_if_fail (GTK_IS_R3K_DISASM (widget), FALSE);

	r3k_disasm = GTK_R3K_DISASM (widget);

	if (!GTK_WIDGET_HAS_FOCUS (widget))
		gtk_widget_grab_focus (widget);

	r3k_disasm->button = event->button;

	if (event->button==1)
	{
		switch (event->type)
		{
			case GDK_BUTTON_PRESS:
			{
				guint y = event->y / r3k_disasm->ch;
				guint x = event->x / r3k_disasm->cw;
				
				// address and opcode field
				if (x < 20) break;
				
//				opcode_t *opcode;

//				opcode = r3k_find_opcode (r3k_disasm->address+(y*4),(insn_t)r3k_disasm->buffer[y]);
//		if (opcode)	if (opcode->has_branch_target)
//		{
//			r3k_disasm->over_link = TRUE;
//			gtk_signal_emit (GTK_OBJECT (r3k_disasm), r3k_disasm_signals[SIG_ENTER_BRANCH_NOTIFY]);
//			queue_draw (r3k_disasm);
//		}

//	g_print ("> %d %08lx %d\n",y,r3k_disasm->targets[y],r3k_disasm->over_link);

				if (r3k_disasm->targets[y] != 0)
				{
					r3k_disasm->stack = g_slist_prepend (r3k_disasm->stack,
						GINT_TO_POINTER(gtk_memory_get_address (r3k_disasm->memory)));

					gtk_memory_set_address (r3k_disasm->memory,r3k_disasm->targets[y]);
				}
			
				queue_draw (r3k_disasm);
			}	break;

			default:
				break;
		}
	}

	if (event->button==2)
	{
		guint y = event->y / r3k_disasm->ch;

		switch (event->type)
		{
			case GDK_BUTTON_PRESS:
			{
				gtk_r3k_disasm_set_cursor (r3k_disasm,gtk_memory_get_address (r3k_disasm->memory) + (4*y));
			}	break;

			case GDK_2BUTTON_PRESS:
			{
				GtkBreakpoint *bkpt;
				gulong addr;
			
				addr = gtk_memory_get_address (r3k_disasm->memory) + (4*y);
			
				bkpt = gtk_breakpoints_find (r3k_disasm->breakpoints,addr);
				if (bkpt != NULL)
				{
					// this safes one refresh
					BUFFER(r3k_disasm)[y] = bkpt->safe;

					gtk_breakpoints_remove (r3k_disasm->breakpoints,bkpt);
				}
				else
				{
					bkpt = gtk_breakpoints_create (r3k_disasm->breakpoints,addr);

					BUFFER(r3k_disasm)[y] = bkpt->insn;
				}
			
				queue_draw (r3k_disasm);
			}	break;

			default:
				break;
		}
	}


	if (event->button==3)
	{
		if (r3k_disasm->stack)
		{
			addr_t addr = (addr_t) r3k_disasm->stack->data;
			r3k_disasm->stack = g_slist_remove (r3k_disasm->stack,(gpointer)addr);
			gtk_memory_set_address (r3k_disasm->memory,addr);
		}
	}

	return TRUE;
}

static gint
gtk_r3k_disasm_button_release (GtkWidget *widget, GdkEventButton *event)
{
	GtkR3kDisasm *r3k_disasm;

	g_return_val_if_fail (widget != NULL, FALSE);
	g_return_val_if_fail (GTK_IS_R3K_DISASM (widget), FALSE);

	r3k_disasm = GTK_R3K_DISASM (widget);

	r3k_disasm->button = 0;

	gtk_widget_queue_draw (GTK_WIDGET (r3k_disasm));

	return TRUE;
}

static gint
gtk_r3k_disasm_motion (GtkWidget *widget, GdkEventMotion *event)
{
	GtkR3kDisasm *r3k_disasm;
	gint x,y;
		
	g_return_val_if_fail (widget != NULL, FALSE);
	g_return_val_if_fail (GTK_IS_R3K_DISASM (widget), FALSE);

	r3k_disasm = GTK_R3K_DISASM (widget);

	if (event->is_hint)
		gdk_window_get_pointer (widget->window, &x, &y, NULL);
	else
	{
		x = event->x;
		y = event->y;
	}

	y /= r3k_disasm->ch;

//	g_print ("pos=%d %d,%d\n",y,x);

	// check if mouse cursor is over a line
	if (y<0 || y>r3k_disasm->rows)
	{
		r3k_disasm->position = -1;
		return FALSE;
	}

//	r3k_disasm->targets[r3k_disasm->position] = NULL;

	// minimize overhead
	if (r3k_disasm->position != y)
	{
		opcode_t *opcode;
	
		r3k_disasm->position = y;

		if (r3k_disasm->over_link == TRUE)
		{
			r3k_disasm->over_link = FALSE;
			gtk_signal_emit (GTK_OBJECT (r3k_disasm), r3k_disasm_signals[SIG_LEAVE_BRANCH_NOTIFY]);
			gdk_window_set_cursor (widget->window,NULL);
			queue_draw (r3k_disasm);
		}

		opcode = r3k_find_opcode (gtk_memory_get_address (r3k_disasm->memory)+(y*4),(insn_t)BUFFER(r3k_disasm)[y]);

		if (opcode)	if (opcode->has_branch_target)
		{
			r3k_disasm->over_link = TRUE;
			gtk_signal_emit (GTK_OBJECT (r3k_disasm), r3k_disasm_signals[SIG_ENTER_BRANCH_NOTIFY]);
			queue_draw (r3k_disasm);
			gdk_window_set_cursor (widget->window,r3k_disasm->branch_cursor);
		}
	}

//	gtk_widget_queue_draw (GTK_WIDGET (r3k_disasm));

	return FALSE;
}

/** private utility functions **/

static void on_bkpt_insert (GtkBreakpoints *breakpoints, GtkBreakpoint *breakpoint, GtkR3kDisasm *r3k_disasm)
{
	queue_draw (r3k_disasm);
}

static void on_bkpt_remove (GtkBreakpoints *breakpoints, GtkBreakpoint *breakpoint, GtkR3kDisasm *r3k_disasm)
{
	queue_draw (r3k_disasm);
}

/** public interface **/

GtkWidget*
gtk_r3k_disasm_new (GtkMemory *memory, GtkBreakpoints *breakpoints)
{
	GtkR3kDisasm *r3k_disasm;

	r3k_disasm = gtk_type_new (gtk_r3k_disasm_get_type());

	r3k_disasm->memory = memory;
	r3k_disasm->breakpoints = breakpoints;

	gtk_signal_connect (GTK_OBJECT(breakpoints),"insert",
		GTK_SIGNAL_FUNC(on_bkpt_insert),r3k_disasm);

	gtk_signal_connect (GTK_OBJECT(breakpoints),"remove",
		GTK_SIGNAL_FUNC(on_bkpt_remove),r3k_disasm);

	return GTK_WIDGET(r3k_disasm);
}

static void
gtk_r3k_disasm_destroy (GtkObject *object)
{
	GtkR3kDisasm *r3k_disasm;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GTK_IS_R3K_DISASM(object));
    
	r3k_disasm = GTK_R3K_DISASM(object);

//	gtk_signal_disconnect_by_func (GTK_OBJECT(r3k_disasm->breakpoints),"remove",
//		GTK_SIGNAL_FUNC(on_bkpt_remove));

//	gtk_signal_disconnect_by_func (GTK_OBJECT(r3k_disasm->breakpoints),"insert",
//		GTK_SIGNAL_FUNC(on_bkpt_insert));

	gdk_pixmap_unref (r3k_disasm->icons);
	gdk_bitmap_unref (r3k_disasm->masks);

	if (GTK_OBJECT_CLASS (parent_class)->destroy)
	 (* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}

static void
gtk_r3k_disasm_set_arg (GtkObject *object,
	GtkArg *arg,
	guint arg_id)
{
	GtkR3kDisasm *r3k_disasm;

	r3k_disasm = GTK_R3K_DISASM (object);
  
	switch (arg_id)
	{
		case ARG_ADDRESS:
			gtk_memory_set_address (r3k_disasm->memory, GTK_VALUE_ULONG (*arg));
			break;
		case ARG_POINTER:
			gtk_r3k_disasm_set_pointer (r3k_disasm, GTK_VALUE_ULONG (*arg));
			break;
		default:
			break;
	}
}

static void
gtk_r3k_disasm_get_arg (GtkObject *object,
	GtkArg *arg,
	guint arg_id)
{
	GtkR3kDisasm *r3k_disasm;

	r3k_disasm = GTK_R3K_DISASM (object);
  
	switch (arg_id)
	{
		case ARG_ADDRESS:
			GTK_VALUE_ULONG (*arg) = gtk_memory_get_address (r3k_disasm->memory);
			break;
		case ARG_POINTER:
			GTK_VALUE_ULONG (*arg) = r3k_disasm->pointer;
			break;
		default:
			arg->type = GTK_TYPE_INVALID;
			break;
	}
}

void
gtk_r3k_disasm_address_changed (GtkR3kDisasm *r3k_disasm)
{
	g_return_if_fail (r3k_disasm != NULL);
	g_return_if_fail (GTK_IS_R3K_DISASM (r3k_disasm));

	if (r3k_disasm->rows<1) return;

	gtk_signal_emit (GTK_OBJECT (r3k_disasm), r3k_disasm_signals[SIG_ADDRESS_CHANGED]);

	queue_draw (r3k_disasm);
}

void
gtk_r3k_disasm_pointer_changed (GtkR3kDisasm *r3k_disasm)
{
	g_return_if_fail (r3k_disasm != NULL);
	g_return_if_fail (GTK_IS_R3K_DISASM (r3k_disasm));

	gtk_signal_emit (GTK_OBJECT (r3k_disasm), r3k_disasm_signals[SIG_POINTER_CHANGED]);

	queue_draw (r3k_disasm);
}

void
gtk_r3k_disasm_cursor_changed (GtkR3kDisasm *r3k_disasm)
{
	g_return_if_fail (r3k_disasm != NULL);
	g_return_if_fail (GTK_IS_R3K_DISASM (r3k_disasm));

	gtk_signal_emit (GTK_OBJECT (r3k_disasm), r3k_disasm_signals[SIG_CURSOR_CHANGED]);

	queue_draw (r3k_disasm);
}

void
gtk_r3k_disasm_set_address (GtkR3kDisasm *r3k_disasm, gulong addr)
{
	g_return_if_fail (r3k_disasm != NULL);
	g_return_if_fail (GTK_IS_R3K_DISASM (r3k_disasm));
	
	gtk_memory_set_address(r3k_disasm->memory, addr);
	gtk_r3k_disasm_address_changed (r3k_disasm);
}

gulong
gtk_r3k_disasm_get_address (GtkR3kDisasm *r3k_disasm)
{
	g_return_val_if_fail (r3k_disasm != NULL, 0);
	g_return_val_if_fail (GTK_IS_R3K_DISASM (r3k_disasm), 0);

	return gtk_memory_get_address(r3k_disasm->memory);
}

void
gtk_r3k_disasm_set_pointer (GtkR3kDisasm *r3k_disasm, gulong addr)
{
	g_return_if_fail (r3k_disasm != NULL);
	g_return_if_fail (GTK_IS_R3K_DISASM (r3k_disasm));
	
	r3k_disasm->pointer = addr;

	gtk_r3k_disasm_pointer_changed (r3k_disasm);
}

gulong
gtk_r3k_disasm_get_pointer (GtkR3kDisasm *r3k_disasm)
{
	g_return_val_if_fail (r3k_disasm != NULL, 0);
	g_return_val_if_fail (GTK_IS_R3K_DISASM (r3k_disasm), 0);

	return r3k_disasm->pointer;
}

void
gtk_r3k_disasm_set_cursor (GtkR3kDisasm *r3k_disasm, gulong addr)

{
	g_return_if_fail (r3k_disasm != NULL);
	g_return_if_fail (GTK_IS_R3K_DISASM (r3k_disasm));
	
	r3k_disasm->cursor = addr;

	gtk_r3k_disasm_cursor_changed (r3k_disasm);
}

gulong
gtk_r3k_disasm_get_cursor (GtkR3kDisasm *r3k_disasm)
{
	g_return_val_if_fail (r3k_disasm != NULL, 0);
	g_return_val_if_fail (GTK_IS_R3K_DISASM (r3k_disasm), 0);

	return r3k_disasm->cursor;
}
