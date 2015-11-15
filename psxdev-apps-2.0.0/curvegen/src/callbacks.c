#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gnome.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"

extern  GtkWidget *app;


/***********************************************/

gchar *filename = NULL;
GtkWidget *fileselection = NULL;


/***********************************************/

void close_dialog (GtkWidget *unused, GtkWidget *dialog)
{
	gtk_widget_hide (dialog);
}

void select_file (gchar *title, GtkSignalFunc func)
{
	GtkWidget *ok_button;
	GtkWidget *cancel_button;

	if (fileselection==NULL)
	{
		fileselection = gtk_file_selection_new (title);
	}
	
 	ok_button = GTK_FILE_SELECTION (fileselection)->ok_button;
 	cancel_button = GTK_FILE_SELECTION (fileselection)->cancel_button;

	gtk_signal_connect (GTK_OBJECT (ok_button), "clicked",
		GTK_SIGNAL_FUNC (func), NULL);

	gtk_signal_connect (GTK_OBJECT (cancel_button), "clicked",
		GTK_SIGNAL_FUNC (close_dialog), fileselection);

	gtk_widget_show (fileselection);
}

/***********************************************/


void curve_save (gchar *filename, long int seekpos)
{
	GtkWidget *curve = lookup_widget(app,"curve");
	guint count, i;
	gfloat *vector;
	gchar *type;
	FILE *out;
	gfloat minx, maxx, miny, maxy;
	gfloat scale, f;
	guint base;

	minx = gtk_spin_button_get_value_as_float (lookup_widget(app,"minx"));
	maxx = gtk_spin_button_get_value_as_float (lookup_widget(app,"maxx"));
	miny = gtk_spin_button_get_value_as_float (lookup_widget(app,"miny"));
	maxy = gtk_spin_button_get_value_as_float (lookup_widget(app,"maxy"));

	scale = gtk_spin_button_get_value_as_float (lookup_widget(app,"scale"));
	base = gtk_spin_button_get_value_as_int (lookup_widget(app,"base"));
	count = gtk_spin_button_get_value_as_int (lookup_widget(app,"count"));
	type = gtk_entry_get_text (lookup_widget(app,"typeentry"));

	vector = g_new (gfloat,count);
	gtk_curve_get_vector (curve,count,vector);

	out = fopen(filename, "wb");
	if (out == NULL) return;

	if (seekpos) fseek (out,seekpos,0);

	if (strcmp(type,"u_char")==0)
	{
		u_char val;
		
		f = (gfloat)(scale) / (maxy - miny);
	
		for (i=0; i<count; i++)
		{
			val = base + (u_char) (vector[i] * f);
			fwrite(&val,sizeof(val),1,out);
		}
	}

	if (strcmp(type,"char")==0)
	{
		char val;
		
		f = (gfloat)(scale) / (maxy - miny);
	
		for (i=0; i<count; i++)
		{
			val = (char) (vector[i] * f);
			fwrite(&val,sizeof(val),1,out);
		}
	}

	if (strcmp(type,"u_short")==0)
	{
		u_short val;
		
		f = (gfloat)(scale) / (maxy - miny);
	
		for (i=0; i<count; i++)
		{
			val = (u_short) (vector[i] * f);
			fwrite(&val,sizeof(val),1,out);
		}
	}

	if (strcmp(type,"short")==0)
	{
		short val;
		
		f = (gfloat)(scale) / (maxy - miny);
		
		printf ("factor=%f scale=%f (%f)\n",f,scale,maxy-miny);
		
		for (i=0; i<count; i++)
		{
			val = (short) (vector[i] * f);
			fwrite(&val,sizeof(val),1,out);
		}
	}

	if (strcmp(type,"u_long")==0)
	{
		u_long val;
		
		f = (gfloat)(scale) / (maxy - miny);
	
		for (i=0; i<count; i++)
		{
			val = (u_long) (vector[i] * f);
			fwrite(&val,sizeof(val),1,out);
		}
	}

	if (strcmp(type,"long")==0)
	{
		long val;
		
		f = (gfloat)(scale) / (maxy - miny);
	
		for (i=0; i<count; i++)
		{
			val = (long) (vector[i] * f);
			fwrite(&val,sizeof(val),1,out);
		}
	}

	if (strcmp(type,"float")==0)
	{
		fwrite(vector,sizeof(gfloat),count,out);
	}

	if (strcmp(type,"double")==0)
	{
		gdouble val;
	
		for (i=0; i<count; i++)
		{
			val = (gdouble) vector[i];
			fwrite(&val,sizeof(val),1,out);
		}
	}

	if (strcmp(type,"fixed (4+12)")==0)
	{
		u_short val;
		
		f = (gfloat)(scale) / (maxy - miny);
	
		for (i=0; i<count; i++)
		{
			val = base + (u_char) (vector[i] * f);
			fwrite(&val,sizeof(val),1,out);
		}
	}
	
	fclose (out);
	
	g_free (vector);
}

void curve_load (gchar *filename)
{
	GtkWidget *curve = lookup_widget(app,"curve");

	gtk_curve_reset (curve);
}

/***********************************************/

gboolean
on_app_delete_event                    (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
	gtk_main_quit();
	return FALSE;
}

void
on_exit1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	gtk_main_quit();
}


void
on_about1_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
  gtk_widget_show (create_about ());
}


void
on_button_new_clicked                  (GtkButton       *button,
                                        gpointer         user_data)
{
	gtk_curve_reset (lookup_widget(app,"curve"));
	g_free (filename);
	filename = NULL;
	
	gnome_appbar_set_status (lookup_widget(app,"appbar1"),"Curve reset.");
	gtk_window_set_title (app,"Untitled curve");
}


void curve_open_cb (void)
{
	g_free (filename);
	filename = gtk_file_selection_get_filename (fileselection);
	gtk_widget_hide (fileselection);
	gtk_window_set_title (app,filename);
	gnome_appbar_set_status (lookup_widget(app,"appbar1"),"Curve opened.");
	curve_load (filename);
}

void curve_save_cb (void)
{
	g_free (filename);
	filename = gtk_file_selection_get_filename (fileselection);
	gtk_widget_hide (fileselection);
	gtk_window_set_title (app,filename);
	gnome_appbar_set_status (lookup_widget(app,"appbar1"),"Curve saved.");
	curve_save(filename,0);
}

void
on_button_open_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
	select_file ("Open Curve",curve_open_cb);
}


void
on_button_save_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
	if (filename)
	{
		gnome_appbar_set_status (lookup_widget(app,"appbar1"),"Curve saved.");
		curve_save(filename,0);
	}
	else select_file ("Save Curve As",curve_save_cb);
}

void
on_button_save_as_clicked              (GtkButton       *button,
                                        gpointer         user_data)
{
	select_file ("Save Curve As",curve_save_cb);
}


void
on_radiobutton_spline_clicked (GtkButton *button, gpointer user_data)
{
	gtk_curve_set_curve_type (lookup_widget(app,"curve"),GTK_CURVE_TYPE_SPLINE);
}

void
on_radiobutton_linear_clicked (GtkButton *button, gpointer user_data)
{
	gtk_curve_set_curve_type (lookup_widget(app,"curve"),GTK_CURVE_TYPE_LINEAR);
}

void
on_radiobutton_free_clicked (GtkButton *button, gpointer user_data)
{
	gtk_curve_set_curve_type (lookup_widget(app,"curve"),GTK_CURVE_TYPE_FREE);
}


typedef struct {
	gfloat minx, maxx, miny, maxy;
	guint count;
	gchar *type;
	gchar *name;
} list_entry_t;


void add_item (gchar *label, gpointer data)
{
	GtkWidget *curve = lookup_widget(app,"curve");
	GtkWidget *clist = lookup_widget(app,"clist");
	list_entry_t *le;
	gint row;
	gchar *labels[2];

	if (label==NULL) return;

	le = g_new (list_entry_t,1);

	le->minx = gtk_spin_button_get_value_as_float (lookup_widget(app,"minx"));
	le->maxx = gtk_spin_button_get_value_as_float (lookup_widget(app,"maxx"));
	le->miny = gtk_spin_button_get_value_as_float (lookup_widget(app,"miny"));
	le->maxy = gtk_spin_button_get_value_as_float (lookup_widget(app,"maxy"));
	le->count = gtk_spin_button_get_value_as_int (lookup_widget(app,"count"));
	le->type = gtk_entry_get_text (lookup_widget(app,"typeentry"));
	le->name = g_strdup(label);

	labels[0] = "A+";
	labels[1] = le->name;
	row = gtk_clist_append (clist,labels);
	gtk_clist_set_row_data (clist,row,le); 
}

void
on_list_open_clicked                   (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkCList *clist = lookup_widget(app,"clist");
	gint i;
	list_entry_t *le;
	FILE *file;
	gchar *filename;
	gchar line[1000];
	
	filename = g_strdup_printf ("%s/.curvegenrc",g_get_home_dir());
	
	file = fopen(filename,"r");
	if (file)
	{
		gint n;
	
		fgets(line,1000,file);
		n = atoi(line);
	
		for (i=0; i<n; i++)
		{
			gint row;
			gchar *labels[2];

			le = g_new (list_entry_t,1);

			fgets (line,1000,file); le->name = g_strdup(line);
			fgets (line,1000,file); le->minx = atof(line);
			fgets (line,1000,file); le->maxx = atof(line);
			fgets (line,1000,file); le->miny = atof(line);
			fgets (line,1000,file); le->maxy = atof(line);
			fgets (line,1000,file); le->count = atoi(line);
			fgets (line,1000,file); le->type = g_strdup(line);

			labels[0] = "A+";
			labels[1] = le->name;
			row = gtk_clist_append (clist,labels);
			gtk_clist_set_row_data (clist,row,le); 
		}
		fclose (file);
	}
	g_free (filename);
}


void
on_list_save_clicked                   (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkCList *clist = lookup_widget(app,"clist");
	gint i;
	list_entry_t *le;
	FILE *file;
	gchar *filename;
	gchar *line;
	
	filename = g_strdup_printf ("%s/.curvegenrc",g_get_home_dir());
	
	file = fopen(filename,"w");
	if (file)
	{
		fprintf(file,"%d\n",clist->rows);
	
		for (i=0; i<clist->rows; i++)
		{
			le = gtk_clist_get_row_data (clist,i); 
			if (le)
			{
				fprintf(file,"%s\n",le->name );
				fprintf(file,"%f\n",le->minx );
				fprintf(file,"%f\n",le->maxx );
				fprintf(file,"%f\n",le->miny );
				fprintf(file,"%f\n",le->maxy );
				fprintf(file,"%d\n",le->count);
				fprintf(file,"%s\n",le->type);
			}
		}
		fclose (file);
	}
	g_free (filename);
}


void
on_list_add_clicked                    (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget *widget;
	
	widget = gnome_request_dialog (FALSE,"Please enter a name for this setting","unnamed",1000,add_item,NULL,app);

	gtk_widget_show (widget);
}


void
on_list_remove_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget *clist = lookup_widget(app,"clist");
}


void
on_list_use_clicked                    (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget *clist = lookup_widget(app,"clist");
}


void
on_minx_changed                        (GtkEditable     *editable,
                                        gpointer         user_data)
{
	GtkWidget *curve = lookup_widget(app,"curve");
	gfloat minx, maxx, miny, maxy;
	minx = gtk_spin_button_get_value_as_float (lookup_widget(app,"minx"));
	maxx = gtk_spin_button_get_value_as_float (lookup_widget(app,"maxx"));
	miny = gtk_spin_button_get_value_as_float (lookup_widget(app,"miny"));
	maxy = gtk_spin_button_get_value_as_float (lookup_widget(app,"maxy"));
	gtk_curve_set_range (curve,minx,maxx,miny,maxy);

}


void
on_maxx_changed                        (GtkEditable     *editable,
                                        gpointer         user_data)
{
	GtkWidget *curve = lookup_widget(app,"curve");
	gfloat minx, maxx, miny, maxy;
	minx = gtk_spin_button_get_value_as_float (lookup_widget(app,"minx"));
	maxx = gtk_spin_button_get_value_as_float (lookup_widget(app,"maxx"));
	miny = gtk_spin_button_get_value_as_float (lookup_widget(app,"miny"));
	maxy = gtk_spin_button_get_value_as_float (lookup_widget(app,"maxy"));
	gtk_curve_set_range (curve,minx,maxx,miny,maxy);

}


void
on_miny_changed                        (GtkEditable     *editable,
                                        gpointer         user_data)
{
	GtkWidget *curve = lookup_widget(app,"curve");
	gfloat minx, maxx, miny, maxy;
	minx = gtk_spin_button_get_value_as_float (lookup_widget(app,"minx"));
	maxx = gtk_spin_button_get_value_as_float (lookup_widget(app,"maxx"));
	miny = gtk_spin_button_get_value_as_float (lookup_widget(app,"miny"));
	maxy = gtk_spin_button_get_value_as_float (lookup_widget(app,"maxy"));
	gtk_curve_set_range (curve,minx,maxx,miny,maxy);

}


void
on_maxy_changed                        (GtkEditable     *editable,
                                        gpointer         user_data)
{
	GtkWidget *curve = lookup_widget(app,"curve");
	gfloat minx, maxx, miny, maxy;
	minx = gtk_spin_button_get_value_as_float (lookup_widget(app,"minx"));
	maxx = gtk_spin_button_get_value_as_float (lookup_widget(app,"maxx"));
	miny = gtk_spin_button_get_value_as_float (lookup_widget(app,"miny"));
	maxy = gtk_spin_button_get_value_as_float (lookup_widget(app,"maxy"));
	gtk_curve_set_range (curve,minx,maxx,miny,maxy);

}


void
on_button_upload_clicked               (GtkButton       *button,
                                        gpointer         user_data)
{
	int dev;
	gchar *text = gtk_entry_get_text (lookup_widget(app,"hexentry"));
	unsigned long addr;
	
	addr = strtoul (text,NULL,0);

	printf("%08lx\n",addr);
	
	curve_save ("/proc/pccl/0/fullmem",addr);
}

