#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gnome.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"
#include <gtkdirbrowser.h>
#include <gtk-xmhtml/gtk-xmhtml.h>

#include <zvt/zvtterm.h>

#include <sys/vfs.h>
#include <signal.h>

#include <sys/stat.h>
#include <unistd.h>

extern GtkWidget *app;
GtkWidget *term;

void on_open_clicked (GtkButton *button, gpointer user_data);

gchar *currentdir = NULL;
void load_doc (void);

void init (void)
{
	currentdir = g_strdup (g_get_current_dir());

	on_open_clicked(0,0);
	chdir (currentdir);

	load_doc();
}

void load_doc (void)
{
	GtkCTreeNode *node, *parent[100];
	GtkCTree *ctree;
	gint row;
	gchar *label[2];
	gchar buf[4096];
	FILE *file;
	gint level = 0;
	GString *string;
	gchar *docpath;
	
	chdir (currentdir);
	
	ctree = lookup_widget(app,"ctree");
	
	docpath = g_strdup_printf ("bzcat %s/share/doc/psxcdmaker.doc.bz2",getenv("PSXDEV"));
	
	file = popen(docpath,"r");
	if (file == NULL) return;

	string = g_string_new ("");
	
	gtk_clist_freeze (GTK_CLIST(ctree));
	gtk_clist_clear (GTK_CLIST(ctree));
	
	parent[0] = NULL;
	label[0] = NULL;
	label[1] = NULL;
	
	while (fgets(buf,4096,file))
	{
		if (level==-1) break;
	
		if (strstr(buf,"<section>"))
		{
			label[0] = fgets(buf,4096,file);
			parent[level+1] = gtk_ctree_insert_node (ctree,parent[level],NULL,label,0,NULL,NULL,NULL,NULL,FALSE,FALSE);
			level++;
			continue;
		}

		if (strstr(buf,"</section>"))
		{
			level--;
			continue;
		}

		if (strstr(buf,"<title>"))
		{
			*strstr(buf,"</title>")=0;
			label[0] = strstr(buf,"<title>")+strlen("<title>");
			
			node = gtk_ctree_insert_node (ctree,parent[level],NULL,label,0,NULL,NULL,NULL,NULL,TRUE,FALSE);
			
			g_string_truncate (string,0);
			continue;
		}

		if (strstr(buf,"</body>\n"))
		{
			gtk_ctree_node_set_row_data (ctree,node,g_strdup(string->str));
		}
		
		g_string_append (string,buf);		
	}
	
	gtk_clist_thaw (GTK_CLIST(ctree));
	
	g_string_free (string,1);
	
	gtk_clist_select_row (ctree,0,0);

	pclose (file);
	g_free (docpath);
}

void
on_exit_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	chdir (currentdir);
	gtk_main_quit();
}


void
on_about_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	gtk_widget_show(create_about1());
}


GtkWidget*
create_dirbrowser (gchar *widget_name, gchar *string1, gchar *string2,
                gint int1, gint int2)
{
	GtkWidget *widget;
	widget = gtk_dir_browser_new ();
	gtk_widget_show (widget);

	gtk_dir_browser_add_root (widget,"System Root","/");
	gtk_dir_browser_add_root (widget,"Temp. Directory","/tmp");
	gtk_dir_browser_add_root (widget,"Current Directory",g_get_current_dir());
	gtk_dir_browser_add_root (widget,"Home Directory",g_get_home_dir());
	gtk_dir_browser_add_root (widget,"GNOME Desktop",g_strdup_printf("%s/.gnome-desktop",g_get_home_dir()));

	return widget;
}

gboolean
on_app1_delete_event                   (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
	chdir (currentdir);
	gtk_main_quit();
  return FALSE;
}


void
on_new_clicked                         (GtkButton       *button,
                                        gpointer         user_data)
{
	chdir (currentdir);
	// clear all fields
}

void add_xafiles (GtkWidget *w, gchar *x)
{
	gchar *label[1];
	label[0] = x;
	gtk_clist_append (lookup_widget(app,"xafiles_clist"),label);
}

void add_exclusion (GtkWidget *w, gchar *x)
{
	gchar *label[1];
	label[0] = x;
	gtk_clist_append (lookup_widget(app,"exclusion_clist"),label);
}

void add_audio (GtkWidget *w, gchar *x)
{
	gchar *label[7];
	label[0] = x;
	label[1] = 0;
	label[2] = "off";
	label[3] = "off";
	label[4] = 0;
	gtk_clist_append (lookup_widget(app,"audio"),label);
}

void add_copy (GtkWidget *w, gchar *x)
{
	GtkCList *clist = lookup_widget(app,"audio");
	gtk_clist_set_text (clist,clist->rows-1,2,x);
}

void add_preemp (GtkWidget *w, gchar *x)
{
	GtkCList *clist = lookup_widget(app,"audio");
	gtk_clist_set_text (clist,clist->rows-1,3,x);
}

void
on_open_clicked                        (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget *w;
	gchar *p;
	FILE *file;
	gchar buf[4096];
	gint i;
	
	chdir (currentdir);

	file = fopen(".psxcdmakerrc","r");
	if (file==NULL) return;

	gtk_clist_clear (lookup_widget(app,"xafiles_clist"));
	gtk_clist_clear (lookup_widget(app,"exclusion_clist"));
	gtk_clist_clear (lookup_widget(app,"audio"));

	while (fgets(buf,4096,file))
	{
		gint len = strlen(buf);
		
		if (len<2) continue;
		buf[len-1] = 0;

		#define checkin(option,function,param) \
		if (strncmp(option,buf,strlen(option))==0)\
		{\
			w = lookup_widget (app,option);\
			p = index(buf,'=');\
			p++;\
			function(w,param);\
		}
		
		checkin("xafiles",add_xafiles,p);
		checkin("exclusion",add_exclusion,p);

		checkin("audio",add_audio,p);
		checkin("copypermission",add_copy,p);
		checkin("preemphasis",add_preemp,p);
		
		checkin("input_path",gtk_dir_browser_set_path,p);
		checkin("output_path",gtk_dir_browser_set_path,p);

		checkin("volume_id",gtk_entry_set_text,p);
		checkin("volume_set",gtk_entry_set_text,p);
		checkin("publisher",gtk_entry_set_text,p);
		checkin("data_preparer",gtk_entry_set_text,p);
		checkin("application_id",gtk_entry_set_text,p);
		checkin("system_id",gtk_entry_set_text,p);

		checkin("followlinks",gtk_toggle_button_set_active,atoi(p));

		checkin("use_abstract",gtk_toggle_button_set_active,atoi(p));
		checkin("abstract",gtk_entry_set_text,p);
		checkin("use_copyright",gtk_toggle_button_set_active,atoi(p));
		checkin("copyright",gtk_entry_set_text,p);
		checkin("use_bibliography",gtk_toggle_button_set_active,atoi(p));
		checkin("bibliography",gtk_entry_set_text,p);

		checkin("use_creation",gtk_toggle_button_set_active,atoi(p));
		checkin("creation",gnome_date_edit_set_time,atoi(p));
		checkin("use_expiration",gtk_toggle_button_set_active,atoi(p));
		checkin("expiration",gnome_date_edit_set_time,atoi(p));
		checkin("use_modification",gtk_toggle_button_set_active,atoi(p));
		checkin("modification",gnome_date_edit_set_time,atoi(p));
		checkin("use_effective",gtk_toggle_button_set_active,atoi(p));
		checkin("effective",gnome_date_edit_set_time,atoi(p));

		checkin("tcb",gtk_entry_set_text,p);
		checkin("event",gtk_entry_set_text,p);
		checkin("stack",gtk_entry_set_text,p);
		checkin("boot",gtk_entry_set_text,p);
		checkin("use_systemcnf",gtk_toggle_button_set_active,atoi(p));
		checkin("license",gtk_entry_set_text,p);

		checkin("device",gtk_entry_set_text,p);
		checkin("driver",gtk_entry_set_text,p);
		checkin("speed",gtk_spin_button_set_value,atoi(p));
		checkin("buffers",gtk_spin_button_set_value,atoi(p));
		checkin("simulate",gtk_toggle_button_set_active,atoi(p));
		checkin("swabaudio",gtk_toggle_button_set_active,atoi(p));
		checkin("reloadtray",gtk_toggle_button_set_active,atoi(p));
		checkin("eject",gtk_toggle_button_set_active,atoi(p));
		checkin("force",gtk_toggle_button_set_active,atoi(p));
		checkin("tenseconds",gtk_toggle_button_set_active,atoi(p));
		checkin("onthefly",gtk_toggle_button_set_active,atoi(p));

		checkin("cdrdao",gtk_entry_set_text,p);
		checkin("mkisofs",gtk_entry_set_text,p);
		checkin("diskusage",gtk_entry_set_text,p);
		checkin("bash",gtk_entry_set_text,p);
		checkin("isoname",gtk_entry_set_text,p);
		checkin("tocname",gtk_entry_set_text,p);
	}

	fclose (file);
	
	on_audio_refresh_clicked (0,0);
}


void
on_save_clicked                        (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget *w;
	gchar *string;
	gint integer;
	gboolean boolean;
	FILE *file;
	
	chdir (currentdir);

	file = fopen(".psxcdmakerrc","w+");
	if (file==NULL)
	{
		perror(0);
		return;
	}
	
	#define checkout(option,pattern,function)\
		fprintf (file,option ## "=" ## pattern ## "\n",function (lookup_widget (app,option)))

	checkout ("input_path","%s",gtk_dir_browser_get_path);
	checkout ("output_path","%s",gtk_dir_browser_get_path);

	checkout ("volume_id","%s",gtk_entry_get_text);
	checkout ("volume_set","%s",gtk_entry_get_text);
	checkout ("publisher","%s",gtk_entry_get_text);
	checkout ("data_preparer","%s",gtk_entry_get_text);
	checkout ("application_id","%s",gtk_entry_get_text);
	checkout ("system_id","%s",gtk_entry_get_text);

	checkout ("followlinks","%d",gtk_toggle_button_get_active);

	checkout ("use_abstract","%d",gtk_toggle_button_get_active);
	checkout ("abstract","%s",gtk_entry_get_text);
	checkout ("use_copyright","%d",gtk_toggle_button_get_active);
	checkout ("copyright","%s",gtk_entry_get_text);
	checkout ("use_bibliography","%d",gtk_toggle_button_get_active);
	checkout ("bibliography","%s",gtk_entry_get_text);

	checkout ("use_creation","%d",gtk_toggle_button_get_active);
	checkout ("creation","%d",gnome_date_edit_get_date);
	checkout ("use_expiration","%d",gtk_toggle_button_get_active);
	checkout ("expiration","%d",gnome_date_edit_get_date);
	checkout ("use_modification","%d",gtk_toggle_button_get_active);
	checkout ("modification","%d",gnome_date_edit_get_date);
	checkout ("use_effective","%d",gtk_toggle_button_get_active);
	checkout ("effective","%d",gnome_date_edit_get_date);

	checkout ("tcb","%s",gtk_entry_get_text);
	checkout ("event","%s",gtk_entry_get_text);
	checkout ("stack","%s",gtk_entry_get_text);
	checkout ("boot","%s",gtk_entry_get_text);
	checkout ("use_systemcnf","%d",gtk_toggle_button_get_active);
	checkout ("license","%s",gtk_entry_get_text);

	checkout ("device","%s",gtk_entry_get_text);
	checkout ("driver","%s",gtk_entry_get_text);
	checkout ("speed","%d",gtk_spin_button_get_value_as_int);
	checkout ("buffers","%d",gtk_spin_button_get_value_as_int);
	checkout ("simulate","%d",gtk_toggle_button_get_active);
	checkout ("swabaudio","%d",gtk_toggle_button_get_active);
	checkout ("reloadtray","%d",gtk_toggle_button_get_active);
	checkout ("eject","%d",gtk_toggle_button_get_active);
	checkout ("force","%d",gtk_toggle_button_get_active);
	checkout ("tenseconds","%d",gtk_toggle_button_get_active);
	checkout ("onthefly","%d",gtk_toggle_button_get_active);

	checkout ("cdrdao","%s",gtk_entry_get_text);
	checkout ("mkisofs","%s",gtk_entry_get_text);
	checkout ("diskusage","%s",gtk_entry_get_text);
	checkout ("bash","%s",gtk_entry_get_text);
	checkout ("isoname","%s",gtk_entry_get_text);
	checkout ("tocname","%s",gtk_entry_get_text);

	//// comment

	{
		GtkCList *clist;
		gint i;
	
		clist = lookup_widget (app,"exclusion_clist");
	
		for (i=0; i<clist->rows; i++)
		{
			gchar *text;
			gtk_clist_get_text (clist,i,0,&text);
			fprintf (file,"exclusion=%s\n",text);
		}

		clist = lookup_widget (app,"xafiles_clist");
	
		for (i=0; i<clist->rows; i++)
		{
			gchar *text;
			gtk_clist_get_text (clist,i,0,&text);
			fprintf (file,"xafiles=%s\n",text);
		}

		clist = lookup_widget (app,"audio");
	
		for (i=0; i<clist->rows; i++)
		{
			gchar *text;
			gtk_clist_get_text (clist,i,0,&text);
			fprintf (file,"audio=%s\n",text);
			gtk_clist_get_text (clist,i,2,&text);
			fprintf (file,"copypermission=%s\n",text);
			gtk_clist_get_text (clist,i,3,&text);
			fprintf (file,"preemphasis=%s\n",text);
		}
	}

	fclose (file);
}


void
on_exit_clicked                        (GtkButton       *button,
                                        gpointer         user_data)
{
	chdir (currentdir);
	gtk_main_quit();
}


void
on_use_systemcnf_toggled               (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	gtk_widget_set_sensitive (lookup_widget(app,"systemcnf"),
		gtk_toggle_button_get_active(togglebutton));
}


void
on_use_abstract_toggled                (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	gtk_widget_set_sensitive (lookup_widget(app,"fileentry_abstract"),
		gtk_toggle_button_get_active(togglebutton));
}


void
on_use_copyright_toggled               (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	gtk_widget_set_sensitive (lookup_widget(app,"fileentry_copyright"),
		gtk_toggle_button_get_active(togglebutton));
}


void
on_use_bibliography_toggled            (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	gtk_widget_set_sensitive (lookup_widget(app,"fileentry_bibliography"),
		gtk_toggle_button_get_active(togglebutton));
}


void
on_use_creation_toggled                (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	gtk_widget_set_sensitive (lookup_widget(app,"creation"),
		gtk_toggle_button_get_active(togglebutton));
}


void
on_use_expiration_toggled              (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	gtk_widget_set_sensitive (lookup_widget(app,"expiration"),
		gtk_toggle_button_get_active(togglebutton));
}


void
on_use_modification_toggled            (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	gtk_widget_set_sensitive (lookup_widget(app,"modification"),
		gtk_toggle_button_get_active(togglebutton));
}


void
on_use_effective_toggled               (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	gtk_widget_set_sensitive (lookup_widget(app,"effective"),
		gtk_toggle_button_get_active(togglebutton));
}


void
on_ctree_tree_select_row               (GtkCTree        *ctree,
                                        GList           *node,
                                        gint             column,
                                        gpointer         user_data)
{
	GtkWidget *html = lookup_widget(app,"html");
	gchar *x;
	if (node==NULL) return;
	x = gtk_ctree_node_get_row_data (ctree,node);
	if (x == NULL) return;
	gtk_xmhtml_freeze (html);
	gtk_xmhtml_source (html,x);
	gtk_xmhtml_thaw (html);
}

void
on_link_activate (GtkWidget *widget, XmHTMLAnchorCallbackStruct *cbs, gpointer data)
{
	if (strncasecmp(cbs->href,"tab:",4)==0)
	{
		gint page = atoi(cbs->href+4);
		gtk_notebook_set_page (lookup_widget(app,"notebook"),page);
	}

	else if (strncasecmp(cbs->href,"toggle:",7)==0)
	{
		GtkWidget *w;
		gchar *s = g_strdup((cbs->href+7));
		gchar *p = index(s,'=');
		*p = 0; p++;
		
		w = lookup_widget (app,s);
		if (w) gtk_toggle_button_set_active (w,atoi(p));
		
		g_free (s);
	}

	else if (strncasecmp(cbs->href,"entry:",6)==0)
	{
		GtkWidget *w;
		gchar *s = g_strdup((cbs->href+6));
		gchar *p = index(s,'=');
		*p = 0; p++;
		
		w = lookup_widget (app,s);
		if (w) gtk_entry_set_text (w,p);
		
		g_free (s);
	}
	else if (strncasecmp(cbs->href,"man:",4)==0)
	{
		gchar *remote = g_strdup_printf (
		"gnome-help-browser %s &"
		,cbs->href);
		system (remote);
		g_free (remote);
	}
	else
	{
		gchar *remote = g_strdup_printf (
		"gnome-moz-remote --remote='OpenURL(%s)' --newwin --local &"
		,cbs->href);
		system (remote);
		g_free (remote);
	}
}

GtkWidget*
create_html (gchar *widget_name, gchar *string1, gchar *string2,
                gint int1, gint int2)
{
	GtkWidget *widget = gtk_xmhtml_new ();
	
	gtk_xmhtml_set_hilight_on_enter (widget,1);
	gtk_xmhtml_set_anchor_underline_type (widget,GTK_ANCHOR_DASHED_LINE);
	gtk_xmhtml_set_anchor_buttons (widget,0);
	gtk_xmhtml_set_anchor_cursor (widget,gdk_cursor_new(GDK_HAND2),1);
	gtk_signal_connect (widget,"activate", GTK_SIGNAL_FUNC(on_link_activate),1);
	gtk_widget_show (widget);
	
	return widget;
}

/***********************************************************************/

void
on_exclusion_clist_select_row          (GtkCList        *clist,
                                        gint             row,
                                        gint             column,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
	GtkWidget *widget = lookup_widget(app,"exclusion");
	gchar *text;
	
	gtk_clist_get_text (clist,row,0,&text);
	gtk_entry_set_text (widget,text);
}


void
on_exclusion_changed                   (GtkEditable     *editable,
                                        gpointer         user_data)
{
	gint row;
	GtkCList *clist = lookup_widget(app,"exclusion_clist");
	gchar *text = gtk_entry_get_text (editable);

	if (clist->selection)
	{
		row = GPOINTER_TO_INT(clist->selection->data);
		gtk_clist_set_text (clist,row,0,text);
	}
}


void
on_exclusion_add_clicked               (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget *widget = lookup_widget(app,"exclusion");
	GtkWidget *clist = lookup_widget(app,"exclusion_clist");
	gchar *label[1];
	
	label[0] = gtk_entry_get_text (widget);
	gtk_clist_append (clist,label);
	gtk_entry_set_text (lookup_widget(app,"exclusion"),"");
}


void
on_exclusion_remove_clicked            (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkCList *clist = lookup_widget(app,"exclusion_clist");
	gint row;

	row = GPOINTER_TO_INT(clist->selection->data);
	gtk_clist_remove (clist,row);
	gtk_entry_set_text (lookup_widget(app,"exclusion"),"");
}


void
on_exclusion_defaults_clicked          (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget *clist = lookup_widget(app,"exclusion_clist");

	gtk_clist_freeze (clist);
	gtk_clist_clear (clist);
	gtk_clist_thaw (clist);
}

/***********************************************************************/

void
on_xafiles_clist_select_row            (GtkCList        *clist,
                                        gint             row,
                                        gint             column,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
	GtkWidget *widget = lookup_widget(app,"xafiles");
	gchar *text;
	
	gtk_clist_get_text (clist,row,0,&text);
	gtk_entry_set_text (widget,text);
}


void
on_xafiles_changed                     (GtkEditable     *editable,
                                        gpointer         user_data)
{
	gint row;
	GtkCList *clist = lookup_widget(app,"xafiles_clist");
	gchar *text = gtk_entry_get_text (editable);

	if (clist->selection)
	{
		row = GPOINTER_TO_INT(clist->selection->data);
		gtk_clist_set_text (clist,row,0,text);
	}
}


void
on_xafiles_add_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget *widget = lookup_widget(app,"xafiles");
	GtkWidget *clist = lookup_widget(app,"xafiles_clist");
	gchar *label[1];
	
	label[0] = gtk_entry_get_text (widget);
	gtk_clist_append (clist,label);
	gtk_entry_set_text (lookup_widget(app,"xafiles"),"");
}


void
on_xafiles_remove_clicked              (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkCList *clist = lookup_widget(app,"xafiles_clist");
	gint row;

	row = GPOINTER_TO_INT(clist->selection->data);
	gtk_clist_remove (clist,row);
	gtk_entry_set_text (lookup_widget(app,"xafiles"),"");
}


void
on_xafiles_defaults_clicked            (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget *clist = lookup_widget(app,"xafiles_clist");

	gtk_clist_freeze (clist);
	gtk_clist_clear (clist);
	gtk_clist_thaw (clist);
}

/***********************************************************************/


void
on_getsize_clicked                     (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget *progress = lookup_widget (app,"input");
	FILE *process;
	gchar *cmd = g_strdup_printf ("%s %s",
		gtk_entry_get_text (lookup_widget (app,"diskusage")),
		gtk_dir_browser_get_path(lookup_widget (app,"input_path")));
	
	process = popen(cmd,"r");
	if (process)
	{
		gchar buf[1024];
		
		if (fgets (buf,1024,process))
		{
			gint x = atoi(buf);
			
			if (x <= 750)
			{
				gtk_progress_configure (progress,(gfloat)x,0.0,750.0);
			}
			else
			{
				gtk_progress_configure (progress,(gfloat)x,0.0,(gfloat)x);
			}
		}
		else perror(0);
	
		pclose (process);
	}
	else perror(0);
	
	g_free (cmd);
}

void
on_output_path_changed (GtkWidget *widget, gchar *path)
{
	GtkWidget *progress = lookup_widget (app,"output");
	struct statfs st;

	if (statfs (path,&st) != -1)
	{
		gtk_progress_configure (progress,
			(gfloat)((gfloat)st.f_bavail*(gfloat)st.f_bsize)/(1024.0*1024.0),
			0.0,
			(gfloat)((gfloat)st.f_blocks*(gfloat)st.f_bsize)/(1024.0*1024.0)
		);
	}
	else perror (path);
}

void
on_input_path_changed (GtkWidget *widget, gchar *path)
{
	// dummy function (not used anymore)
}

/***********************************************************************/

#define getparam(name,func) func(lookup_widget(app,name))

gchar **make_call_for_master (void)
{
	gchar **call;
	GtkCList *clist;
	gint i,j;

	call = g_new (gchar*,40);
	i=0;

	call[i++] = gtk_entry_get_text (lookup_widget (app,"mkisofs"));

	call[i++] =	"-appid";
	call[i++] =	getparam ("application_id",gtk_entry_get_text);
	call[i++] =	"-preparer";
	call[i++] =	getparam ("data_preparer",gtk_entry_get_text);
	call[i++] =	"-publisher";
	call[i++] =	getparam ("publisher",gtk_entry_get_text);
	call[i++] =	"-sysid";
	call[i++] =	getparam ("system_id",gtk_entry_get_text);
	call[i++] =	"-volid";
	call[i++] = getparam ("volume_id",gtk_entry_get_text);
	call[i++] =	"-volset";
	call[i++] =	getparam ("volume_set",gtk_entry_get_text);

	if (getparam ("followlinks",gtk_toggle_button_get_active))
	{
		call[i++] =	"-f";
	}

	if (getparam ("use_abstract",gtk_toggle_button_get_active))
	{
		call[i++] =	"-abstract";
		call[i] =	getparam ("abstract",gtk_entry_get_text);
		if (strlen(call[i])) i++; else i--;
	}

	if (getparam ("use_copyright",gtk_toggle_button_get_active))
	{
		call[i++] =	"-copyright";
		call[i] =	getparam ("copyright",gtk_entry_get_text);
		if (strlen(call[i])) i++; else i--;
	}

	if (getparam ("use_bibliography",gtk_toggle_button_get_active))
	{
		call[i++] =	"-biblio";
		call[i] =	getparam ("bibliography",gtk_entry_get_text);
		if (strlen(call[i])) i++; else i--;
	}

	// spit out license file

	call[i++] =	"-psx-license";
	call[i] =	getparam ("license",gtk_entry_get_text);
	if (strlen(call[i])) i++; else i--;

	clist = lookup_widget (app,"exclusion_clist");
	
	for (j=0; j<clist->rows; j++)
	{
		gchar *text;
		gtk_clist_get_text (clist,j,0,&text);
		call[i++] =	"-m";
		call[i++] =	text;
	}

	clist = lookup_widget (app,"xafiles_clist");
	
	for (j=0; j<clist->rows; j++)
	{
		gchar *text;
		gtk_clist_get_text (clist,j,0,&text);
		call[i++] =	"-xa-files";
		call[i++] =	text;
	}

	// finish command line creation

	call[i++] =	"-o";
	call[i++] =	g_strdup_printf ("%s/%s",getparam ("output_path",gtk_dir_browser_get_path),getparam ("isoname",gtk_entry_get_text));
	call[i++] =	getparam ("input_path",gtk_dir_browser_get_path);
	call[i++] = NULL;

//	for (j=0; j<(i-1); j++)
//	{
//		g_print ("%s ",call[j]);
//	}
//	g_print ("\n");

	if (getparam ("use_systemcnf",gtk_toggle_button_get_active))
	{
		gchar *filename = g_strdup_printf ("%s/system.cnf",getparam ("output_path",gtk_dir_browser_get_path));
		FILE *f;
		
		f = fopen(filename,"w");
		if (f)
		{
			fprintf(f,"BOOT=%s",getparam ("boot",gtk_entry_get_text));
			fputc(13,f);fputc(10,f);
			fprintf(f,"TCB=%s",getparam ("tcb",gtk_entry_get_text));
			fputc(13,f);fputc(10,f);
			fprintf(f,"EVENT=%s",getparam ("event",gtk_entry_get_text));
			fputc(13,f);fputc(10,f);
			fprintf(f,"STACK=%s",getparam ("stack",gtk_entry_get_text));
			fputc(13,f);fputc(10,f);
			fclose(f);
		}
		else perror(filename);
	}

	return call;
}

gchar **make_call_for_write (void)
{
	gchar **call;
	GtkCList *clist;
	gint i,j;
	gchar *s;

	call = g_new (gchar*,40);
	i=0;

	call[i++] = gtk_entry_get_text (lookup_widget (app,"cdrdao"));

	call[i++] = getparam ("simulate",gtk_toggle_button_get_active) ?
		"simulate" : "write";

	s = getparam ("device",gtk_entry_get_text);
	if (strlen(s))
	{
		call[i++] = "--device"; call[i++] = s;
	}

	s = getparam ("driver",gtk_entry_get_text);
	if (strlen(s))
	{
		call[i++] = "--driver"; call[i++] = s;
	}

	j = getparam ("speed",gtk_spin_button_get_value_as_int);
	if (j)
	{
		call[i++] = "--speed"; call[i++] = g_strdup_printf("%d",j);
	}

	j = getparam ("buffers",gtk_spin_button_get_value_as_int);
	if (j)
	{
		call[i++] = "--buffers"; call[i++] = g_strdup_printf("%d",j);
	}

	if (getparam ("swabaudio",gtk_toggle_button_get_active))
	{
		call[i++] = "--swap";
	}

	if (getparam ("reloadtray",gtk_toggle_button_get_active))
	{
		call[i++] = "--reload";
	}

	if (getparam ("eject",gtk_toggle_button_get_active))
	{
		call[i++] = "--eject";
	}

	if (getparam ("force",gtk_toggle_button_get_active))
	{
		call[i++] = "--force";
	}

	if (getparam ("tenseconds",gtk_toggle_button_get_active))
	{
		call[i++] = "-n";
	}

	{
		FILE *toc;
		GtkWidget *w = lookup_widget (app,"toc");
		gchar *name = g_strdup_printf ("%s/%s",getparam ("output_path",gtk_dir_browser_get_path),getparam ("tocname",gtk_entry_get_text));
		gchar *file = g_strdup_printf ("%s/%s",getparam ("output_path",gtk_dir_browser_get_path),getparam ("isoname",gtk_entry_get_text));

		toc = fopen(name,"w");
		if (toc)
		{
			GtkCList *clist;
			gint j;
		
			fprintf (toc,"// automatically created by psxcdmaker\n");
			fprintf (toc,"// Copyright (C) 2000 by Daniel Balster <dbalster@psxdev.de>\n\n");
			fprintf (toc,"CD_ROM_XA\n\n\n");
			fprintf (toc,"// TRACK 1\n");
			fprintf (toc,"TRACK MODE2_FORM_MIX\n");
			fprintf (toc,"NO COPY\n");
			fprintf (toc,"DATAFILE \"%s\" 0\n",file);
			
			clist = lookup_widget(app,"audio");
			
			for (j=0; j<clist->rows; j++)
			{
				gchar *text;
				
				fprintf (toc,"\n// TRACK %d\n",i+2);

				fprintf (toc,"TRACK AUDIO\n");
				gtk_clist_get_text (clist,j,3,&text);
				fprintf (toc,(strcmp(text,"on")==0)?"PRE_EMPHASIS":"NO PRE_EMPHASIS" "\n");

				gtk_clist_get_text (clist,j,2,&text);
				fprintf (toc,(strcmp(text,"on")==0)?"COPY":"NO COPY" "\n");

				fprintf (toc,"// four channels not supported yet\n");
				fprintf (toc,"TWO_CHANNEL_AUDIO\n");

				gtk_clist_get_text (clist,j,0,&text);
				fprintf (toc,"FILE \"%s\" 0\n",text);

				fprintf (toc,"START 00:02:00\n");
			}
			
			fclose (toc);

			call[i++] =	g_strdup(name);
		}

		g_free (file);
		g_free (name);		
	}

	call[i++] = NULL;

//	getparam ("onthefly",gtk_toggle_button_get_active);

	return call;
}

void
on_master_clicked                      (GtkButton       *button,
                                        gpointer         user_data)
{
	gchar **cmd = NULL;

	chdir (currentdir);
	gtk_widget_set_sensitive (lookup_widget(app,"write"),FALSE);
	gtk_widget_set_sensitive (lookup_widget(app,"master"),FALSE);

	gtk_notebook_set_page (lookup_widget(app,"notebook"),6);

	cmd = make_call_for_master();

	switch (zvt_term_forkpty(ZVT_TERM (term), ZVT_TERM_DO_UTMP_LOG |  ZVT_TERM_DO_WTMP_LOG))
	{
		case -1:
			perror("ERROR: unable to fork:");
			exit(1);
			break;
		case 0:
			execvp (cmd[0], cmd);
			perror ("Could not exec\n");
			_exit (127);
		default:
			break;
	}
}


static void
child_died_event (ZvtTerm *term)
{
//	g_print ("child died!\n");
	gtk_widget_set_sensitive (lookup_widget(app,"write"),TRUE);
	gtk_widget_set_sensitive (lookup_widget(app,"master"),TRUE);
}

void
on_write_clicked                       (GtkButton       *button,
                                        gpointer         user_data)
{
	gchar **cmd = NULL;

	chdir (currentdir);
	gtk_widget_set_sensitive (lookup_widget(app,"write"),FALSE);
	gtk_widget_set_sensitive (lookup_widget(app,"master"),FALSE);

	gtk_notebook_set_page (lookup_widget(app,"notebook"),6);

	cmd = make_call_for_write();

	switch (zvt_term_forkpty(ZVT_TERM (term), ZVT_TERM_DO_UTMP_LOG |  ZVT_TERM_DO_WTMP_LOG))
	{
		case -1:
			perror("ERROR: unable to fork:");
			exit(1);
			break;
		case 0:
			execvp (cmd[0], cmd);
			perror ("Could not exec\n");
			_exit (127);
		default:
			break;
	}
}

void
on_button_reset_clicked                (GtkButton       *button,
                                        gpointer         user_data)
{
	zvt_term_reset (ZVT_TERM (term),1);
}


void
on_button_kill_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
	zvt_term_killchild (ZVT_TERM (term),SIGINT);
	zvt_term_killchild (ZVT_TERM (term),SIGKILL);
	zvt_term_closepty (ZVT_TERM (term));
}


GtkWidget*
create_shell (gchar *widget_name, gchar *string1, gchar *string2,
                gint int1, gint int2)
{
	GtkWidget *hbox;
	GtkWidget *scrollbar;

	hbox = gtk_hbox_new (FALSE, 0);
	gtk_box_set_spacing (GTK_BOX (hbox), 2);
	gtk_container_set_border_width (GTK_CONTAINER (hbox), 2);
	gtk_widget_show (hbox);

	#define FONT "-misc-fixed-medium-r-normal--12-200-75-75-c-100-iso8859-1"	
	term = zvt_term_new();

	zvt_term_set_font_name(ZVT_TERM (term), FONT);
	zvt_term_set_blink (ZVT_TERM (term), TRUE);
	zvt_term_set_bell (ZVT_TERM (term), TRUE);
	zvt_term_set_scrollback(ZVT_TERM (term), 10000);
	zvt_term_set_scroll_on_keystroke (ZVT_TERM (term), TRUE);
	zvt_term_set_scroll_on_output (ZVT_TERM (term), FALSE);
	zvt_term_set_background (ZVT_TERM (term), NULL, 0, 0);
	zvt_term_set_wordclass (ZVT_TERM (term), "-A-Za-z0-9/_:.,?+%=");

	gtk_signal_connect (GTK_OBJECT (term), "child_died",
		GTK_SIGNAL_FUNC (child_died_event), NULL);

	gtk_widget_show (term);

	scrollbar = 
		gtk_vscrollbar_new (GTK_ADJUSTMENT (ZVT_TERM (term)->adjustment));
	GTK_WIDGET_UNSET_FLAGS (scrollbar, GTK_CAN_FOCUS);
	gtk_box_pack_start (GTK_BOX (hbox), term, 1, 1, 0);
	gtk_box_pack_start (GTK_BOX (hbox), scrollbar, FALSE, TRUE, 0);
	gtk_widget_show (scrollbar);

	return hbox;
}

/***********************************************************************/

// audio editor

void
on_audio_clist_select_row              (GtkCList        *clist,
                                        gint             row,
                                        gint             column,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
	gchar *text;

	gtk_clist_get_text (clist,row,2,&text);
	gtk_toggle_button_set_active (lookup_widget(app,"copypermission"),
		strcmp(text,"on")==0);

	gtk_clist_get_text (clist,row,3,&text);
	gtk_toggle_button_set_active (lookup_widget(app,"preemphasis"),
		strcmp(text,"on")==0);
}


void
on_audio_add_clicked                   (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget *widget = lookup_widget(app,"audio_file");
	GtkWidget *clist = lookup_widget(app,"audio");
	gchar *label[7];
	
	label[0] = gtk_entry_get_text (widget);
	label[1] = 0;
	label[2] = "off";
	label[3] = "off";
	label[4] = 0;
	
	if (strlen(label[0]))
	{
		gtk_clist_append (clist,label);
		gtk_entry_set_text (lookup_widget(app,"audio_file"),"");
	}

	on_audio_refresh_clicked (0,0);
}


void
on_audio_remove_clicked                (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkCList *clist = lookup_widget(app,"audio");
	gint row;

	row = GPOINTER_TO_INT(clist->selection->data);
	gtk_clist_remove (clist,row);
	gtk_entry_set_text (lookup_widget(app,"audio_file"),"");
	
	on_audio_refresh_clicked (0,0);
}


void
on_audio_refresh_clicked               (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkCList *clist;
	gint i;
	glong total = 0;
	clist = lookup_widget (app,"audio");
	
	for (i=0; i<clist->rows; i++)
	{
		gchar *text;
		struct stat st;
		gtk_clist_get_text (clist,i,0,&text);
		
		if (-1 != stat (text,&st))
		{
			gchar buf[1024];
			gchar *cmd;
			FILE *process;
			
			cmd = g_strdup_printf ("file -b %s",text);
			
			process = popen(cmd,"r");
			if (process)
			{
				if (fgets (buf,1024,process))
				{
					gtk_clist_set_text (clist,i,4,buf);
				}
				pclose (process);
			}
			g_free (cmd);
			
			g_snprintf (buf,1024,"%d",st.st_size);
			gtk_clist_set_text (clist,i,1,buf);
		}
	}
}


void
on_audio_copyperm_toggled              (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	gint row;
	GtkCList *clist = lookup_widget(app,"audio");
	gint toggle = gtk_toggle_button_get_active (togglebutton);

	if (clist->selection)
	{
		row = GPOINTER_TO_INT(clist->selection->data);
		gtk_clist_set_text (clist,row,2,toggle?"on":"off");
	}
}


void
on_audio_preemp_toggled                (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	gint row;
	GtkCList *clist = lookup_widget(app,"audio");
	gint toggle = gtk_toggle_button_get_active (togglebutton);

	if (clist->selection)
	{
		row = GPOINTER_TO_INT(clist->selection->data);
		gtk_clist_set_text (clist,row,3,toggle?"on":"off");
	}
}
