// GtkDirBrowser example
// Copyright (C) 2000 by Daniel Balster <dbalster@psxdev.de>

#include <gtk/gtk.h>
#include <gtkdirbrowser.h>

void on_path_changed (GtkWidget *widget, const gchar *path)
{
	g_print ("path changed: %s\n",path);
}

int main (int argc, char **argv)
{
	GtkWidget *widget;
	GtkWidget *window;
	GtkWidget *scrolledwindow;

	gtk_init (&argc, &argv);
	
	// create a new window.
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (window),
		"GtkDirBrowser test");

	// insert a scrolled window (viewport)
	scrolledwindow = gtk_scrolled_window_new (0,0);
	gtk_widget_show (scrolledwindow);
	gtk_container_add (GTK_CONTAINER (window), scrolledwindow);

	// insert a dir browser widget
	widget = gtk_dir_browser_new ();
	gtk_widget_show (widget);
	gtk_container_add (GTK_CONTAINER (scrolledwindow), widget);

	// populate the dir browser with some starting points

	gtk_dir_browser_add_root (GTK_DIR_BROWSER(widget),
		"System Root", "/");

	gtk_dir_browser_add_root (GTK_DIR_BROWSER(widget),
		"Temp Dir", "/tmp");

	gtk_dir_browser_add_root (GTK_DIR_BROWSER(widget),
		"Home Dir", g_get_home_dir());

	gtk_dir_browser_add_root (GTK_DIR_BROWSER(widget),
		"Current Dir", g_get_current_dir());

	// connect: click close window to quit
	gtk_signal_connect (GTK_OBJECT (window), "delete_event",
		GTK_SIGNAL_FUNC (gtk_main_quit), NULL);

	// connect: print path if changed
	gtk_signal_connect (GTK_OBJECT (widget), "path_changed",
		GTK_SIGNAL_FUNC (on_path_changed), NULL);

	// resize and show the window
	gtk_widget_set_usize (window,400,400);
	gtk_widget_show (window);
	gtk_main();

	return 0;
}
