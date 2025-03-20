#include "main.h"
#include "minimarker.c"
#include "plots.c"

// Обработчик сигнала для вывода выбранного элемента в консоль
static void on_dropdown_selection_changed(GtkDropDown *dropdown, GParamSpec *pspec, gpointer user_data) {
    // Получаем индекс выбранного элемента
    guint selected_index = gtk_drop_down_get_selected(dropdown);

    // Получаем модель данных
    GListModel *model = gtk_drop_down_get_model(dropdown);

    // Получаем строку по выбранному индексу
    if (selected_index < g_list_model_get_n_items(model)) {
        GObject *item = g_list_model_get_item(model, selected_index);
        const char *selected_text = gtk_string_object_get_string(GTK_STRING_OBJECT(item));
        // g_print("Selected: %s\n", selected_text);
        Data_TC data_C = fread_TC(g_build_filename(directory_path_materials, g_strconcat(selected_text, "_C.txt", NULL), NULL));
        
        draw_plot_TC(data_C, "plot_C.png");
        freeData_TC(&data_C);
        gtk_image_set_from_file(GTK_IMAGE(image_plot_C), "plot_C.png");

        Data_TC data_a = fread_TC(g_build_filename(directory_path_materials, g_strconcat(selected_text, "_a.txt", NULL), NULL));
        draw_plot_TC(data_a, "plot_a.png");
        freeData_TC(&data_a);
        gtk_image_set_from_file(GTK_IMAGE(image_plot_a), "plot_a.png");

        g_object_unref(item); // Освобождаем ссылку на объект
    }
}

// Функция для получения списка уникальных материалов из каталога
static GtkStringList *get_materials_from_directory(const char *directory_path) {
    DIR *dir;
    struct dirent *entry;
    GtkStringList *materials = gtk_string_list_new(NULL); // Создаем пустой список строк

    // Открываем каталог
    dir = opendir(directory_path);
    if (!dir) {
        g_print("Failed to open directory: %s\n", directory_path);
        return materials;
    }

    // Читаем содержимое каталога
    while ((entry = readdir(dir)) != NULL) {
        const char *filename = entry->d_name;

        // Проверяем, что файл заканчивается на "_C.txt"
        if (strstr(filename, "_C.txt") != NULL) {
            // Извлекаем имя материала (убираем "_C.txt")
            char *material_name = g_strndup(filename, strlen(filename) - strlen("_C.txt"));

            // Преобразуем в UTF-8, если необходимо
            char *utf8_material_name = g_locale_to_utf8(material_name, -1, NULL, NULL, NULL);
            if (!utf8_material_name) {
                g_print("Invalid UTF-8 material name: %s\n", material_name);
                g_free(material_name);
                continue; // Пропускаем некорректные имена
            }

            // Добавляем материал в список
            gtk_string_list_append(materials, utf8_material_name);

            // Освобождаем выделенную память
            g_free(material_name);
            g_free(utf8_material_name);
        }
    }

    closedir(dir);
    return materials;
}

void update_plots(GtkWidget *widget, gpointer data){
    Data_Tt data_Tt = fread_Tt("out_Tt.txt");
    draw_plot_Tt(data_Tt, "plotTt.png");
    freeData_Tt(&data_Tt);

    Data_Txx data_Txy = fread_Txx("out_Txy.txt");
    Data_Txx data_Txy_shape = shape_Txx(&data_Txy);
    draw_heatmap(data_Txy_shape, 300, 300, "heatmapTxy.png");
    freeData_Txx(&data_Txy_shape);

    Data_Txx data_Txz = fread_Txx("out_Txz.txt");
    Data_Txx data_Txz_shape = shape_Txx(&data_Txz);
    draw_heatmap(data_Txz_shape, 300, 300, "heatmapTxz.png");
    freeData_Txx(&data_Txz_shape);

    Data_Txx data_Tyz = fread_Txx("out_Tyz.txt");
    Data_Txx data_Tyz_shape = shape_Txx(&data_Tyz);
    draw_heatmap(data_Tyz_shape, 300, 300, "heatmapTyz.png");
    freeData_Txx(&data_Tyz_shape);

    gtk_image_set_from_file(GTK_IMAGE(image_heatmapTxy), "heatmapTxy.png");
    gtk_image_set_from_file(GTK_IMAGE(image_heatmapTxz), "heatmapTxz.png");
    gtk_image_set_from_file(GTK_IMAGE(image_heatmapTyz), "heatmapTyz.png");
    gtk_image_set_from_file(GTK_IMAGE(image_plotTt)    , "plotTt.png"    );

}

void do_thread(){ // поток программы minimarker
    double P = strtod(gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(entry_P))),NULL); 
    double v = strtod(gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(entry_V))),NULL); 
    double f = strtod(gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(entry_F))),NULL); 
    if (P > 0 && v > 0 && f > 0){
        minimarker(v, f, P, progress_bar);
    }
    else{
        g_print("error value\n");
    }
    g_mutex_unlock (&mut);
}

void click_start_button(GtkWidget *widget, gpointer data) // кнопка старта
{
    if (g_mutex_trylock (&mut)) // проверка, запущен ли поток с программой minimarker
        thread = g_thread_new("minimarker", (GThreadFunc) do_thread,NULL);
    else 
        g_print("Программа уже выполняется\n"); 

}

static void activate(GtkApplication *app, gpointer user_data)
{
    GtkWidget *window;
    window = gtk_application_window_new (app); // создаём окно
    gtk_window_set_title(GTK_WINDOW(window), "lazer");
    gtk_window_set_default_size(GTK_WINDOW (window), 800, 600);

    GtkWidget *notebook = gtk_notebook_new ();
    gtk_window_set_child (GTK_WINDOW (window), notebook);
    
    GtkWidget *h_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *grid = gtk_grid_new();
    gtk_notebook_append_page (GTK_NOTEBOOK (notebook), h_box, gtk_label_new("Параметры лазера"));
    gtk_box_append(GTK_BOX(h_box), grid);

    entry_P = gtk_entry_new();
    gtk_entry_set_input_purpose(GTK_ENTRY(entry_P),  GTK_INPUT_PURPOSE_NUMBER); 
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("мощьность"),1,1,1,1);
    gtk_grid_attach(GTK_GRID(grid), entry_P,2,1,1,1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Вт"),3,1,1,1);

    entry_V = gtk_entry_new();
    gtk_entry_set_input_purpose(GTK_ENTRY(entry_P),  GTK_INPUT_PURPOSE_NUMBER);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("скорость"),1,2,1,1);
    gtk_grid_attach(GTK_GRID(grid), entry_V, 2,2,1,1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("мм/c"),3,2,1,1);

    entry_F = gtk_entry_new();
    gtk_entry_set_input_purpose(GTK_ENTRY(entry_F),  GTK_INPUT_PURPOSE_NUMBER);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("частота"),1,3,1,1);
    gtk_grid_attach(GTK_GRID(grid), entry_F, 2,3,1,1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("кГц"),3,3,1,1);

    progress_bar = gtk_progress_bar_new();
    gtk_grid_attach(GTK_GRID(grid), progress_bar, 1,5,3,1);

    GtkWidget *v_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);
    gtk_box_append(GTK_BOX(h_box), v_box);
    image_plot_a = gtk_image_new_from_file ("plot_a.png");
    gtk_widget_set_size_request(image_plot_a, 200, 200);
    gtk_box_append(GTK_BOX(v_box), image_plot_a);
    image_plot_C = gtk_image_new_from_file ("plot_C.png");
    gtk_widget_set_size_request(image_plot_C, 200, 200);
    gtk_box_append(GTK_BOX(v_box), image_plot_C);

    // Создаем список строк
    directory_path_materials = "materials"; 
    GtkStringList *materials = get_materials_from_directory(directory_path_materials);

    if (g_list_model_get_n_items(G_LIST_MODEL(materials)) == 0) {
        g_print("No materials found in directory: %s\n", directory_path_materials);
        return;
    }
    // Создаем выпадающий список
    GtkWidget *dropdown = gtk_drop_down_new(G_LIST_MODEL(materials), NULL);

    // Добавляем выпадающий список в контейнер
    gtk_box_append(GTK_BOX(v_box), dropdown);

    // Включаем поиск
    gtk_drop_down_set_enable_search(GTK_DROP_DOWN(dropdown), TRUE);

    // Устанавливаем выражение для извлечения текста из элементов модели
    GtkExpression *expression = gtk_property_expression_new(GTK_TYPE_STRING_OBJECT, NULL, "string");
    gtk_drop_down_set_expression(GTK_DROP_DOWN(dropdown), expression);
    gtk_expression_unref(expression);

    on_dropdown_selection_changed(GTK_DROP_DOWN(dropdown), NULL, NULL);

    // Подключаем обработчик сигнала для изменения выбора
    g_signal_connect(dropdown, "notify::selected", G_CALLBACK(on_dropdown_selection_changed), NULL);
    
    GtkWidget *button_2 = gtk_button_new_from_icon_name("media-playback-start");  // кнопка с иконкой, стандартные иконки `gtk4-icon-browser.exe`
    g_signal_connect (button_2, "clicked", G_CALLBACK (click_start_button), NULL);
    gtk_notebook_set_action_widget (GTK_NOTEBOOK(notebook), button_2,  GTK_PACK_END); // добавление виджета в конец notebook

    GtkWidget *grid2 = gtk_grid_new();
    gtk_notebook_append_page (GTK_NOTEBOOK (notebook), grid2, gtk_label_new("область"));

    GtkWidget *entry_Nx1 = gtk_entry_new();
    gtk_entry_set_input_purpose(GTK_ENTRY(entry_Nx1),  GTK_INPUT_PURPOSE_NUMBER);
    gtk_grid_attach(GTK_GRID(grid2), gtk_label_new("Число узлов по оси х локальной сетки"),1,1,1,1);
    gtk_grid_attach(GTK_GRID(grid2), entry_Nx1, 2,1,1,1);
    gtk_grid_attach(GTK_GRID(grid2), gtk_label_new("Узлов"),3,1,1,1);

    GtkWidget *entry_Ny = gtk_entry_new();
    gtk_entry_set_input_purpose(GTK_ENTRY(entry_Ny),  GTK_INPUT_PURPOSE_NUMBER);
    gtk_grid_attach(GTK_GRID(grid2), gtk_label_new("Число узлов по оси у"),1,2,1,1);
    gtk_grid_attach(GTK_GRID(grid2), entry_Ny, 2,2,1,1);
    gtk_grid_attach(GTK_GRID(grid2), gtk_label_new("Узлов"),3,2,1,1);

    GtkWidget *entry_Nz = gtk_entry_new();
    gtk_entry_set_input_purpose(GTK_ENTRY(entry_Nz),  GTK_INPUT_PURPOSE_NUMBER);
    gtk_grid_attach(GTK_GRID(grid2), gtk_label_new("Число узлов по оси z"),1,3,1,1);
    gtk_grid_attach(GTK_GRID(grid2), entry_Nz, 2,3,1,1);
    gtk_grid_attach(GTK_GRID(grid2), gtk_label_new("Узлов"),3,3,1,1);

    GtkWidget *entry_Nx2 = gtk_entry_new();
    gtk_entry_set_input_purpose(GTK_ENTRY(entry_Nx2),  GTK_INPUT_PURPOSE_NUMBER);
    gtk_grid_attach(GTK_GRID(grid2), gtk_label_new("Число узлов по оси х глобальной сетки"),1,4,1,1);
    gtk_grid_attach(GTK_GRID(grid2), entry_Nx2, 2,4,1,1);
    gtk_grid_attach(GTK_GRID(grid2), gtk_label_new("Узлов"),3,4,1,1);

    GtkWidget *entry_Ntr = gtk_entry_new();
    gtk_entry_set_input_purpose(GTK_ENTRY(entry_Ntr),  GTK_INPUT_PURPOSE_NUMBER);
    gtk_grid_attach(GTK_GRID(grid2), gtk_label_new("Количество потоков"),1,5,1,1);
    gtk_grid_attach(GTK_GRID(grid2), entry_Ntr, 2,5,1,1);
    gtk_grid_attach(GTK_GRID(grid2), gtk_label_new("Потоков"),3,5,1,1);
    
    GtkWidget *grid3 = gtk_grid_new();
    gtk_grid_set_column_homogeneous (GTK_GRID(grid3), FALSE);
    gtk_grid_set_row_homogeneous(GTK_GRID(grid3), FALSE);
    gtk_notebook_append_page (GTK_NOTEBOOK (notebook), grid3, gtk_label_new("Графики"));

    GtkWidget *button_plots = gtk_button_new_with_label("Redraw");
    gtk_grid_attach(GTK_GRID(grid3), button_plots, 1, 1, 2, 1);
    g_signal_connect(button_plots, "clicked", G_CALLBACK (update_plots), NULL);

    image_heatmapTxy = gtk_image_new_from_file ("heatmapTxy.png");
    gtk_grid_attach(GTK_GRID(grid3), image_heatmapTxy, 1,2,1,1);
    gtk_widget_set_size_request(image_heatmapTxy, 300, 300);

    image_heatmapTxz = gtk_image_new_from_file ("heatmapTxz.png");
    gtk_grid_attach(GTK_GRID(grid3), image_heatmapTxz, 2,2,1,1);
    gtk_widget_set_size_request(image_heatmapTxz, 300, 300);

    image_heatmapTyz = gtk_image_new_from_file ("heatmapTyz.png");
    gtk_grid_attach(GTK_GRID(grid3), image_heatmapTyz, 1,3,1,1);
    gtk_widget_set_size_request(image_heatmapTyz, 300, 300);

    image_plotTt = gtk_image_new_from_file ("plotTt.png");
    gtk_grid_attach(GTK_GRID(grid3), image_plotTt, 2,3,1,1);
    gtk_widget_set_size_request(image_plotTt, 300, 300);
    
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
