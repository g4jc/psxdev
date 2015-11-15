/* $Id: gtkbreakpoints.c,v 1.1.1.1 2001/05/17 11:50:27 dbalster Exp $ */

/*
 *	GTK+/PSXDEV debugger widgets
 *	
 *	Copyright (C) 1998,1999,2000 Daniel Balster (dbalster@psxdev.de)
 *	http://www.psxdev.de
 *	
 *	This is not free software.
 */

#include <gtk/gtk.h>
#include <gtkbreakpoints.h>

enum {
	SIG_INSERT,
	SIG_REMOVE,
	SIG_CHANGED,
	SIG_LAST
};

static void gtk_breakpoints_class_init (GtkBreakpointsClass *klass);
static void gtk_breakpoints_init (GtkBreakpoints *breakpoints);
static void gtk_breakpoints_destroy (GtkObject *object);

static GtkDataClass *parent_class = NULL;
static guint breakpoints_signals[SIG_LAST] = { 0 };

void gtk_breakpoints_changed (GtkBreakpoints *breakpoints)
{
	GList *list = breakpoints->list;
	GtkBreakpoint *bkpt;
	
	while (list)
	{
		bkpt = list->data;
		list = list->next;
	}
}

GtkBreakpoint *gtk_breakpoints_create (GtkBreakpoints *breakpoints, gulong address)
{
	GtkBreakpoint *bkpt;
	
	bkpt = g_new0(GtkBreakpoint,1);
	
	breakpoints->list = g_list_append (breakpoints->list,bkpt);
	
	bkpt->address = address;
	bkpt->insn = 0xa0d;

	bkpt->enabled = 1;
	
	gtk_signal_emit (GTK_OBJECT (breakpoints), breakpoints_signals[SIG_INSERT],
		bkpt);
	
	return bkpt;
}

GtkBreakpoint *gtk_breakpoints_find (GtkBreakpoints *breakpoints, gulong address)
{
	GList *list = breakpoints->list;
	GtkBreakpoint *bkpt;
	
	while (list)
	{
		bkpt = list->data;
		if (bkpt->address == address) return bkpt;
		list = list->next;
	}
	
	return NULL;
}

void gtk_breakpoints_remove (GtkBreakpoints *breakpoints, GtkBreakpoint *bkpt)
{
	bkpt->enabled = 0;

	breakpoints->list = g_list_remove (breakpoints->list,bkpt);

	gtk_signal_emit (GTK_OBJECT (breakpoints), breakpoints_signals[SIG_REMOVE],
		bkpt);
	
	g_free (bkpt);
}

GtkType
gtk_breakpoints_get_type (void)
{
	static GtkType breakpoints_type = 0;
  
	if (!breakpoints_type)
	{
		GtkTypeInfo breakpoints_info =
		{
			(gchar*)"GtkBreakpoints",
			sizeof (GtkBreakpoints),
			sizeof (GtkBreakpointsClass),
			(GtkClassInitFunc) gtk_breakpoints_class_init,
			(GtkObjectInitFunc) gtk_breakpoints_init,
			/* reserved_1 */ NULL,
			/* reserved_2 */ NULL,
			(GtkClassInitFunc) NULL
		};
		breakpoints_type = gtk_type_unique (gtk_data_get_type (), &breakpoints_info);
	}

	return breakpoints_type;
}

static void
gtk_breakpoints_class_init (GtkBreakpointsClass *klass)
{
	GtkObjectClass *object_class;

	parent_class = gtk_type_class(gtk_data_get_type());
	object_class = (GtkObjectClass*) klass;

	object_class->destroy = gtk_breakpoints_destroy;

	/* register signals */

	breakpoints_signals[SIG_CHANGED] =
	gtk_signal_new ("changed",
		GTK_RUN_FIRST | GTK_RUN_NO_RECURSE,
		object_class->type,
		GTK_SIGNAL_OFFSET (GtkBreakpointsClass, changed),
		gtk_marshal_NONE__NONE,
		GTK_TYPE_NONE, 0);

	breakpoints_signals[SIG_INSERT] =
	gtk_signal_new ("insert",
		GTK_RUN_FIRST | GTK_RUN_NO_RECURSE,
		object_class->type,
		GTK_SIGNAL_OFFSET (GtkBreakpointsClass, insert),
		gtk_marshal_NONE__POINTER,
		GTK_TYPE_NONE, 1, GTK_TYPE_POINTER);

	breakpoints_signals[SIG_REMOVE] =
	gtk_signal_new ("remove",
		GTK_RUN_FIRST | GTK_RUN_NO_RECURSE,
		object_class->type,
		GTK_SIGNAL_OFFSET (GtkBreakpointsClass, remove),
		gtk_marshal_NONE__POINTER,
		GTK_TYPE_NONE, 1, GTK_TYPE_POINTER);

	gtk_object_class_add_signals (object_class, breakpoints_signals, SIG_LAST);

	klass->insert = NULL;
	klass->remove = NULL;
	klass->changed = NULL;
}

static void
gtk_breakpoints_init (GtkBreakpoints *breakpoints)
{
	breakpoints->list = NULL;
}

GtkObject*
gtk_breakpoints_new (void)
{
	GtkBreakpoints *breakpoints;

	breakpoints = gtk_type_new (gtk_breakpoints_get_type());

	return GTK_OBJECT(breakpoints);
}

static void
gtk_breakpoints_destroy(GtkObject *object)
{
	GtkBreakpoints *breakpoints;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GTK_IS_BREAKPOINTS(object));
    
	breakpoints = GTK_BREAKPOINTS(object);

// destroy list, remove breakpoints
//	g_free (breakpoints->buffer);

	if (GTK_OBJECT_CLASS (parent_class)->destroy)
	 (* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}
