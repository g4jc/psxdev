/* $Id: gtkbreakpoints.h,v 1.1.1.1 2001/05/17 11:50:27 dbalster Exp $ */

/*
 *	GTK+/PSXDEV debugger widgets
 *	
 *	Copyright (C) 1998,1999,2000 Daniel Balster (dbalster@psxdev.de)
 *	http://www.psxdev.de
 *	
 *	This is not free software.
 */

#ifndef __GTK_BREAKPOINTS_H__
#define __GTK_BREAKPOINTS_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define GTK_TYPE_BREAKPOINTS          (gtk_breakpoints_get_type())
#define GTK_BREAKPOINTS(obj)          (GTK_CHECK_CAST ((obj), GTK_TYPE_BREAKPOINTS, GtkBreakpoints))
#define GTK_BREAKPOINTS_CLASS(klass)  (GTK_CHECK_CLASS_CAST (klass, GTK_TYPE_BREAKPOINTS, GtkBreakpointsClass))
#define GTK_IS_BREAKPOINTS(obj)       (GTK_CHECK_TYPE ((obj), GTK_TYPE_BREAKPOINTS))
#define GTK_IS_BREAKPOINTS_CLASS      (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_BREAKPOINTS))

typedef struct _GtkBreakpoints		GtkBreakpoints;
typedef struct _GtkBreakpointsClass	GtkBreakpointsClass;

typedef struct _GtkBreakpoint       GtkBreakpoint;

struct _GtkBreakpoint
{
	gulong address;		// breakpoint address
	gulong insn;		// the breakpoint instruction
	gulong safe;		// the saved instruction

	gulong pc_addr;		// for hardware breakpoint support
	gulong pc_mask;
	gulong data_addr;
	gulong data_mask;

	guint32 enabled : 1;
	guint32 temporary : 1;
	guint32 hardware : 1;
};

struct _GtkBreakpoints
{
	GtkData data; /* parent object data */

	GList *list;	// of type GtkBreakpoint
};


struct _GtkBreakpointsClass
{
	GtkDataClass parent_class; /* parent class data */

	void (* changed) (GtkBreakpoints *breakpoints);
	void (* insert) (GtkBreakpoints *breakpoints, GtkBreakpoint *breakpoint);
	void (* remove) (GtkBreakpoints *breakpoints, GtkBreakpoint *breakpoint);
};

GtkType    gtk_breakpoints_get_type   (void);
GtkObject* gtk_breakpoints_new        (void);

GtkBreakpoint *gtk_breakpoints_create (GtkBreakpoints *breakpoints, gulong address);
GtkBreakpoint *gtk_breakpoints_find (GtkBreakpoints *breakpoints, gulong address);
void gtk_breakpoints_remove (GtkBreakpoints *breakpoints, GtkBreakpoint *breakpoint);

void gtk_breakpoints_changed (GtkBreakpoints *breakpoints);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __GTK_BREAKPOINTS_H__ */
