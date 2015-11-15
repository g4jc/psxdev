#include <gnome.h>


gboolean
on_window1_delete_event                (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_add_clicked                         (GtkButton       *button,
                                        gpointer         user_data);

void
on_clear_clicked                       (GtkButton       *button,
                                        gpointer         user_data);

void
on_quit_clicked                        (GtkButton       *button,
                                        gpointer         user_data);

GtkWidget*
create_filebrowser (gchar *widget_name, gchar *string1, gchar *string2,
                gint int1, gint int2);
				
void on_manpage_path_changed (GtkWidget *widget, gchar *path, gpointer data);
void on_manpage_clicked (GtkWidget *widget, gchar *path, gpointer data);
void on_path_changed (GtkWidget *widget, gchar *path, gpointer data);

void
on_manpage_add_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
;
void
on_manpage_sync_clicked                (GtkButton       *button,
                                        gpointer         user_data)
;
void
on_manpage_delete_clicked              (GtkButton       *button,
                                        gpointer         user_data)
;
void
on_manpage_clear_clicked                (GtkButton       *button,
                                        gpointer         user_data)
;

void
on_text_changed                        (GtkEditable     *editable,
                                        gpointer         user_data);

GtkWidget*
create_dirbrowser (gchar *widget_name, gchar *string1, gchar *string2,
                gint int1, gint int2);

GtkWidget*
create_filebrowser (gchar *widget_name, gchar *string1, gchar *string2,
                gint int1, gint int2);

void
on_news_delete_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_news_sync_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

void on_news_clicked (GtkWidget *widget, gchar *path, gpointer data);
void on_news_path_changed (GtkWidget *widget, gchar *path, gpointer data);

void
on_button_get_time_and_date_clicked    (GtkButton       *button,
                                        gpointer         user_data);

GtkWidget*
create_filebrowser (gchar *widget_name, gchar *string1, gchar *string2,
                gint int1, gint int2);

void
on_faq_add_clicked                     (GtkButton       *button,
                                        gpointer         user_data);

void
on_faq_clear_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

void
on_faq_delete_clicked                  (GtkButton       *button,
                                        gpointer         user_data);

void
on_faq_sync_clicked                    (GtkButton       *button,
                                        gpointer         user_data);

void on_faq_clicked (GtkWidget *widget, gchar *path, gpointer data);

GtkWidget*
create_filebrowser (gchar *widget_name, gchar *string1, gchar *string2,
                gint int1, gint int2);

void
on_screenshot_add_clicked              (GtkButton       *button,
                                        gpointer         user_data);

void
on_screenshot_clear_clicked            (GtkButton       *button,
                                        gpointer         user_data);

void
on_screenshot_delete_clicked           (GtkButton       *button,
                                        gpointer         user_data);

void
on_screenshot_sync_clicked             (GtkButton       *button,
                                        gpointer         user_data);

void on_screenshot_clicked (GtkWidget *widget, gchar *path, gpointer data);
