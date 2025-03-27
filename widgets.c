#include "main.h"


// Обработчик сигнала для вывода выбранного элемента в консоль
static void on_dropdown_selection_changed(GtkDropDown *dropdown, GParamSpec *pspec, gpointer user_data) {
    // Получаем индекс выбранного элемента
    guint selected_index = gtk_drop_down_get_selected(dropdown);

    // Получаем модель данных
    GListModel *model = gtk_drop_down_get_model(dropdown);
    
    // Получаем строку по выбранному индексу
    if (selected_index < g_list_model_get_n_items(model)) {
        const gchar *selected_text = gtk_string_object_get_string(GTK_STRING_OBJECT(g_list_model_get_item(model, selected_index)));
        // Находим индекс материала в массиве materials
        for (size_t i = 0; i < material_database.size; i++) {
            if (strcmp(material_database.materials[i].name, selected_text) == 0) {
                selected_material = &material_database.materials[i];
                break;
            }
        }
        
        if (selected_material) {
            char *file = g_build_filename(DIRECTORY_PATH_MATERIALS, selected_material->conductivity_image, NULL);
            if (g_file_test(file, G_FILE_TEST_EXISTS)) gtk_image_set_from_file(GTK_IMAGE(image_plot_cond), file);
            else g_print("Error: %s file not found\n", file);
            g_free(file);

            file = g_build_filename(DIRECTORY_PATH_MATERIALS, selected_material->capacity_image, NULL);
            if (g_file_test(file, G_FILE_TEST_EXISTS)) gtk_image_set_from_file(GTK_IMAGE(image_plot_cap), file);
            else g_print("Error: %s file not found\n", file);
            g_free(file);

            file = g_build_filename(DIRECTORY_PATH_MATERIALS, selected_material->absorption_image, NULL);
            if (g_file_test(file, G_FILE_TEST_EXISTS)) gtk_image_set_from_file(GTK_IMAGE(image_plot_abs), file);
            else g_print("Error: %s file not found\n", file);
            g_free(file);

            // g_print("%s\n", g_build_filename(DIRECTORY_PATH_MATERIALS, selected_material->conductivity_data, NULL));
            // Data_TC data_a = fread_TC(g_build_filename(DIRECTORY_PATH_MATERIALS, selected_material->conductivity_data, NULL));
            // draw_plot_TC(data_a, g_build_filename(DIR_TEMP,"plot_cond.png", NULL));
            // freeData_TC(&data_a);
            // Data_TC data_c = fread_TC(g_build_filename(DIRECTORY_PATH_MATERIALS, selected_material->capacity_data, NULL));
            // draw_plot_TC(data_c, g_build_filename(DIR_TEMP,"plot_cap.png", NULL));
            // freeData_TC(&data_c);
            // gtk_image_set_from_file(GTK_IMAGE(image_plot_a), g_build_filename(DIR_TEMP,"plot_a.png", NULL));
        }
    }
}

void update_plots(GtkWidget *widget, gpointer data){
    Data_Tt data_Tt = fread_Tt(g_build_filename(DIR_TEMP,"out_Tt.txt", NULL));
    draw_plot_Tt(data_Tt, g_build_filename(DIR_TEMP,"plotTt.png", NULL));
    freeData_Tt(&data_Tt);

    Data_Txx data_Txy = fread_Txx(g_build_filename(DIR_TEMP,"out_Txy.txt", NULL));
    Data_Txx data_Txy_shape = shape_Txx(&data_Txy);
    draw_heatmap(data_Txy_shape, 300, 300, g_build_filename(DIR_TEMP,"heatmapTxy.png", NULL));
    freeData_Txx(&data_Txy_shape);

    Data_Txx data_Txz = fread_Txx(g_build_filename(DIR_TEMP,"out_Txz.txt", NULL));
    Data_Txx data_Txz_shape = shape_Txx(&data_Txz);
    draw_heatmap(data_Txz_shape, 300, 300, g_build_filename(DIR_TEMP,"heatmapTxz.png", NULL));
    freeData_Txx(&data_Txz_shape);

    Data_Txx data_Tyz = fread_Txx(g_build_filename(DIR_TEMP,"out_Tyz.txt", NULL));
    Data_Txx data_Tyz_shape = shape_Txx(&data_Tyz);
    draw_heatmap(data_Tyz_shape, 300, 300, g_build_filename(DIR_TEMP,"heatmapTyz.png", NULL));
    freeData_Txx(&data_Tyz_shape);

    gtk_image_set_from_file(GTK_IMAGE(image_heatmapTxy), g_build_filename(DIR_TEMP,"heatmapTxy.png", NULL));
    gtk_image_set_from_file(GTK_IMAGE(image_heatmapTxz), g_build_filename(DIR_TEMP,"heatmapTxz.png", NULL));
    gtk_image_set_from_file(GTK_IMAGE(image_heatmapTyz), g_build_filename(DIR_TEMP,"heatmapTyz.png", NULL));
    gtk_image_set_from_file(GTK_IMAGE(image_plotTt)    , g_build_filename(DIR_TEMP,"plotTt.png", NULL)    );

}

// Проверка правильности ввода данных
bool check_input_parameters(LaserParams params){
    if (params.P <= 0 || params.v <= 0 || params.f <= 0 || params.tp <= 0 || params.r0 <= 0){
        return false;
    }
    return true;
}

void do_thread(){ // поток программы minimarker
    LaserParams params;
    params.P = strtod(gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(entry_P))),NULL); 
    params.v = strtod(gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(entry_V))),NULL); 
    params.f = strtod(gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(entry_F))),NULL); 
    params.tp = strtod(gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(entry_tp))),NULL); 
    params.r0 = strtod(gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(entry_r0))),NULL); 
    if (check_input_parameters(params)){
        minimarker(params, selected_material, progress_bar);
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

// Инициализация всех виджетов
void init_widgets(GtkApplication *app) {
    // Создаем основное окно
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "lazer");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

    // Создаем notebook (вкладки)
    notebook = gtk_notebook_new();
    gtk_window_set_child(GTK_WINDOW(window), notebook);
    
    // Инициализируем вкладки
    init_parameters_tab(notebook);
    init_area_tab(notebook);
    init_plots_tab(notebook);
    
    // Кнопка запуска
    GtkWidget *button_2 = gtk_button_new_from_icon_name("media-playback-start");
    g_signal_connect(button_2, "clicked", G_CALLBACK(click_start_button), NULL);
    gtk_notebook_set_action_widget(GTK_NOTEBOOK(notebook), button_2, GTK_PACK_END);
}

// Инициализация вкладки параметров
void init_parameters_tab(GtkWidget *notebook) {
    GtkWidget *h_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *grid = gtk_grid_new();
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), h_box, gtk_label_new("Параметры лазера"));
    gtk_box_append(GTK_BOX(h_box), grid);

    // Поле ввода мощности
    entry_P = gtk_entry_new();
    gtk_entry_set_input_purpose(GTK_ENTRY(entry_P), GTK_INPUT_PURPOSE_NUMBER); 
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("мощьность"), 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_P, 2, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("Вт"), 3, 1, 1, 1);

    // Поле ввода скорости
    entry_V = gtk_entry_new();
    gtk_entry_set_input_purpose(GTK_ENTRY(entry_V), GTK_INPUT_PURPOSE_NUMBER);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("скорость"), 1, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_V, 2, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("мм/c"), 3, 2, 1, 1);

    // Поле ввода частоты
    entry_F = gtk_entry_new();
    gtk_entry_set_input_purpose(GTK_ENTRY(entry_F), GTK_INPUT_PURPOSE_NUMBER);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("частота"), 1, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_F, 2, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("кГц"), 3, 3, 1, 1);

    // Поле ввода длительности импульса
    entry_tp = gtk_entry_new();
    gtk_entry_set_input_purpose(GTK_ENTRY(entry_tp), GTK_INPUT_PURPOSE_NUMBER);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("длительнось импульса"), 1, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_tp, 2, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("нс"), 3, 4, 1, 1);

    // Поле ввода радиуса пучка по уровню e^-1
    entry_r0 = gtk_entry_new();
    gtk_entry_set_input_purpose(GTK_ENTRY(entry_r0), GTK_INPUT_PURPOSE_NUMBER);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("радиус пучка по уровню e^-1"), 1, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_r0, 2, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("мкм"), 3, 5, 1, 1);

    // Прогресс-бар
    progress_bar = gtk_progress_bar_new();
    gtk_grid_attach(GTK_GRID(grid), progress_bar, 1, 6, 3, 1);

    // Вертикальный бокс для графиков и выпадающего списка
    GtkWidget *v_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);
    gtk_box_append(GTK_BOX(h_box), v_box);
    
    // Графики
    // теплопроводность
    image_plot_cond = gtk_image_new_from_file(NULL);
    gtk_widget_set_size_request(image_plot_cond, 200, 200);
    gtk_box_append(GTK_BOX(v_box), image_plot_cond);
    // теплоемкость
    image_plot_cap = gtk_image_new_from_file(NULL);
    gtk_widget_set_size_request(image_plot_cap, 200, 200);
    gtk_box_append(GTK_BOX(v_box), image_plot_cap);
    // коэффициент поглощения
    image_plot_abs = gtk_image_new_from_file(NULL);
    gtk_widget_set_size_request(image_plot_abs, 200, 200);
    gtk_box_append(GTK_BOX(v_box), image_plot_abs);

    // Выпадающий список материалов
    material_database = load_material_database(DIRECTORY_PATH_MATERIALS);

    if (g_list_model_get_n_items(G_LIST_MODEL(material_database.material_names)) == 0) {
        g_critical("No materials found in directory: %s\n", DIRECTORY_PATH_MATERIALS);
        return;
    }
    
    dropdown = gtk_drop_down_new(G_LIST_MODEL(material_database.material_names), NULL);
    gtk_box_append(GTK_BOX(v_box), dropdown);
    gtk_drop_down_set_enable_search(GTK_DROP_DOWN(dropdown), TRUE);

    // Настройка выражения для извлечения текста
    GtkExpression *expression = gtk_property_expression_new(GTK_TYPE_STRING_OBJECT, NULL, "string");
    gtk_drop_down_set_expression(GTK_DROP_DOWN(dropdown), expression);
    gtk_expression_unref(expression);

    // Подключение обработчика сигнала
    g_signal_connect(dropdown, "notify::selected", G_CALLBACK(on_dropdown_selection_changed), NULL);
    on_dropdown_selection_changed(GTK_DROP_DOWN(dropdown), NULL, NULL);
}// Инициализация вкладки области
void init_area_tab(GtkWidget *notebook) {
    GtkWidget *grid2 = gtk_grid_new();
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), grid2, gtk_label_new("область"));

    // Поля ввода для параметров сетки
    entry_Nx1 = gtk_entry_new();
    gtk_entry_set_input_purpose(GTK_ENTRY(entry_Nx1), GTK_INPUT_PURPOSE_NUMBER);
    gtk_grid_attach(GTK_GRID(grid2), gtk_label_new("Число узлов по оси x локальной сетки"), 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid2), entry_Nx1, 2, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid2), gtk_label_new("Узлов"), 3, 1, 1, 1);

    entry_Ny = gtk_entry_new();
    gtk_entry_set_input_purpose(GTK_ENTRY(entry_Ny), GTK_INPUT_PURPOSE_NUMBER);
    gtk_grid_attach(GTK_GRID(grid2), gtk_label_new("Число узлов по оси y"), 1, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid2), entry_Ny, 2, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid2), gtk_label_new("Узлов"), 3, 2, 1, 1);

    entry_Nz = gtk_entry_new();
    gtk_entry_set_input_purpose(GTK_ENTRY(entry_Nz), GTK_INPUT_PURPOSE_NUMBER);
    gtk_grid_attach(GTK_GRID(grid2), gtk_label_new("Число узлов по оси z"), 1, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid2), entry_Nz, 2, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid2), gtk_label_new("Узлов"), 3, 3, 1, 1);

    entry_Nx2 = gtk_entry_new();
    gtk_entry_set_input_purpose(GTK_ENTRY(entry_Nx2), GTK_INPUT_PURPOSE_NUMBER);
    gtk_grid_attach(GTK_GRID(grid2), gtk_label_new("Число узлов по оси x глобальной сетки"), 1, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid2), entry_Nx2, 2, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid2), gtk_label_new("Узлов"), 3, 4, 1, 1);

    entry_Ntr = gtk_entry_new();
    gtk_entry_set_input_purpose(GTK_ENTRY(entry_Ntr), GTK_INPUT_PURPOSE_NUMBER);
    gtk_grid_attach(GTK_GRID(grid2), gtk_label_new("Количество потоков"), 1, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(grid2), entry_Ntr, 2, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(grid2), gtk_label_new("Потоков"), 3, 5, 1, 1);
}

// Инициализация вкладки графиков
void init_plots_tab(GtkWidget *notebook) {
    GtkWidget *grid3 = gtk_grid_new();
    gtk_grid_set_column_homogeneous(GTK_GRID(grid3), FALSE);
    gtk_grid_set_row_homogeneous(GTK_GRID(grid3), FALSE);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), grid3, gtk_label_new("Графики"));

    // Кнопка обновления графиков
    GtkWidget *button_plots = gtk_button_new_with_label("Redraw");
    gtk_grid_attach(GTK_GRID(grid3), button_plots, 1, 1, 2, 1);
    g_signal_connect(button_plots, "clicked", G_CALLBACK(update_plots), NULL);

    // Графики
    image_heatmapTxy = gtk_image_new_from_file(g_build_filename(DIR_TEMP, "heatmapTxy.png", NULL));
    gtk_grid_attach(GTK_GRID(grid3), image_heatmapTxy, 1, 2, 1, 1);
    gtk_widget_set_size_request(image_heatmapTxy, 300, 300);

    image_heatmapTxz = gtk_image_new_from_file(g_build_filename(DIR_TEMP, "heatmapTxz.png", NULL));
    gtk_grid_attach(GTK_GRID(grid3), image_heatmapTxz, 2, 2, 1, 1);
    gtk_widget_set_size_request(image_heatmapTxz, 300, 300);

    image_heatmapTyz = gtk_image_new_from_file(g_build_filename(DIR_TEMP, "heatmapTyz.png", NULL));
    gtk_grid_attach(GTK_GRID(grid3), image_heatmapTyz, 1, 3, 1, 1);
    gtk_widget_set_size_request(image_heatmapTyz, 300, 300);

    image_plotTt = gtk_image_new_from_file(g_build_filename(DIR_TEMP, "plotTt.png", NULL));
    gtk_grid_attach(GTK_GRID(grid3), image_plotTt, 2, 3, 1, 1);
    gtk_widget_set_size_request(image_plotTt, 300, 300);
}
