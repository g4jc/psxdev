#include <gnome.h>


void
on_open1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_exit1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_preferences1_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_create_new_window1_activate         (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_close_this_window1_activate         (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_about1_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_new_game1_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_pause_game1_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_restart_game1_activate              (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_end_game1_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_Q_uchar_changed                     (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_Q_char_changed                      (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_Q_ushort_changed                    (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_Q_short_changed                     (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_Q_long_changed                      (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_Q_ulong_changed                     (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_Q_svector_changed                   (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_Q_vector_changed                    (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_Q_matrix_changed                    (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_Q_colorwheel_color_changed          (GtkColorSelection *colorselection,
                                        gpointer         user_data);

void
on_B_char_clicked                      (GtkButton       *button,
                                        gpointer         user_data);

void
on_B_uchar_clicked                     (GtkButton       *button,
                                        gpointer         user_data);

void
on_B_short_clicked                     (GtkButton       *button,
                                        gpointer         user_data);

void
on_B_ushort_clicked                    (GtkButton       *button,
                                        gpointer         user_data);

void
on_B_long_clicked                      (GtkButton       *button,
                                        gpointer         user_data);

void
on_B_ulong_clicked                     (GtkButton       *button,
                                        gpointer         user_data);

void
on_B_vector_clicked                    (GtkButton       *button,
                                        gpointer         user_data);

void
on_clist_select_row                    (GtkCList        *clist,
                                        gint             row,
                                        gint             column,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_clist_unselect_row                  (GtkCList        *clist,
                                        gint             row,
                                        gint             column,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_clist_unselect_all                  (GtkCList        *clist,
                                        gpointer         user_data);

void
on_Q_uchar_reload_clicked              (GtkButton       *button,
                                        gpointer         user_data);

void
on_Q_char_reload_clicked               (GtkButton       *button,
                                        gpointer         user_data);

void
on_Q_ushort_reload_clicked             (GtkButton       *button,
                                        gpointer         user_data);

void
on_Q_short_reload_clicked              (GtkButton       *button,
                                        gpointer         user_data);

void
on_Q_long_reload_clicked               (GtkButton       *button,
                                        gpointer         user_data);

void
on_Q_ulong_reload_clicked              (GtkButton       *button,
                                        gpointer         user_data);

void
on_Q_svector_reload_clicked            (GtkButton       *button,
                                        gpointer         user_data);

void
on_Q_vector_reload_clicked             (GtkButton       *button,
                                        gpointer         user_data);

void
on_Q_matrix_reload_clicked             (GtkButton       *button,
                                        gpointer         user_data);

void
on_resume_clicked                      (GtkButton       *button,
                                        gpointer         user_data);

void
on_halt_clicked                        (GtkButton       *button,
                                        gpointer         user_data);

void
on_reset_clicked                       (GtkButton       *button,
                                        gpointer         user_data);

void
on_Q_toggle_toggled                    (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_B_toggle_clicked                    (GtkButton       *button,
                                        gpointer         user_data);

void
on_B_svector_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

void
on_B_matrix_clicked                    (GtkButton       *button,
                                        gpointer         user_data);

void
on_B_cvector_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

void
on_Q_cvector_reload_clicked            (GtkButton       *button,
                                        gpointer         user_data);

void
on_open_clicked                        (GtkButton       *button,
                                        gpointer         user_data);

void
on_Q_toggle_reload_clicked             (GtkButton       *button,
                                        gpointer         user_data);

void
on_open2_clicked                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_Q_rect_reload_clicked               (GtkButton       *button,
                                        gpointer         user_data);

void
on_Q_rect_changed                      (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_B_rect_clicked                      (GtkButton       *button,
                                        gpointer         user_data);
