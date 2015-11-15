// GtkR3kDisasm example
// Copyright (C) 2000 by Daniel Balster <dbalster@psxdev.de>

#include <gtk/gtk.h>
#include <gtkr3kdisasm.h>
#include <sys/file.h>

static gchar *filename = NULL;

void on_memory_read_sync (GtkMemory *memory,
	gulong address,
	guchar *buffer,
	gulong size,
	gpointer userdata)
{
	int file;
	
	file = open(filename,O_RDONLY);
	if (file)
	{
		lseek (file, address, 0);
		read (file, buffer, size);
		close (file);
	}
	else perror (filename);
}

int main (int argc, char **argv)
{
	GtkWidget *window;
	GtkWidget *widget;
	GtkMemory *memory;
	GtkBreakpoints *breakpoints;

	gtk_init (&argc, &argv);
	
	if (argc==1)
		g_error("please specify a filename as parameter!");
	
	filename = argv[1];
	
	// create a new window.
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (window),
		"GtkR3kDisasm test");

	// create a memory object
	memory = gtk_memory_new (0,0);

	// create a breakpoints object
	breakpoints = gtk_breakpoints_new ();

	// insert a R3k disasm widget
	widget = gtk_r3k_disasm_new (memory, breakpoints);
	gtk_widget_show (widget);
	gtk_container_add (GTK_CONTAINER (window), widget);

	// connect: click close window to quit
	gtk_signal_connect (GTK_OBJECT (window), "delete_event",
		GTK_SIGNAL_FUNC (gtk_main_quit), NULL);

	gtk_signal_connect (GTK_OBJECT (memory), "read_sync",
		GTK_SIGNAL_FUNC (on_memory_read_sync), NULL);

	if (argc>2)
	{
		gtk_r3k_disasm_set_address (widget,
			strtoul (argv[2],0,0));
	}

	// resize and show the window
	gtk_widget_set_usize (window,500,400);
	gtk_widget_show (window);
	gtk_main();

	return 0;
}
