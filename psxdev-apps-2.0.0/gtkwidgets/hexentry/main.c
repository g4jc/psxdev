// GtkHexEntry example
// Copyright (C) 2000 by Daniel Balster <dbalster@psxdev.de>

#include <gtk/gtk.h>
#include <gtkhexentry.h>

void on_activate (GtkWidget *widget, gpointer data)
{
	guint32 val = gtk_hex_entry_get_value (widget);

	g_print ("on_activate: %08lx\n",val);
}

int main (int argc, char **argv)
{
	GtkWidget *widget;
	GtkWidget *window;

	gtk_init (&argc, &argv);
	
	// create a new window.
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (window),
		"GtkHexEntry test");

	// insert a hex entry widget
	widget = gtk_hex_entry_new (8, 0x12345678);
	gtk_widget_show (widget);
	gtk_container_add (GTK_CONTAINER (window), widget);

	// connect: click close window to quit
	gtk_signal_connect (GTK_OBJECT (window), "delete_event",
		GTK_SIGNAL_FUNC (gtk_main_quit), NULL);

	// connect: print path if changed
	gtk_signal_connect (GTK_OBJECT (widget), "activate",
		GTK_SIGNAL_FUNC (on_activate), NULL);

	// resize and show the window
	gtk_widget_show (window);
	gtk_main();

	return 0;
}
