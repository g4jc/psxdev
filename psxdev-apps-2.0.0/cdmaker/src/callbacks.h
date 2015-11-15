#include <gnome.h>


void
on_exit_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_about_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

GtkWidget*
create_dirbrowser (gchar *widget_name, gchar *string1, gchar *string2,
                gint int1, gint int2);

GtkWidget*
create_dirbrowser (gchar *widget_name, gchar *string1, gchar *string2,
                gint int1, gint int2);

gboolean
on_app1_delete_event                   (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_new_clicked                         (GtkButton       *button,
                                        gpointer         user_data);

void
on_open_clicked                        (GtkButton       *button,
                                        gpointer         user_data);

void
on_save_clicked                        (GtkButton       *button,
                                        gpointer         user_data);

void
on_master_clicked                      (GtkButton       *button,
                                        gpointer         user_data);

void
on_write_clicked                       (GtkButton       *button,
                                        gpointer         user_data);

void
on_exit_clicked                        (GtkButton       *button,
                                        gpointer         user_data);

GtkWidget*
create_dirbrowser (gchar *widget_name, gchar *string1, gchar *string2,
                gint int1, gint int2);

GtkWidget*
create_dirbrowser (gchar *widget_name, gchar *string1, gchar *string2,
                gint int1, gint int2);

void
on_cb_systemcnf_toggled                (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_use_systemcnf_toggled               (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_use_abstract_toggled                (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_use_copyright_toggled               (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_use_bibliography_toggled            (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_use_creation_toggled                (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_use_expiration_toggled              (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_use_modification_toggled            (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_use_effective_toggled               (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_ctree_tree_select_row               (GtkCTree        *ctree,
                                        GList           *node,
                                        gint             column,
                                        gpointer         user_data);

GtkWidget*
create_html (gchar *widget_name, gchar *string1, gchar *string2,
                gint int1, gint int2);

void
on_exclusion_clist_select_row          (GtkCList        *clist,
                                        gint             row,
                                        gint             column,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_exclusion_changed                   (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_xafiles_clist_select_row            (GtkCList        *clist,
                                        gint             row,
                                        gint             column,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_xafiles_changed                     (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_exclusion_add_clicked               (GtkButton       *button,
                                        gpointer         user_data);

void
on_exclusion_remove_clicked            (GtkButton       *button,
                                        gpointer         user_data);

void
on_exclusion_defaults_clicked          (GtkButton       *button,
                                        gpointer         user_data);

void
on_xafiles_add_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_xafiles_remove_clicked              (GtkButton       *button,
                                        gpointer         user_data);

void
on_xafiles_defaults_clicked            (GtkButton       *button,
                                        gpointer         user_data);

void
on_getsize_clicked                     (GtkButton       *button,
                                        gpointer         user_data);

void
on_output_path_changed (GtkWidget *widget, gchar *path);

void
on_input_path_changed (GtkWidget *widget, gchar *path);

void
on_save_clicked                        (GtkButton       *button,
                                        gpointer         user_data);

GtkWidget*
create_zvt (gchar *widget_name, gchar *string1, gchar *string2,
                gint int1, gint int2);

void
on_button_reset_clicked                (GtkButton       *button,
                                        gpointer         user_data);

void
on_button_kill_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

GtkWidget*
create_shell (gchar *widget_name, gchar *string1, gchar *string2,
                gint int1, gint int2);

void
on_audio_clist_select_row              (GtkCList        *clist,
                                        gint             row,
                                        gint             column,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_audio_add_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

void
on_audio_remove_clicked                (GtkButton       *button,
                                        gpointer         user_data);

void
on_audio_refresh_clicked               (GtkButton       *button,
                                        gpointer         user_data);

void
on_audio_copyperm_toggled              (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_audio_preemp_toggled                (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_audio_clist_select_row              (GtkCList        *clist,
                                        gint             row,
                                        gint             column,
                                        GdkEvent        *event,
                                        gpointer         user_data);
