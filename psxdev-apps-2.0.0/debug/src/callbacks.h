#include <gnome.h>


gboolean
on_app_delete_event                    (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_exit1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_about1_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_button_reset_clicked                (GtkButton       *button,
                                        gpointer         user_data);

void
on_button_resume_clicked               (GtkButton       *button,
                                        gpointer         user_data);

void
on_button_halt_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_button_step_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

GtkWidget*
create_pc (gchar *widget_name, gchar *string1, gchar *string2,
                gint int1, gint int2);

GtkWidget*
create_registers (gchar *widget_name, gchar *string1, gchar *string2,
                gint int1, gint int2);

GtkWidget*
create_disassembly (gchar *widget_name, gchar *string1, gchar *string2,
                gint int1, gint int2);

GtkWidget*
create_search_from (gchar *widget_name, gchar *string1, gchar *string2,
                gint int1, gint int2);

GtkWidget*
create_search_to (gchar *widget_name, gchar *string1, gchar *string2,
                gint int1, gint int2);

void
on_button_search_clicked               (GtkButton       *button,
                                        gpointer         user_data);

void
on_button_stop_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_button_exit_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_log_clear_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

void
on_log_status_clicked                  (GtkButton       *button,
                                        gpointer         user_data);

void
on_log_versions_clicked                (GtkButton       *button,
                                        gpointer         user_data);

void
on_search_clist_select_row             (GtkCList        *clist,
                                        gint             row,
                                        gint             column,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_search_entry_activate               (GtkEditable     *editable,
                                        gpointer         user_data);

GtkWidget*
create_edit_addr (gchar *widget_name, gchar *string1, gchar *string2,
                gint int1, gint int2);

GtkWidget*
create_edit_value (gchar *widget_name, gchar *string1, gchar *string2,
                gint int1, gint int2);

void
on_load_memory_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

GtkWidget*
create_findhex (gchar *widget_name, gchar *string1, gchar *string2,
                gint int1, gint int2);

void
on_toggle_server_toggled               (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_usemask_toggled                     (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

GtkWidget*
create_mask (gchar *widget_name, gchar *string1, gchar *string2,
                gint int1, gint int2);

GtkWidget*
create_hexentry (gchar *widget_name, gchar *string1, gchar *string2,
                gint int1, gint int2);

GtkWidget*
create_hexentry (gchar *widget_name, gchar *string1, gchar *string2,
                gint int1, gint int2);

GtkWidget*
create_hexentry (gchar *widget_name, gchar *string1, gchar *string2,
                gint int1, gint int2);

GtkWidget*
create_hexentry (gchar *widget_name, gchar *string1, gchar *string2,
                gint int1, gint int2);

GtkWidget*
create_hexentry (gchar *widget_name, gchar *string1, gchar *string2,
                gint int1, gint int2);

void
on_bkpt_clist_select_row               (GtkCList        *clist,
                                        gint             row,
                                        gint             column,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_bkpt_add_clicked                    (GtkButton       *button,
                                        gpointer         user_data);

void
on_bkpt_remove_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_bkpt_enable_toggled                 (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_bkpt_hardware_toggled               (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_bkpt_clist_unselect_row             (GtkCList        *clist,
                                        gint             row,
                                        gint             column,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_bookmark_clist_select_row           (GtkCList        *clist,
                                        gint             row,
                                        gint             column,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_entry_comment_changed               (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_bookmark_add_clicked                (GtkButton       *button,
                                        gpointer         user_data);

void
on_bookmark_remove_clicked             (GtkButton       *button,
                                        gpointer         user_data);

void
on_bookmark_save_clicked               (GtkButton       *button,
                                        gpointer         user_data);

void
on_bookmark_load_clicked               (GtkButton       *button,
                                        gpointer         user_data);

void
on_bookmark_clist_unselect_row         (GtkCList        *clist,
                                        gint             row,
                                        gint             column,
                                        GdkEvent        *event,
                                        gpointer         user_data);

GtkWidget*
create_hexentry (gchar *widget_name, gchar *string1, gchar *string2,
                gint int1, gint int2);

void
on_exe_run_clicked                     (GtkButton       *button,
                                        gpointer         user_data);

void
on_exe_info_clicked                    (GtkButton       *button,
                                        gpointer         user_data);

void
on_exe_save_clicked                    (GtkButton       *button,
                                        gpointer         user_data);

void
on_exe_load_clicked                    (GtkButton       *button,
                                        gpointer         user_data);

void
on_bookmark_clear_clicked              (GtkButton       *button,
                                        gpointer         user_data);

void
on_button_resume_with_server_clicked   (GtkButton       *button,
                                        gpointer         user_data);

void
on_generate_bookmarks1_activate        (GtkMenuItem     *menuitem,
                                        gpointer         user_data);
