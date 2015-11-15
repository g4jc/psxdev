/* GtkHexEntry Widget */

/*
	TODO:
	
	- selection/clipboard
	- more keyboard control
	- activate/advance, last container element is double activated
	
	+ tooltips with dec/oct/bin base conversion aid ?
*/

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <ctype.h>

#include <gtkhexentry.h>

enum {
	ARG_0, 
	ARG_VALUE,
};

enum {
	SIG_VALUE_CHANGED,
	SIG_ACTIVATE,
	SIG_LAST
};

static void gtk_hex_entry_class_init (GtkHexEntryClass *klass);
static void gtk_hex_entry_init (GtkHexEntry *hex_entry);
static void gtk_hex_entry_destroy (GtkObject *object);
static void gtk_hex_entry_finalize (GtkObject *object);
static void gtk_hex_entry_set_arg (GtkObject *object, GtkArg *arg, guint arg_id);
static void gtk_hex_entry_get_arg (GtkObject *object, GtkArg *arg, guint arg_id);

static void gtk_hex_entry_realize (GtkWidget *widget);
static void gtk_hex_entry_unrealize (GtkWidget *widget);
static void gtk_hex_entry_size_request (GtkWidget *widget, GtkRequisition *requisition);
static void gtk_hex_entry_size_allocate (GtkWidget *widget, GtkAllocation *allocation);
static void gtk_hex_entry_draw (GtkWidget *widget, GdkRectangle *area);
static gint gtk_hex_entry_expose (GtkWidget *widget, GdkEventExpose *event);
static void gtk_hex_entry_paint (GtkWidget *widget, GdkRectangle *area);
static gint gtk_hex_entry_button_press (GtkWidget *widget, GdkEventButton *event);
static gint gtk_hex_entry_button_release (GtkWidget *widget, GdkEventButton *event);
static gint gtk_hex_entry_motion (GtkWidget *widget, GdkEventMotion *event);
static gint gtk_hex_entry_configure_event (GtkWidget *widget, GdkEventConfigure *event);
static gint gtk_hex_entry_key_press (GtkWidget *widget, GdkEventKey *event);
static gint gtk_hex_entry_focus_in (GtkWidget *widget, GdkEventFocus *event);
static gint gtk_hex_entry_focus_out (GtkWidget *widget, GdkEventFocus *event);
static gint gtk_hex_entry_enter_notify (GtkWidget *widget, GdkEventCrossing *event);
static gint gtk_hex_entry_leave_notify (GtkWidget *widget, GdkEventCrossing *event);

static GtkWidgetClass *parent_class = NULL;
static guint hex_entry_signals[SIG_LAST] = { 0 };

GtkType
gtk_hex_entry_get_type (void)
{
	static GtkType hex_entry_type = 0;
  
	if (!hex_entry_type)
	{
		GtkTypeInfo hex_entry_info =
		{
			(gchar*)"GtkHexEntry",
			sizeof (GtkHexEntry),
			sizeof (GtkHexEntryClass),
			(GtkClassInitFunc) gtk_hex_entry_class_init,
			(GtkObjectInitFunc) gtk_hex_entry_init,
			/* reserved_1 */ NULL,
			/* reserved_2 */ NULL,
			(GtkClassInitFunc) NULL
		};
		hex_entry_type = gtk_type_unique (gtk_widget_get_type (), &hex_entry_info);
	}

	return hex_entry_type;
}

static void
gtk_hex_entry_class_init (GtkHexEntryClass *klass)
{
	GtkObjectClass *object_class;
	GtkWidgetClass *widget_class;

	parent_class = gtk_type_class(gtk_widget_get_type());
	object_class = (GtkObjectClass*) klass;
	widget_class = (GtkWidgetClass*) klass;

	object_class->destroy = gtk_hex_entry_destroy;
	object_class->finalize = gtk_hex_entry_finalize;
	object_class->set_arg = gtk_hex_entry_set_arg;
	object_class->get_arg = gtk_hex_entry_get_arg;

	widget_class->realize = gtk_hex_entry_realize;
	widget_class->unrealize = gtk_hex_entry_unrealize;
	widget_class->size_request = gtk_hex_entry_size_request;
	widget_class->size_allocate = gtk_hex_entry_size_allocate;
	widget_class->configure_event = gtk_hex_entry_configure_event;
	widget_class->draw = gtk_hex_entry_draw;
	widget_class->expose_event = gtk_hex_entry_expose;
	widget_class->button_press_event = gtk_hex_entry_button_press;
	widget_class->button_release_event = gtk_hex_entry_button_release;
	widget_class->motion_notify_event = gtk_hex_entry_motion;
	widget_class->key_press_event = gtk_hex_entry_key_press;
	widget_class->focus_out_event = gtk_hex_entry_focus_out;
	widget_class->focus_in_event = gtk_hex_entry_focus_in;
	widget_class->enter_notify_event = gtk_hex_entry_enter_notify;
	widget_class->leave_notify_event = gtk_hex_entry_leave_notify;

	/* register arguments */

	gtk_object_add_arg_type ("GtkHexEntry::value",
		GTK_TYPE_ULONG,
		GTK_ARG_READWRITE,
		ARG_VALUE);

	/* register signals */

	hex_entry_signals[SIG_VALUE_CHANGED] =
	gtk_signal_new ("value_changed",
		GTK_RUN_FIRST | GTK_RUN_NO_RECURSE,
		object_class->type,
		GTK_SIGNAL_OFFSET (GtkHexEntryClass, value_changed),
		gtk_marshal_NONE__NONE,
		GTK_TYPE_NONE, 0);

	hex_entry_signals[SIG_ACTIVATE] =
	gtk_signal_new ("activate",
		GTK_RUN_LAST | GTK_RUN_NO_RECURSE,
		object_class->type,
		GTK_SIGNAL_OFFSET (GtkHexEntryClass, activate),
		gtk_marshal_NONE__NONE,
		GTK_TYPE_NONE, 0);

	gtk_object_class_add_signals (object_class, hex_entry_signals, SIG_LAST);

	klass->value_changed = NULL;
	klass->activate = NULL;
}

static void
make_copy (GtkHexEntry *hex_entry, gboolean modified)
{
	gint i;
	
	for (i=0; i<hex_entry->digits; i++)
	{
		hex_entry->buffer[i+hex_entry->digits] = hex_entry->buffer[i];
	}

	hex_entry->modified = modified;
}

static void
make_undo (GtkHexEntry *hex_entry, gboolean modified)
{
	gint i;
	
	for (i=0; i<hex_entry->digits; i++)
	{
		hex_entry->buffer[i] = hex_entry->buffer[i+hex_entry->digits];
	}

	hex_entry->modified = modified;
}

static gint
queue_draw_timeout (gpointer data)
{
	GtkHexEntry *hex_entry;

	GDK_THREADS_ENTER ();

	hex_entry = GTK_HEX_ENTRY (data);
	hex_entry->timer = 0;
	gtk_hex_entry_paint (GTK_WIDGET(hex_entry),NULL);

	GDK_THREADS_LEAVE ();

	return FALSE;
}

static void
queue_draw (GtkHexEntry *hex_entry)
{
	g_return_if_fail (hex_entry != NULL);
	g_return_if_fail (GTK_IS_HEX_ENTRY (hex_entry));

	if (!hex_entry->timer)
		hex_entry->timer = gtk_timeout_add (20, queue_draw_timeout, hex_entry);
}

static void
gtk_hex_entry_init (GtkHexEntry *hex_entry)
{
	GTK_WIDGET_SET_FLAGS (hex_entry, GTK_CAN_FOCUS);

	/* */
	hex_entry->digits = 8;
	hex_entry->buffer = NULL;

	hex_entry->gc = NULL;
	hex_entry->pixmap = NULL;
	hex_entry->editable = TRUE;
	hex_entry->cursor_position = -1;

	hex_entry->modified = FALSE;
	
	gdk_color_parse ("red",&hex_entry->cursor_color);
	
//	hex_entry->font = gdk_font_load ("fixed");
	hex_entry->font = gdk_font_load ("9x15");
}

GtkWidget*
gtk_hex_entry_new (guint digits, gulong value)
{
	GtkHexEntry *hex_entry;

	hex_entry = gtk_type_new (gtk_hex_entry_get_type());

	if ((digits<1) || (digits>8)) digits = 8;
	hex_entry->digits = digits;

	hex_entry->buffer = g_malloc (hex_entry->digits * 2);

	gtk_hex_entry_set_value (hex_entry, value);

	return GTK_WIDGET(hex_entry);
}

static void
gtk_hex_entry_destroy (GtkObject *object)
{
	GtkHexEntry *hex_entry;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GTK_IS_HEX_ENTRY(object));
    
	hex_entry = GTK_HEX_ENTRY(object);

	g_free (hex_entry->buffer);

	if (GTK_OBJECT_CLASS (parent_class)->destroy)
	 (* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}

static void
gtk_hex_entry_finalize (GtkObject *object)
{
	g_return_if_fail (object != NULL);
	g_return_if_fail (GTK_IS_HEX_ENTRY (object));

	if (GTK_HEX_ENTRY (object)->pixmap)
		gdk_pixmap_unref (GTK_HEX_ENTRY (object)->pixmap);

	gdk_font_unref (GTK_HEX_ENTRY (object)->font);

	GTK_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gtk_hex_entry_size_request (GtkWidget *widget, GtkRequisition *requisition)
{
	GtkHexEntry *hex_entry;

	g_return_if_fail (widget != NULL);
	g_return_if_fail (GTK_IS_HEX_ENTRY (widget));
	g_return_if_fail (requisition != NULL);

	hex_entry = GTK_HEX_ENTRY (widget);

	requisition->width = (hex_entry->digits * gdk_char_width (hex_entry->font,'1'));
	requisition->height = hex_entry->font->ascent + hex_entry->font->descent;
}

static void
gtk_hex_entry_size_allocate (GtkWidget *widget, GtkAllocation *allocation)
{
	GtkHexEntry *hex_entry;

	g_return_if_fail (widget != NULL);
	g_return_if_fail (GTK_IS_HEX_ENTRY (widget));
	g_return_if_fail (allocation != NULL);

	widget->allocation = *allocation;
	hex_entry = GTK_HEX_ENTRY (widget);

	if (GTK_WIDGET_REALIZED (widget))
	{
		GtkRequisition requisition;
		gtk_widget_get_child_requisition (widget, &requisition);
  
		gdk_window_move_resize (widget->window,
			allocation->x,
			allocation->y,
			requisition.width, requisition.height);

		queue_draw (hex_entry);
	}
}

static gint
gtk_hex_entry_configure_event (GtkWidget *widget, GdkEventConfigure *event)
{
	GtkHexEntry *hex_entry;

	g_return_val_if_fail (widget != NULL,FALSE);
	g_return_val_if_fail (GTK_IS_HEX_ENTRY (widget),FALSE);

	hex_entry = GTK_HEX_ENTRY (widget);

	if (hex_entry->pixmap) gdk_pixmap_unref (hex_entry->pixmap);
	hex_entry->pixmap = NULL;

//	if (GTK_WIDGET_CLASS (parent_class)->configure_event)
//	return (* GTK_WIDGET_CLASS (parent_class)->configure_event) (widget,event);

	return FALSE;
}

static void
gtk_hex_entry_realize (GtkWidget *widget)
{
	GtkHexEntry *hex_entry;
	GtkRequisition requisition;
	GdkWindowAttr attributes;
	gint attributes_mask;

	g_return_if_fail (widget != NULL);
	g_return_if_fail (GTK_IS_HEX_ENTRY (widget));

	GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);
	hex_entry = GTK_HEX_ENTRY (widget);

	gtk_widget_get_child_requisition (widget, &requisition);

	attributes.x = widget->allocation.x;
	attributes.y = widget->allocation.y;
	attributes.width = requisition.width;
	attributes.height = requisition.height;

	attributes.window_type = GDK_WINDOW_CHILD;
	attributes.wclass = GDK_INPUT_OUTPUT;
	attributes.visual = gtk_widget_get_visual (widget);
	attributes.colormap = gtk_widget_get_colormap (widget);

	attributes.cursor = hex_entry->cursor = gdk_cursor_new (GDK_XTERM);

	attributes.event_mask = gtk_widget_get_events (widget)
	| GDK_EXPOSURE_MASK | GDK_KEY_PRESS_MASK
	| GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK
	| GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_BUTTON1_MOTION_MASK
	| GDK_POINTER_MOTION_HINT_MASK;

	attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP |
		GDK_WA_CURSOR;
  
	widget->window = gdk_window_new (gtk_widget_get_parent_window (widget),
		&attributes, attributes_mask);
	gdk_window_set_user_data (widget->window, hex_entry);

	widget->style = gtk_style_attach (widget->style, widget->window);

	gtk_style_set_background (widget->style, widget->window, GTK_STATE_NORMAL);

	gdk_window_set_background (widget->window, &widget->style->base[GTK_WIDGET_STATE (widget)]);
	gdk_window_set_back_pixmap (widget->window, NULL, TRUE);

	gdk_color_alloc (gtk_widget_get_colormap (widget),&hex_entry->cursor_color);

	hex_entry->gc = gdk_gc_new (widget->window);	
	gdk_gc_set_foreground (hex_entry->gc,&hex_entry->cursor_color);
}

static void
gtk_hex_entry_unrealize (GtkWidget *widget)
{
	GtkHexEntry *hex_entry;

	g_return_if_fail (widget != NULL);
	g_return_if_fail (GTK_IS_HEX_ENTRY (widget));

	hex_entry = GTK_HEX_ENTRY (widget);

	gdk_gc_destroy (hex_entry->gc);
	hex_entry->gc = NULL;

	gdk_cursor_destroy (hex_entry->cursor);
	hex_entry->cursor = NULL;

	if (GTK_WIDGET_CLASS (parent_class)->unrealize)
	(* GTK_WIDGET_CLASS (parent_class)->unrealize) (widget);
}

static void
gtk_hex_entry_paint (GtkWidget *widget,
	GdkRectangle *area)
{
	GtkHexEntry *hex_entry;
	GdkRectangle def_area;

	g_return_if_fail (widget != NULL);
	g_return_if_fail (GTK_IS_HEX_ENTRY (widget));

	hex_entry = GTK_HEX_ENTRY (widget);

	if (area == NULL)
	{
		area = &def_area;
		area->x = area->y = 0;
		area->width = widget->allocation.width;
		area->height = widget->allocation.height;
	}

	if (hex_entry->pixmap==NULL)
		hex_entry->pixmap = gdk_pixmap_new (widget->window,
			widget->allocation.width, widget->allocation.height, -1);

	if (GTK_WIDGET_DRAWABLE (widget))
	{
		gint w,h;

		w = gdk_char_width (hex_entry->font,'0');
		h = hex_entry->font->ascent + hex_entry->font->descent;

		if (hex_entry->selected)
		{
			gdk_draw_rectangle (hex_entry->pixmap,widget->style->white_gc,TRUE,
			0,0,widget->allocation.width,widget->allocation.height);
		}
		else
		{
			gdk_draw_rectangle (hex_entry->pixmap,widget->style->bg_gc[GTK_WIDGET_STATE (widget)],TRUE,
			0,0,widget->allocation.width,widget->allocation.height);
		}
/*
		if (hex_entry->has_selection)
		{
			g_print ("%d, %d\n",hex_entry->selection_start_pos,hex_entry->selection_end_pos);

			gdk_draw_rectangle (hex_entry->pixmap,widget->style->black_gc,TRUE,
			(hex_entry->selection_start_pos*w),0,
			((hex_entry->selection_end_pos-hex_entry->selection_start_pos)*w),h);
		}
*/
		if (GTK_WIDGET_HAS_FOCUS(widget))
		{
			if ( (hex_entry->cursor_position>=0) &&
				(hex_entry->cursor_position<hex_entry->digits) )
			{
				gdk_draw_rectangle (hex_entry->pixmap,hex_entry->gc,TRUE,
				(hex_entry->cursor_position*w),
				0,w,h);
			}
		}

		gdk_draw_string (hex_entry->pixmap,
			hex_entry->font,
			widget->style->fg_gc[hex_entry->modified ? GTK_STATE_SELECTED : GTK_WIDGET_STATE (widget)],
			0,
			hex_entry->font->ascent,
			hex_entry->buffer);

		gdk_draw_pixmap (widget->window, widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
			hex_entry->pixmap, area->x, area->y, area->x, area->y, area->width, area->height);
	}
}

static void
gtk_hex_entry_draw (GtkWidget *widget,
	GdkRectangle *area)
{
	g_return_if_fail (widget != NULL);
	g_return_if_fail (GTK_IS_HEX_ENTRY (widget));
	g_return_if_fail (area != NULL);

	if (GTK_WIDGET_DRAWABLE (widget))
		gtk_hex_entry_paint (widget, area);
}

static gint
gtk_hex_entry_expose (GtkWidget *widget,
	GdkEventExpose *event)
{
	GtkHexEntry *hex_entry;

	g_return_val_if_fail (widget != NULL, FALSE);
	g_return_val_if_fail (GTK_IS_HEX_ENTRY (widget), FALSE);
	g_return_val_if_fail (event != NULL, FALSE);

	hex_entry = GTK_HEX_ENTRY (widget);
	
	if (GTK_WIDGET_DRAWABLE (widget))
		gtk_hex_entry_paint (widget, &event->area);

	return FALSE;
}

static void
move_beginning_of_line (GtkHexEntry *hex_entry)
{
	hex_entry->cursor_position = 0;
}

static void
move_end_of_line (GtkHexEntry *hex_entry)
{
	hex_entry->cursor_position = hex_entry->digits-1;
}

static void
move_forward_character (GtkHexEntry *hex_entry)
{
	hex_entry->cursor_position++;
	if (hex_entry->cursor_position>=hex_entry->digits)
		hex_entry->cursor_position = hex_entry->digits-1;
}

static void
move_backward_character (GtkHexEntry *hex_entry)
{
	hex_entry->cursor_position--;
	if (hex_entry->cursor_position<0)
		hex_entry->cursor_position = 0;
}

static gint
gtk_hex_entry_key_press (GtkWidget *widget, GdkEventKey *event)
{
	GtkHexEntry *hex_entry;
	int key;

	g_return_val_if_fail (widget != NULL, FALSE);
	g_return_val_if_fail (GTK_IS_HEX_ENTRY (widget), FALSE);
	g_return_val_if_fail (event != NULL, FALSE);

	hex_entry = GTK_HEX_ENTRY (widget);

	if (hex_entry->editable == FALSE) return FALSE;

	if (hex_entry->cursor_position==-1)
	hex_entry->cursor_position = 0;

	key = event->keyval;

	switch (key)
	{
		case GDK_Return:
		case GDK_KP_Enter:
			make_copy (hex_entry,FALSE);
			hex_entry->cursor_position = 0;
			gtk_signal_emit (GTK_OBJECT (hex_entry), hex_entry_signals[SIG_ACTIVATE]);
			break;

		case GDK_F1:
			if (hex_entry->modified)
			{
				make_undo (hex_entry,FALSE);
				gtk_signal_emit (GTK_OBJECT (hex_entry), hex_entry_signals[SIG_VALUE_CHANGED]);
			}
			break;

		case GDK_plus:
		case GDK_KP_Add:
			gtk_hex_entry_set_value (hex_entry,
				gtk_hex_entry_get_value(hex_entry)+1);
			break;
		case GDK_KP_Subtract:
		case GDK_minus:
			gtk_hex_entry_set_value (hex_entry,
				gtk_hex_entry_get_value(hex_entry)-1);
			break;

		case GDK_KP_Page_Up:
		case GDK_Page_Up:
			gtk_hex_entry_set_value (hex_entry,
				gtk_hex_entry_get_value(hex_entry)+256);
			break;
		case GDK_KP_Page_Down:
		case GDK_Page_Down:
			gtk_hex_entry_set_value (hex_entry,
				gtk_hex_entry_get_value(hex_entry)-256);
			break;
	
		case GDK_Delete:
		case GDK_Clear:
		case GDK_KP_Delete:
			move_beginning_of_line(hex_entry);
			gtk_hex_entry_set_value (hex_entry,0);
			break;
		case GDK_BackSpace:
			move_backward_character(hex_entry);
			hex_entry->buffer[hex_entry->cursor_position] = '0';
			gtk_hex_entry_value_changed (hex_entry);
			break;
		case GDK_Home:
			move_beginning_of_line(hex_entry);
			break;
		case GDK_End:
			move_end_of_line(hex_entry);
			break;
		case GDK_Left:
			move_backward_character(hex_entry);
			break;
		case GDK_Right:
			move_forward_character(hex_entry);
			break;

		case GDK_KP_0...GDK_KP_9:
			key -= GDK_KP_0;
			key += '0';

		default:
			if ( ((key>='0')&&(key<='9')) || ((key>='a')&&(key<='f')) || ((key>='A')&&(key<='F')))
			{
				hex_entry->buffer[hex_entry->cursor_position] = toupper(key);
				move_forward_character(hex_entry);
				gtk_hex_entry_value_changed (hex_entry);
				break;
			}
			else return FALSE;
	}

	queue_draw (hex_entry);

	return TRUE;
}

static gint
gtk_hex_entry_enter_notify (GtkWidget *widget, GdkEventCrossing *event)
{
	GtkHexEntry *hex_entry;

	g_return_val_if_fail (widget != NULL, FALSE);
	g_return_val_if_fail (GTK_IS_HEX_ENTRY (widget), FALSE);
	g_return_val_if_fail (event != NULL, FALSE);

	hex_entry = GTK_HEX_ENTRY (widget);

	widget->state = GTK_WIDGET_HAS_FOCUS(widget) ? GTK_STATE_ACTIVE : GTK_STATE_PRELIGHT;

	queue_draw (hex_entry);
	
	return FALSE;
}

static gint
gtk_hex_entry_leave_notify (GtkWidget *widget, GdkEventCrossing *event)
{
	GtkHexEntry *hex_entry;

	g_return_val_if_fail (widget != NULL, FALSE);
	g_return_val_if_fail (GTK_IS_HEX_ENTRY (widget), FALSE);
	g_return_val_if_fail (event != NULL, FALSE);

	hex_entry = GTK_HEX_ENTRY (widget);

	widget->state = GTK_STATE_NORMAL;

	queue_draw (hex_entry);

	return FALSE;
}

static gint
gtk_hex_entry_focus_in (GtkWidget *widget, GdkEventFocus *event)
{
	GtkHexEntry *hex_entry;

	g_return_val_if_fail (widget != NULL, FALSE);
	g_return_val_if_fail (GTK_IS_HEX_ENTRY (widget), FALSE);
	g_return_val_if_fail (event != NULL, FALSE);

	hex_entry = GTK_HEX_ENTRY (widget);
	
	GTK_WIDGET_SET_FLAGS (widget, GTK_HAS_FOCUS);

	widget->state = GTK_STATE_ACTIVE;

	hex_entry->cursor_position = 0;

	queue_draw (hex_entry);

	return FALSE;
}

static gint
gtk_hex_entry_focus_out (GtkWidget *widget, GdkEventFocus *event)
{
	GtkHexEntry *hex_entry;

	g_return_val_if_fail (widget != NULL, FALSE);
	g_return_val_if_fail (GTK_IS_HEX_ENTRY (widget), FALSE);
	g_return_val_if_fail (event != NULL, FALSE);

	hex_entry = GTK_HEX_ENTRY (widget);

	GTK_WIDGET_UNSET_FLAGS (widget, GTK_HAS_FOCUS);

	widget->state = GTK_STATE_NORMAL;

	queue_draw (hex_entry);

	return FALSE;
}

static gint
gtk_hex_entry_button_press (GtkWidget *widget, GdkEventButton *event)
{
	GtkHexEntry *hex_entry;

	g_return_val_if_fail (widget != NULL, FALSE);
	g_return_val_if_fail (GTK_IS_HEX_ENTRY (widget), FALSE);

	hex_entry = GTK_HEX_ENTRY (widget);
	hex_entry->button = event->button;

	if (!GTK_WIDGET_HAS_FOCUS (widget))
		gtk_widget_grab_focus (widget);

	if (event->button==1)
	{
		switch (event->type)
		{
			case GDK_BUTTON_PRESS:
				gtk_grab_add (widget);
				hex_entry->has_selection = TRUE;
				hex_entry->cursor_position = event->x / gdk_char_width (hex_entry->font,'0');
				hex_entry->selection_end_pos =
				hex_entry->selection_start_pos = hex_entry->cursor_position;
				break;
			case GDK_2BUTTON_PRESS:
				hex_entry->selected ^= 1;
				break;
			
			default:
				break;
		}
	}

	return TRUE;
}

static gint
gtk_hex_entry_button_release (GtkWidget *widget, GdkEventButton *event)
{
	GtkHexEntry *hex_entry;

	g_return_val_if_fail (widget != NULL, FALSE);
	g_return_val_if_fail (GTK_IS_HEX_ENTRY (widget), FALSE);

	hex_entry = GTK_HEX_ENTRY (widget);

	if (hex_entry->button != event->button) return FALSE;
	
	if (event->button == 1)
	{
		gtk_grab_remove (widget);
		hex_entry->has_selection = FALSE;
		if (hex_entry->selection_start_pos != hex_entry->selection_end_pos)
		{
			hex_entry->has_selection = TRUE;
		}
	}

	queue_draw (hex_entry);

	return FALSE;
}

static gint
gtk_hex_entry_motion (GtkWidget *widget, GdkEventMotion *event)
{
	GtkHexEntry *hex_entry;
	gint x;
		
	g_return_val_if_fail (widget != NULL, FALSE);
	g_return_val_if_fail (GTK_IS_HEX_ENTRY (widget), FALSE);

	hex_entry = GTK_HEX_ENTRY (widget);

	if (hex_entry->button == 0) return FALSE;

	x = event->x;
	if (event->is_hint || (widget->window != event->window))
		gdk_window_get_pointer (widget->window, &x, NULL, NULL);

	hex_entry->cursor_position = x / gdk_char_width (hex_entry->font,'0');
	hex_entry->selection_end_pos = hex_entry->cursor_position;

	if (hex_entry->selection_end_pos < hex_entry->selection_start_pos)
	{
		hex_entry->selection_end_pos = hex_entry->selection_start_pos;
		hex_entry->selection_start_pos = hex_entry->cursor_position;
	}

	queue_draw (hex_entry);

	return TRUE;
}

static void
gtk_hex_entry_set_arg (GtkObject *object,
	GtkArg *arg,
	guint arg_id)
{
	GtkHexEntry *hex_entry;

	hex_entry = GTK_HEX_ENTRY (object);
  
	switch (arg_id)
	{
		case ARG_VALUE:
			gtk_hex_entry_set_value (hex_entry, GTK_VALUE_ULONG (*arg));
			break;
		default:
			break;
	}
}

static void
gtk_hex_entry_get_arg (GtkObject *object,
	GtkArg *arg,
	guint arg_id)
{
	GtkHexEntry *hex_entry;

	hex_entry = GTK_HEX_ENTRY (object);
  
	switch (arg_id)
	{
		case ARG_VALUE:
//			GTK_VALUE_ULONG (*arg) = hex_entry->value;
			break;
		default:
			arg->type = GTK_TYPE_INVALID;
			break;
	}
}

void
gtk_hex_entry_value_changed (GtkHexEntry *hex_entry)
{
	g_return_if_fail (hex_entry != NULL);
	g_return_if_fail (GTK_IS_HEX_ENTRY (hex_entry));

	if (hex_entry->modified == FALSE)
	{
		make_copy (hex_entry,TRUE);
	}

	gtk_signal_emit (GTK_OBJECT (hex_entry), hex_entry_signals[SIG_VALUE_CHANGED]);
}

void 
gtk_hex_entry_set_value (GtkHexEntry *hex_entry, gulong arg)
{
	gint i,j;
	
	g_return_if_fail (hex_entry != NULL);
	g_return_if_fail (GTK_IS_HEX_ENTRY (hex_entry));

	for (i=hex_entry->digits; i; --i)
	{
		j = hex_entry->digits-i;
	
		hex_entry->buffer[j] = ((arg >> ((i-1)*4)) & 0xf) + 0x30;
		
		if (hex_entry->buffer[j]>'9') hex_entry->buffer[j] += 7;
	}

	make_copy (hex_entry,FALSE);

	gtk_signal_emit (GTK_OBJECT (hex_entry), hex_entry_signals[SIG_VALUE_CHANGED]);

	queue_draw (hex_entry);
}

gulong
gtk_hex_entry_get_value (GtkHexEntry *hex_entry)
{
	gulong value;
	gint i,j,ch;

	g_return_val_if_fail (hex_entry != NULL, 0);
	g_return_val_if_fail (GTK_IS_HEX_ENTRY (hex_entry), 0);

	value = 0;
	for (i=hex_entry->digits; i; --i)
	{
		j = hex_entry->digits-i;
		ch = hex_entry->buffer[j] - 0x30;
		if (ch > 9) ch -= 7;
		value <<= 4;
		value |= ch & 0xf;
	}

	return value;
}
