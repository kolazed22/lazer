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

static void activate (GtkApplication *app, gpointer user_data)
{
  GtkWidget *window;
  window = gtk_application_window_new (app); // создаём окно
  gtk_window_set_title(GTK_WINDOW(window), "lazer");
  gtk_window_set_default_size(GTK_WINDOW (window), 500, 300);

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
  
  GtkWidget *button_2;
  button_2 = gtk_button_new_with_label ("область");
  g_signal_connect (button_2, "clicked", G_CALLBACK (print_hello), NULL);
  GtkWidget *grid2 = gtk_grid_new();
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), grid2, button_2);

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
