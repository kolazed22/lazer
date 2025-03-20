#include <gtk/gtk.h>
#include <dirent.h>

// Обработчик сигнала для вывода выбранного элемента в консоль
static void on_dropdown_selection_changed(GtkDropDown *dropdown, GParamSpec *pspec, gpointer user_data) {
    guint selected_index = gtk_drop_down_get_selected(dropdown);
    GListModel *model = gtk_drop_down_get_model(dropdown);

    if (selected_index < g_list_model_get_n_items(model)) {
        GObject *item = g_list_model_get_item(model, selected_index);
        const char *selected_text = gtk_string_object_get_string(GTK_STRING_OBJECT(item));
        g_print("Selected material: %s\n", selected_text);
        g_object_unref(item);
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

static void activate(GtkApplication *app, gpointer user_data) {
    // Создаем окно
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Materials DropDown Example");
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 200);

    // Создаем контейнер
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_window_set_child(GTK_WINDOW(window), box);

    // Получаем список материалов из каталога
    const char *directory_path = "materials"; // Путь к каталогу
    GtkStringList *materials = get_materials_from_directory(directory_path);

    if (g_list_model_get_n_items(G_LIST_MODEL(materials)) == 0) {
        g_print("No materials found in directory: %s\n", directory_path);
        return;
    }

    // Создаем выпадающий список
    GtkWidget *dropdown = gtk_drop_down_new(G_LIST_MODEL(materials), NULL);

    // Включаем поиск
    gtk_drop_down_set_enable_search(GTK_DROP_DOWN(dropdown), TRUE);

    // Устанавливаем выражение для извлечения текста из элементов модели
    GtkExpression *expression = gtk_property_expression_new(GTK_TYPE_STRING_OBJECT, NULL, "string");
    gtk_drop_down_set_expression(GTK_DROP_DOWN(dropdown), expression);
    gtk_expression_unref(expression);

    // Добавляем выпадающий список в контейнер
    gtk_box_append(GTK_BOX(box), dropdown);

    // Подключаем обработчик сигнала для изменения выбора
    g_signal_connect(dropdown, "notify::selected", G_CALLBACK(on_dropdown_selection_changed), NULL);

    // Отображаем окно
    gtk_widget_set_visible(window, TRUE);
}

int main(int argc, char **argv) {
    GtkApplication *app = gtk_application_new("org.example.materials.dropdown", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}