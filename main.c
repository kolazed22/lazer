#include <gtk/gtk.h>
static void print_hello (GtkWidget *widget, gpointer   data)
{
  g_print ("Hello World\n");
}
static void print_entry(GtkEntry *entry, gpointer   data)
{
  g_print (gtk_entry_buffer_get_text (gtk_entry_get_buffer(entry)));
  g_print("\n");
}
static void draw_function(GtkDrawingArea *area, cairo_t *cr, int width, int height, gpointer data)
{
  GdkRGBA color;

  cairo_pattern_t *r1; 
    
  cairo_set_source_rgba(cr, 0, 0, 0, 1);
  // cairo_set_line_width(cr, 12);  
  // cairo_translate(cr, 60, 60);
  
  r1 = cairo_pattern_create_radial(0, 100, 200, 150, 100, 70);
  cairo_pattern_add_color_stop_rgba(r1, 0.9, 0.1, 0.1, 1, 1);
  cairo_pattern_add_color_stop_rgba(r1, 1, 1, 1, 1, 1);
  cairo_set_source(cr, r1);
  cairo_arc(cr,100, 100, 100, 0, G_PI * 2);
  cairo_fill(cr);
         
  cairo_pattern_destroy(r1);
}

static void activate (GtkApplication *app, gpointer user_data)
{
  GtkWidget *window;
  window = gtk_application_window_new (app); // создаём окно
  gtk_window_set_title(GTK_WINDOW(window), "lazer");
  gtk_window_set_default_size(GTK_WINDOW (window), 800, 600);

  GtkWidget *notebook = gtk_notebook_new ();
  gtk_window_set_child (GTK_WINDOW (window), notebook);
  
  GtkWidget *grid = gtk_grid_new();
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), grid, gtk_label_new("Параметры лазера"));

  GtkWidget *entry_P = gtk_entry_new();
  gtk_entry_set_input_purpose(GTK_ENTRY(entry_P),  GTK_INPUT_PURPOSE_NUMBER);
  g_signal_connect(entry_P, "activate", G_CALLBACK(print_entry), NULL);
  gtk_grid_attach(GTK_GRID(grid), gtk_label_new("мощьность"),1,1,1,1);
  gtk_grid_attach(GTK_GRID(grid), entry_P,2,1,1,1);
  gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Вт"),3,1,1,1);

  GtkWidget *entry_V = gtk_entry_new();
  gtk_entry_set_input_purpose(GTK_ENTRY(entry_P),  GTK_INPUT_PURPOSE_NUMBER);
  g_signal_connect(entry_V, "activate", G_CALLBACK(print_entry), NULL);
  gtk_grid_attach(GTK_GRID(grid), gtk_label_new("скорость"),1,2,1,1);
  gtk_grid_attach(GTK_GRID(grid), entry_V, 2,2,1,1);
  gtk_grid_attach(GTK_GRID(grid), gtk_label_new("мм/c"),3,2,1,1);

  GtkWidget *entry_F = gtk_entry_new();
  gtk_entry_set_input_purpose(GTK_ENTRY(entry_F),  GTK_INPUT_PURPOSE_NUMBER);
  g_signal_connect(entry_F, "activate", G_CALLBACK(print_entry), NULL);
  gtk_grid_attach(GTK_GRID(grid), gtk_label_new("частота"),1,3,1,1);
  gtk_grid_attach(GTK_GRID(grid), entry_F, 2,3,1,1);
  gtk_grid_attach(GTK_GRID(grid), gtk_label_new("кГц"),3,3,1,1);
  
  
  GtkWidget *grid2 = gtk_grid_new();
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), grid2, gtk_label_new("область"));
  GtkWidget *button_2 = gtk_button_new_from_icon_name("media-playback-start");  // кнопка с иконкой, стандартные иконки `gtk4-icon-browser.exe`
  g_signal_connect (button_2, "clicked", G_CALLBACK (print_hello), NULL);
  gtk_notebook_set_action_widget (GTK_NOTEBOOK(notebook), button_2,  GTK_PACK_END); // добавление виджета в конец notebook

  GtkWidget *area = gtk_drawing_area_new ();
  gtk_drawing_area_set_content_width (GTK_DRAWING_AREA (area), 200);
  gtk_drawing_area_set_content_height (GTK_DRAWING_AREA (area), 200);
  gtk_drawing_area_set_draw_func (GTK_DRAWING_AREA (area), draw_function ,NULL, NULL);
  gtk_grid_attach(GTK_GRID(grid2), area, 1, 1, 1, 1);

  gtk_window_present (GTK_WINDOW (window));
}

int main (int argc, char **argv)
{
  GtkApplication *app;
  int status;

  app = gtk_application_new ("org.gtk.example", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
  status = g_application_run (G_APPLICATION (app), argc, argv);
  g_object_unref (app);

  return status;
}
