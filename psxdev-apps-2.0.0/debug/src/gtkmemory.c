/* $Id: gtkmemory.c,v 1.1.1.1 2001/05/17 11:50:27 dbalster Exp $ */

/*
 *	GTK+/PSXDEV debugger widgets
 *	
 *	Copyright (C) 1998,1999,2000 Daniel Balster (dbalster@psxdev.de)
 *	http://www.psxdev.de
 *	
 *	This is not free software.
 */

#include <gtk/gtk.h>
#include <gtkmemory.h>

enum {
	ARG_0,
	ARG_ADDRESS,
	ARG_SIZE,
};

enum {
	SIG_WRITE_SYNC,
	SIG_READ_SYNC,
	SIG_LAST
};

static void gtk_memory_class_init (GtkMemoryClass *klass);
static void gtk_memory_init (GtkMemory *memory);
static void gtk_memory_destroy (GtkObject *object);
static void gtk_memory_set_arg (GtkObject *object, GtkArg *arg, guint arg_id);
static void gtk_memory_get_arg (GtkObject *object, GtkArg *arg, guint arg_id);

static GtkDataClass *parent_class = NULL;
static guint memory_signals[SIG_LAST] = { 0 };

GtkType
gtk_memory_get_type (void)
{
	static GtkType memory_type = 0;
  
	if (!memory_type)
	{
		GtkTypeInfo memory_info =
		{
			(gchar*)"GtkMemory",
			sizeof (GtkMemory),
			sizeof (GtkMemoryClass),
			(GtkClassInitFunc) gtk_memory_class_init,
			(GtkObjectInitFunc) gtk_memory_init,
			/* reserved_1 */ NULL,
			/* reserved_2 */ NULL,
			(GtkClassInitFunc) NULL
		};
		memory_type = gtk_type_unique (gtk_data_get_type (), &memory_info);
	}

	return memory_type;
}

static void
gtk_memory_class_init (GtkMemoryClass *klass)
{
	GtkObjectClass *object_class;

	parent_class = gtk_type_class(gtk_data_get_type());
	object_class = (GtkObjectClass*) klass;

	object_class->destroy = gtk_memory_destroy;
	object_class->set_arg = gtk_memory_set_arg;
	object_class->get_arg = gtk_memory_get_arg;

	/* register arguments */

	gtk_object_add_arg_type ("GtkMemory::address",
		GTK_TYPE_ULONG,
		GTK_ARG_READWRITE,
		ARG_ADDRESS);

	gtk_object_add_arg_type ("GtkMemory::size",
		GTK_TYPE_ULONG,
		GTK_ARG_READWRITE,
		ARG_SIZE);

	/* register signals */

// the following signal marshaller is bad
// gtk_marshal_NONE__INT_INT_POINTER
// but
// gtk_marshal_NONE__INT_POINTER_INT
// wasn't available

	memory_signals[SIG_READ_SYNC] =
	gtk_signal_new ("read_sync",
		GTK_RUN_FIRST | GTK_RUN_NO_RECURSE,
		object_class->type,
		GTK_SIGNAL_OFFSET (GtkMemoryClass, read_sync),
		gtk_marshal_NONE__INT_INT_POINTER,
		GTK_TYPE_NONE, 3, GTK_TYPE_ULONG, GTK_TYPE_POINTER, GTK_TYPE_ULONG);

	memory_signals[SIG_WRITE_SYNC] =
	gtk_signal_new ("write_sync",
		GTK_RUN_FIRST | GTK_RUN_NO_RECURSE,
		object_class->type,
		GTK_SIGNAL_OFFSET (GtkMemoryClass, write_sync),
		gtk_marshal_NONE__INT_INT_POINTER,
		GTK_TYPE_NONE, 3, GTK_TYPE_ULONG, GTK_TYPE_POINTER, GTK_TYPE_ULONG);

	gtk_object_class_add_signals (object_class, memory_signals, SIG_LAST);

	klass->read_sync = NULL;
	klass->write_sync = NULL;
}

static void
gtk_memory_init (GtkMemory *memory)
{
	memory->address = 0x00000000;
	memory->buffer = NULL;
	memory->size = 0;
}

GtkObject*
gtk_memory_new (gulong address, gulong size)
{
	GtkMemory *memory;

	memory = gtk_type_new (gtk_memory_get_type());

	memory->address = address;
	gtk_memory_set_size (memory, size);

	return GTK_OBJECT(memory);
}

static void
gtk_memory_destroy(GtkObject *object)
{
	GtkMemory *memory;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GTK_IS_MEMORY(object));
    
	memory = GTK_MEMORY(object);

	g_free (memory->buffer);

	if (GTK_OBJECT_CLASS (parent_class)->destroy)
	 (* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}

static void
gtk_memory_set_arg (GtkObject *object,
	GtkArg *arg,
	guint arg_id)
{
	GtkMemory *memory;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GTK_IS_MEMORY(object));
    
	memory = GTK_MEMORY(object);
  
	switch (arg_id)
	{
		case ARG_ADDRESS:
			gtk_memory_set_address (memory, GTK_VALUE_ULONG (*arg));
			break;
		case ARG_SIZE:
			gtk_memory_set_size (memory, GTK_VALUE_ULONG (*arg));
			break;
		default:
			break;
	}
}

static void
gtk_memory_get_arg (GtkObject *object,
	GtkArg *arg,
	guint arg_id)
{
	GtkMemory *memory;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GTK_IS_MEMORY(object));
    
	memory = GTK_MEMORY(object);
  
	switch (arg_id)
	{
		case ARG_ADDRESS:
			GTK_VALUE_ULONG (*arg) = memory->address;
			break;
		case ARG_SIZE:
			GTK_VALUE_ULONG (*arg) = memory->size;
			break;
		default:
			arg->type = GTK_TYPE_INVALID;
			break;
	}
}

void
gtk_memory_read_sync (GtkMemory *memory)
{
	g_return_if_fail (memory != NULL);
	g_return_if_fail (GTK_IS_MEMORY (memory));

	if (memory->buffer && memory->size)
	{
		gtk_signal_emit (GTK_OBJECT (memory), memory_signals[SIG_READ_SYNC],
			memory->address, memory->buffer, memory->size);
	}
}

void
gtk_memory_set_address (GtkMemory *memory, gulong addr)
{
	g_return_if_fail (memory != NULL);
	g_return_if_fail (GTK_IS_MEMORY (memory));
	
	memory->address = addr;

	gtk_memory_read_sync (memory);
}

void
gtk_memory_set_size (GtkMemory *memory, gulong size)
{
	g_return_if_fail (memory != NULL);
	g_return_if_fail (GTK_IS_MEMORY (memory));

	if (size == 0) return;
	
	memory->size = size;
	g_free (memory->buffer);
	memory->buffer = g_malloc (memory->size);

	gtk_memory_read_sync (memory);
}

gulong
gtk_memory_get_size (GtkMemory *memory)
{
	g_return_val_if_fail (memory != NULL, 0);
	g_return_val_if_fail (GTK_IS_MEMORY (memory), 0);

	return memory->size;
}

gulong
gtk_memory_get_address (GtkMemory *memory)
{
	g_return_val_if_fail (memory != NULL, 0);
	g_return_val_if_fail (GTK_IS_MEMORY (memory), 0);

	return memory->address;
}
