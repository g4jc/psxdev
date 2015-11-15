#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gnome.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"

#include <stdlib.h>

gboolean
on_window1_delete_event                (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
	gtk_main_quit();
  return FALSE;
}

void gtk_text_clear (GtkWidget *text)
{
	gtk_editable_delete_text (text,0,-1);
}

void
on_add_clicked                         (GtkButton       *button,
                                        gpointer         user_data)
{
	gchar *body = gtk_editable_get_chars (lookup_widget (button,"body"),0,-1);
	gchar *author = gtk_entry_get_text (lookup_widget (button,"author"));
	gchar *url =  gtk_entry_get_text (lookup_widget (button,"url"));
	gchar *title =  gtk_entry_get_text (lookup_widget (button,"title"));
	FILE *file;
	struct tm *curtime;
	time_t t;
	char buf[1024];
	char *filename;
	
	t = gnome_date_edit_get_date (lookup_widget (button,"dateedit1"));
	curtime = localtime(&t);

	strftime (buf,1024,"%Y.%m.%d-%H:%M:%S",curtime);
	
	filename = g_strdup_printf ("%s/psxdev/data/news/%s",g_get_home_dir(),buf);
	
	strftime (buf,1024,"%c",curtime);
		
	file = fopen (filename,"w");
	if (file)
	{
		fprintf (file,"%s\n",title);
		fprintf (file,"%s\n",author);
		fprintf (file,"%ld\n",t);		// date
		fprintf (file,"%s\n",url);
		fprintf (file,"%s\n",body);
		fclose (file);
	}
	else perror(filename);
	
	g_free (body);
	g_free (filename);
	gtk_file_browser_path_changed(lookup_widget(button,"news"));
}


void
on_clear_clicked                       (GtkButton       *button,
                                        gpointer         user_data)
{
	gtk_editable_delete_text (lookup_widget (button,"body"),0,-1);
	gtk_entry_set_text (lookup_widget (button,"url"),"");
	gtk_entry_set_text (lookup_widget (button,"title"),"");
}


void
on_quit_clicked                        (GtkButton       *button,
                                        gpointer         user_data)
{
	gtk_main_quit();
}

#include <gtkfilebrowser.h>
#include <gtkdirbrowser.h>

GtkWidget*
create_filebrowser (gchar *widget_name, gchar *string1, gchar *string2,
                gint int1, gint int2)
{
	GtkWidget *widget;
	gchar *bla;
	widget = gtk_file_browser_new ();
	
	bla = g_strdup_printf ("%s/%s",g_get_home_dir(),string1);
	gtk_file_browser_set_path (widget,bla);
	g_free (bla);
	gtk_widget_show (widget);
	return widget;
}

GtkWidget*
create_dirbrowser (gchar *widget_name, gchar *string1, gchar *string2,
                gint int1, gint int2)
{
	GtkWidget *widget;
	widget = gtk_dir_browser_new ();
	gtk_dir_browser_add_root (widget,"/","/");
	gtk_dir_browser_add_root (widget,"CurDir",g_get_current_dir());
	gtk_dir_browser_set_path (widget,string1);
	gtk_widget_show (widget);
	
	g_print ("%s\n",string1);
	
	return widget;
}

void load_news (GtkWidget *widget, gchar *manpage)
{
	FILE *file;
	GtkText *text;
	
	file = fopen (manpage,"r");
	if (file)
	{
		gchar buf[1024];
		GtkWidget *text = lookup_widget (widget,"body");

		if (fgets (buf,1024,file)) { buf[strlen(buf)-1]=0;
		gtk_entry_set_text (lookup_widget(widget,"title"),buf);}
		if (fgets (buf,1024,file)) { buf[strlen(buf)-1]=0;
		gtk_entry_set_text (lookup_widget(widget,"author"),buf);}
		if (fgets (buf,1024,file)) { time_t ti;
		
			ti = strtoul (buf,0,0);
			gnome_date_edit_set_time (lookup_widget (widget,"dateedit1"),ti);
		}

		if (fgets (buf,1024,file)) { buf[strlen(buf)-1]=0;
		gtk_entry_set_text (lookup_widget(widget,"url"),buf);}
		
		gtk_text_freeze (text);
		gtk_text_clear (text);
		while (fgets (buf,1024,file))
		{
			gtk_text_insert (text,NULL,NULL,NULL,buf,-1);
		}
		gtk_text_thaw (text);
	}
	fclose (file);
}

void load_manpage (GtkWidget *widget, gchar *manpage)
{
	FILE *file;
	GtkText *text;
	
	file = fopen (manpage,"r");
	if (file)
	{
		gchar buf[1024];
		
		if (fgets (buf,1024,file)) { buf[strlen(buf)-1]=0;
			if (strcmp(buf,"<MANPAGE>"))
			{
				fclose (file);
				return;
			}
		}

		// TITLE == filename
		if (fgets (buf,1024,file)) { buf[strlen(buf)-1]=0;
		gtk_entry_set_text (lookup_widget(widget,"manpage_title"),g_basename(buf));}

		if (fgets (buf,1024,file)) { buf[strlen(buf)-1]=0;
		gtk_entry_set_text (lookup_widget(widget,"manpage_description"),buf);}

		if (fgets (buf,1024,file)) { buf[strlen(buf)-1]=0;
		gtk_entry_set_text (lookup_widget(widget,"manpage_author"),buf);}

		if (fgets (buf,1024,file)) { buf[strlen(buf)-1]=0;
		gtk_entry_set_text (lookup_widget(widget,"manpage_package"),buf);}

		if (fgets (buf,1024,file)) { buf[strlen(buf)-1]=0;
		gtk_entry_set_text (lookup_widget(widget,"manpage_seealso"),buf);}
	
		while (fgets (buf,1024,file))
		{
	
			if (strcmp(buf,"<DESCRIPTION>\n")==0)
			{
				text = lookup_widget (widget,"manpage_body");
				gtk_text_freeze (text);
				gtk_text_clear (text);
				while (fgets (buf,1024,file))
				{
					if (strcmp(buf,"</DESCRIPTION>\n")==0) break;
					
					gtk_text_insert (text,NULL,NULL,NULL,buf,-1);
				}
				gtk_text_thaw (text);
			}
			else if (strcmp(buf,"<EXAMPLES>\n")==0)
			{
				text = lookup_widget (widget,"manpage_examples");
				gtk_text_freeze (text);
				gtk_text_clear (text);
				while (fgets (buf,1024,file))
				{
					if (strcmp(buf,"</EXAMPLES>\n")==0) break;
					
					gtk_text_insert (text,NULL,NULL,NULL,buf,-1);
				}
				gtk_text_thaw (text);
			}
			else if (strcmp(buf,"<BUGS>\n")==0)
			{
				text = lookup_widget (widget,"manpage_bugs");
				gtk_text_freeze (text);
				gtk_text_clear (text);
				while (fgets (buf,1024,file))
				{
					if (strcmp(buf,"</BUGS>\n")==0) break;
					
					gtk_text_insert (text,NULL,NULL,NULL,buf,-1);
				}
				gtk_text_thaw (text);
			}
			else if (strcmp(buf,"<NOTES>\n")==0)
			{
				text = lookup_widget (widget,"manpage_notes");
				gtk_text_freeze (text);
				gtk_text_clear (text);
				while (fgets (buf,1024,file))
				{
					if (strcmp(buf,"</NOTES>\n")==0) break;
					
					gtk_text_insert (text,NULL,NULL,NULL,buf,-1);
				}
				gtk_text_thaw (text);
			}
		}
	
		fclose (file);
	}

//	gtk_file_browser_path_changed(lookup_widget(widget,"manpages"));
}

void save_manpage (GtkWidget *widget, gchar *manpage)
{
	FILE *file;
	
	file = fopen (manpage,"w");
	if (file)
	{
		gchar buf[1024];
		gchar *string;
		GtkWidget *text;
		
		fprintf(file,"<MANPAGE>\n");

		fprintf (file,"%s\n",gtk_entry_get_text (lookup_widget(widget,"manpage_title")));
		fprintf (file,"%s\n",gtk_entry_get_text (lookup_widget(widget,"manpage_description")));
		fprintf (file,"%s\n",gtk_entry_get_text (lookup_widget(widget,"manpage_author")));
		fprintf (file,"%s\n",gtk_entry_get_text (lookup_widget(widget,"manpage_package")));
		fprintf (file,"%s\n",gtk_entry_get_text (lookup_widget(widget,"manpage_seealso")));

		text = lookup_widget (widget,"manpage_body");
		gtk_text_freeze (text);
		string = gtk_editable_get_chars (text,0,-1);
		if (string)
		{
			if (strlen(string)>0)
			{
				fprintf(file,"<DESCRIPTION>\n");
				fprintf(file,string);
				if (string[strlen(string)]!='\n') fprintf(file,"\n");
				fprintf(file,"</DESCRIPTION>\n");
			}
			g_free (string);
		}
		gtk_text_thaw (text);

		text = lookup_widget (widget,"manpage_examples");
		gtk_text_freeze (text);
		string = gtk_editable_get_chars (text,0,-1);
		if (string)
		{
			if (strlen(string)>0)
			{
				fprintf(file,"<EXAMPLES>\n");
				fprintf(file,string);
				if (string[strlen(string)]!='\n') fprintf(file,"\n");
				fprintf(file,"</EXAMPLES>\n");
			}
			g_free (string);
		}
		gtk_text_thaw (text);

		text = lookup_widget (widget,"manpage_bugs");
		gtk_text_freeze (text);
		string = gtk_editable_get_chars (text,0,-1);
		if (string)
		{
			if (strlen(string)>0)
			{
				fprintf(file,"<BUGS>\n");
				fprintf(file,string);
				if (string[strlen(string)]!='\n') fprintf(file,"\n");
				fprintf(file,"</BUGS>\n");
			}
			g_free (string);
		}
		gtk_text_thaw (text);

		text = lookup_widget (widget,"manpage_notes");
		gtk_text_freeze (text);
		string = gtk_editable_get_chars (text,0,-1);
		if (string)
		{
			if (strlen(string)>0)
			{
				fprintf(file,"<NOTES>\n");
				fprintf(file,string);
				if (string[strlen(string)]!='\n') fprintf(file,"\n");
				fprintf(file,"</NOTES>\n");
			}
			g_free (string);
		}
		gtk_text_thaw (text);

		fclose (file);
	}
	
	gtk_file_browser_path_changed(lookup_widget(widget,"manpages"));
}

void on_path_changed (GtkWidget *widget, gchar *path, gpointer data)
{
	gtk_file_browser_set_path (lookup_widget(widget,"manpages"),path);
}

void on_manpage_clicked (GtkWidget *widget, gchar *path, gpointer data)
{
	gtk_button_clicked (lookup_widget(widget,"manpage_clear"));

	load_manpage (widget,path);
}

void
on_manpage_add_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
	gchar *name, *x;
	name = g_strdup_printf ("%s/%s",gtk_file_browser_get_path(
		lookup_widget(button,"manpages")),gtk_entry_get_text (lookup_widget(button,"manpage_title")));
	save_manpage (button,name);
	g_free (name);
}


void
on_manpage_sync_clicked                (GtkButton       *button,
                                        gpointer         user_data)
{
	gtk_file_browser_path_changed(lookup_widget(button,"manpages"));
}

void
on_manpage_clear_clicked                (GtkButton       *widget,
                                        gpointer         user_data)
{
	gtk_entry_set_text (lookup_widget(widget,"manpage_title"),"");
	gtk_entry_set_text (lookup_widget(widget,"manpage_description"),"");
	gtk_entry_set_text (lookup_widget(widget,"manpage_author"),"Daniel Balster <dbalster@psxdev.de>");
	gtk_entry_set_text (lookup_widget(widget,"manpage_package"),"psxdev-tools");
	gtk_entry_set_text (lookup_widget(widget,"manpage_seealso"),"");
	gtk_text_clear (lookup_widget (widget,"manpage_body"));
	gtk_text_clear (lookup_widget (widget,"manpage_examples"));
	gtk_text_clear (lookup_widget (widget,"manpage_bugs"));
	gtk_text_clear (lookup_widget (widget,"manpage_notes"));
}


void
on_manpage_delete_clicked              (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkCList *clist = GTK_CLIST(lookup_widget(button,"manpages"));
	GList *selection = clist->selection;
	gchar *path = gtk_file_browser_get_path(GTK_FILE_BROWSER(clist));
	
	while (selection)
	{
		gchar *entry;
		gchar *filename;
		gtk_clist_get_text (clist,GPOINTER_TO_INT(selection->data),1,&entry);
		selection = selection->next;
		filename = g_strdup_printf ("%s/%s",path,entry);
		unlink (filename);
		g_free (filename);
	
	}
	gtk_file_browser_path_changed(lookup_widget(button,"manpages"));
}


void on_news_clicked (GtkWidget *widget, gchar *path, gpointer data)
{
	gtk_button_clicked (lookup_widget(widget,"clear"));
	load_news (widget,path);
}

void on_news_path_changed (GtkWidget *widget, gchar *path, gpointer data)
{
	gtk_file_browser_set_path (lookup_widget(widget,"news"),path);
}

void
on_news_delete_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkCList *clist = GTK_CLIST(lookup_widget(button,"news"));
	GList *selection = clist->selection;
	gchar *path = gtk_file_browser_get_path(GTK_FILE_BROWSER(clist));
	
	while (selection)
	{
		gchar *entry;
		gchar *filename;
		gtk_clist_get_text (clist,GPOINTER_TO_INT(selection->data),1,&entry);
		selection = selection->next;
		filename = g_strdup_printf ("%s/%s",path,entry);
		unlink (filename);
		g_free (filename);
	}
	gtk_file_browser_path_changed(lookup_widget(button,"news"));
}


void
on_news_sync_clicked                   (GtkButton       *button,
                                        gpointer         user_data)
{
	gtk_file_browser_path_changed(lookup_widget(button,"news"));
}


void
on_button_get_time_and_date_clicked    (GtkButton       *button,
                                        gpointer         user_data)
{
	gnome_date_edit_set_time (lookup_widget (button,"dateedit1"),time(NULL));
}

/*************************************************************************/

/** FAQ stuff **/

void save_faq (GtkWidget *widget, gchar *manpage)
{
	FILE *file;
	
	file = fopen (manpage,"w");
	if (file)
	{
		gchar buf[1024];
		gchar *string;
		GtkWidget *text;
		
		fprintf(file,"<FAQ>\n");

		fprintf (file,"%s\n",gtk_entry_get_text (lookup_widget(widget,"faq_question")));
		fprintf (file,"%s\n",gtk_entry_get_text (lookup_widget(widget,"faq_category")));

		text = lookup_widget (widget,"faq_answer");
		gtk_text_freeze (text);
		string = gtk_editable_get_chars (text,0,-1);
		if (string)
		{
			fprintf(file,string);
			if (string[strlen(string)]!='\n') fprintf(file,"\n");
		}
		gtk_text_thaw (text);
		fclose (file);
	}
	
	gtk_file_browser_path_changed(lookup_widget(widget,"faqs"));
}

void load_faq (GtkWidget *widget, gchar *manpage)
{
	FILE *file;
	GtkText *text;
	
	file = fopen (manpage,"r");
	if (file)
	{
		gchar buf[1024];
		
		if (fgets (buf,1024,file)) { buf[strlen(buf)-1]=0;
			if (strcmp(buf,"<FAQ>"))
			{
				fclose (file);
				return;
			}
		}

		// TITLE == filename
		if (fgets (buf,1024,file)) { buf[strlen(buf)-1]=0;
		gtk_entry_set_text (lookup_widget(widget,"faq_question"),g_basename(buf));}

		if (fgets (buf,1024,file)) { buf[strlen(buf)-1]=0;
		gtk_entry_set_text (lookup_widget(widget,"faq_category"),buf);}

		text = lookup_widget (widget,"faq_answer");
		gtk_text_freeze (text);
		gtk_text_clear (text);
		while (fgets (buf,1024,file)) gtk_text_insert (text,NULL,NULL,NULL,buf,-1);
		gtk_text_thaw (text);

		fclose (file);
	}
}


void on_faq_clicked (GtkWidget *widget, gchar *path, gpointer data)
{
	gtk_button_clicked (lookup_widget(widget,"faq_clear"));
	load_faq (widget,path);
}

void
on_faq_add_clicked                     (GtkButton       *button,
                                        gpointer         user_data)
{
	gchar *name;
	name = g_strdup_printf ("%s/%s",gtk_file_browser_get_path(
		lookup_widget(button,"faqs")),gtk_entry_get_text (lookup_widget(button,"faq_question")));
	save_faq (button,name);
	g_free (name);
}


void
on_faq_clear_clicked                   (GtkButton       *button,
                                        gpointer         user_data)
{
	gtk_entry_set_text (lookup_widget(button,"faq_question"),"");
	gtk_entry_set_text (lookup_widget(button,"faq_category"),"General");
	gtk_text_clear (lookup_widget (button,"faq_answer"));
}


void
on_faq_delete_clicked                  (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkCList *clist = GTK_CLIST(lookup_widget(button,"faqs"));
	GList *selection = clist->selection;
	gchar *path = gtk_file_browser_get_path(GTK_FILE_BROWSER(clist));
	while (selection)
	{
		gchar *entry;
		gchar *filename;
		gtk_clist_get_text (clist,GPOINTER_TO_INT(selection->data),1,&entry);
		selection = selection->next;
		filename = g_strdup_printf ("%s/%s",path,entry);
		unlink (filename);
		g_free (filename);
	}
	gtk_file_browser_path_changed(lookup_widget(button,"faqs"));
}


void
on_faq_sync_clicked                    (GtkButton       *button,
                                        gpointer         user_data)
{
	gtk_file_browser_path_changed(lookup_widget(button,"faqs"));
}

/**************************************************************************/

/** Screenshot stuff **/


void save_screenshot (GtkWidget *widget, gchar *manpage)
{
	FILE *file;
	
	file = fopen (manpage,"w");
	if (file)
	{
		gchar buf[1024];
		gchar *string;
		GtkWidget *text;
		
		fprintf(file,"<SCREENSHOT>\n");

		fprintf (file,"%s\n",gtk_entry_get_text (lookup_widget(widget,"screenshot_title")));
		fprintf (file,"%s\n",gtk_entry_get_text (lookup_widget(widget,"screenshot_filename")));

		text = lookup_widget (widget,"screenshot_comment");
		gtk_text_freeze (text);
		string = gtk_editable_get_chars (text,0,-1);
		if (string)
		{
			fprintf(file,string);
			if (string[strlen(string)]!='\n') fprintf(file,"\n");
		}
		gtk_text_thaw (text);
		fclose (file);
	}
	
	gtk_file_browser_path_changed(lookup_widget(widget,"screenshots"));
}

void load_screenshot (GtkWidget *widget, gchar *manpage)
{
	FILE *file;
	GtkText *text;
	
	file = fopen (manpage,"r");
	if (file)
	{
		gchar buf[1024];
		
		if (fgets (buf,1024,file)) { buf[strlen(buf)-1]=0;
			if (strcmp(buf,"<SCREENSHOT>"))
			{
				fclose (file);
				return;
			}
		}

		// TITLE == filename
		if (fgets (buf,1024,file)) { buf[strlen(buf)-1]=0;
		gtk_entry_set_text (lookup_widget(widget,"screenshot_title"),g_basename(buf));}

		if (fgets (buf,1024,file)) { buf[strlen(buf)-1]=0;
		gtk_entry_set_text (lookup_widget(widget,"screenshot_filename"),buf);}

		text = lookup_widget (widget,"screenshot_comment");
		gtk_text_freeze (text);
		gtk_text_clear (text);
		while (fgets (buf,1024,file)) gtk_text_insert (text,NULL,NULL,NULL,buf,-1);
		gtk_text_thaw (text);

		fclose (file);
	}
}


void on_screenshot_clicked (GtkWidget *widget, gchar *path, gpointer data)
{
	gtk_button_clicked (lookup_widget(widget,"screenshot_clear"));
	load_screenshot (widget,path);
}


void
on_screenshot_add_clicked              (GtkButton       *button,
                                        gpointer         user_data)
{
	gchar *name;
	name = g_strdup_printf ("%s/%s",gtk_file_browser_get_path(
		lookup_widget(button,"screenshots")),gtk_entry_get_text (lookup_widget(button,"screenshot_title")));
	save_screenshot (button,name);
	g_free (name);
}


void
on_screenshot_clear_clicked            (GtkButton       *button,
                                        gpointer         user_data)
{
	gtk_entry_set_text (lookup_widget(button,"screenshot_title"),"");
	gtk_entry_set_text (lookup_widget(button,"screenshot_filename"),"");
	gtk_text_clear (lookup_widget (button,"screenshot_comment"));
}


void
on_screenshot_delete_clicked           (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkCList *clist = GTK_CLIST(lookup_widget(button,"screenshots"));
	GList *selection = clist->selection;
	gchar *path = gtk_file_browser_get_path(GTK_FILE_BROWSER(clist));
	while (selection)
	{
		gchar *entry;
		gchar *filename;
		gtk_clist_get_text (clist,GPOINTER_TO_INT(selection->data),1,&entry);
		selection = selection->next;
		filename = g_strdup_printf ("%s/%s",path,entry);
		unlink (filename);
		g_free (filename);
	}
	gtk_file_browser_path_changed(lookup_widget(button,"screenshots"));
}


void
on_screenshot_sync_clicked             (GtkButton       *button,
                                        gpointer         user_data)
{
	gtk_file_browser_path_changed(lookup_widget(button,"screenshots"));
}

