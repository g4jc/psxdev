/* $Id: gtkmemory.h,v 1.1.1.1 2001/05/17 11:50:27 dbalster Exp $ */

/*
 *	GTK+/PSXDEV debugger widgets
 *	
 *	Copyright (C) 1998,1999,2000 Daniel Balster (dbalster@psxdev.de)
 *	http://www.psxdev.de
 *	
 *	This is not free software.
 */

#ifndef __GTK_MEMORY_H__
#define __GTK_MEMORY_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define GTK_TYPE_MEMORY          (gtk_memory_get_type())
#define GTK_MEMORY(obj)          (GTK_CHECK_CAST ((obj), GTK_TYPE_MEMORY, GtkMemory))
#define GTK_MEMORY_CLASS(klass)  (GTK_CHECK_CLASS_CAST (klass, GTK_TYPE_MEMORY, GtkMemoryClass))
#define GTK_IS_MEMORY(obj)       (GTK_CHECK_TYPE ((obj), GTK_TYPE_MEMORY))
#define GTK_IS_MEMORY_CLASS      (GTK_CHECK_CLASS_TYPE ((klass), GTK_TYPE_MEMORY))

typedef struct _GtkMemory		GtkMemory;
typedef struct _GtkMemoryClass	GtkMemoryClass;

struct _GtkMemory
{
	GtkData data; /* parent object data */
	
	gulong address;
	guchar *buffer;
	gulong size;
};

struct _GtkMemoryClass
{
	GtkDataClass parent_class; /* parent class data */

	void (* read_sync) (GtkMemory *memory, gulong address, guchar *buffer, gulong size);
	void (* write_sync) (GtkMemory *memory, gulong address, guchar *buffer, gulong size);
};

GtkType    gtk_memory_get_type   (void);
GtkObject* gtk_memory_new        (gulong address, gulong size);

void gtk_memory_read_sync (GtkMemory *memory);

void gtk_memory_set_address (GtkMemory *memory, gulong addr);
gulong gtk_memory_get_address (GtkMemory *memory);
void gtk_memory_set_size (GtkMemory *memory, gulong size);
gulong gtk_memory_get_size (GtkMemory *memory);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __GTK_MEMORY_H__ */
