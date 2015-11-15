#include <gnome.h>


void
on_exit1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_about1_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_button_new_clicked                  (GtkButton       *button,
                                        gpointer         user_data);

void
on_button_open_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_button_save_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_button_save_as_clicked              (GtkButton       *button,
                                        gpointer         user_data);

void
on_radiobutton_spline_clicked          (GtkButton       *button,
                                        gpointer         user_data);

void
on_radiobutton_linear_clicked          (GtkButton       *button,
                                        gpointer         user_data);

void
on_radiobutton_free_clicked            (GtkButton       *button,
                                        gpointer         user_data);

gboolean
on_app_delete_event                    (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_list_open_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

void
on_list_save_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

void
on_list_add_clicked                    (GtkButton       *button,
                                        gpointer         user_data);

void
on_list_remove_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_list_use_clicked                    (GtkButton       *button,
                                        gpointer         user_data);

void
on_minx_changed                        (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_maxx_changed                        (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_miny_changed                        (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_maxy_changed                        (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_button_upload_clicked               (GtkButton       *button,
                                        gpointer         user_data);
