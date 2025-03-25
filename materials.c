# include "main.h"

// количество json файлов в каталоге
int count_json_files(const char *directory) {
    DIR *dir;
    struct dirent *entry;
    int count = 0;
    
    // Открываем каталог
    dir = opendir(directory);
    if (dir == NULL) {
        return -1; // Ошибка открытия каталога
    }
    
    // Читаем содержимое каталога
    while ((entry = readdir(dir)) != NULL) {
        // Получаем длину имени файла
        size_t name_len = strlen(entry->d_name);
        
        // Проверяем, заканчивается ли файл на .json
        if (name_len > 5 && // Минимальная длина для .json
            strcmp(entry->d_name + name_len - 5, ".json") == 0) {
            count++;
        }
    }
    
    // Закрываем каталог
    closedir(dir);
    return count;
}

// Вспомогательная функция для извлечения значения между кавычками
void extract_string(const char *json, const char *key, char *result, size_t result_size) {
    char search_key[256];
    snprintf(search_key, sizeof(search_key), "\"%s\":\"", key);
    const char *start = strstr(json, search_key);
    if (!start) {
        result[0] = '\0'; // Если ключ не найден, возвращаем пустую строку
        return;
    }

    start += strlen(search_key); // Перемещаемся к началу значения
    const char *end = strchr(start, '"');
    if (!end) {
        result[0] = '\0'; // Если значение некорректно, возвращаем пустую строку
        return;
    }

    size_t length = end - start;
    if (length >= result_size) {
        length = result_size - 1; // Обрезаем, если значение слишком длинное
    }
    strncpy(result, start, length);
    result[length] = '\0';
}

// Вспомогательная функция для извлечения числового значения
void extract_number(const char *json, const char *key, double *result) {
    char search_key[256];
    snprintf(search_key, sizeof(search_key), "\"%s\":", key);

    const char *start = strstr(json, search_key);
    if (!start) {
        *result = 0.0; // Если ключ не найден, возвращаем 0.0
        return;
    }

    start += strlen(search_key); // Перемещаемся к началу значения
    *result = strtod(start, NULL); // Преобразуем строку в число
}

Material Material_from_json(cJSON *root) {
    Material material;
    // extract_string(json, "name", material.name, sizeof(material.name));
    // extract_number(json, "A", &material.A);
    // extract_number(json, "m0", &material.m0);
    // extract_number(json, "Tb", &material.Tb);
    // extract_number(json, "Lev", &material.Lev);
    // extract_number(json, "pol", &material.pol);
    // extract_string(json, "conductivity_image", material.conductivity_image, sizeof(material.conductivity_image));
    // extract_string(json, "capacity_image", material.capacity_image, sizeof(material.capacity_image));
    // extract_string(json, "conductivity_data", material.conductivity_data, sizeof(material.conductivity_data));
    // extract_string(json, "capacity_data", material.capacity_data, sizeof(material.capacity_data));
    // extract_string(json, "absorption_image", material.absorption_image, sizeof(material.absorption_image));
    // extract_string(json, "absorption_data", material.absorption_data, sizeof(material.absorption_data));

    // Извлечение данных из JSON
    cJSON *name = cJSON_GetObjectItemCaseSensitive(root, "name");
    cJSON *A = cJSON_GetObjectItemCaseSensitive(root, "A");
    cJSON *m0 = cJSON_GetObjectItemCaseSensitive(root, "m0");
    cJSON *Tb = cJSON_GetObjectItemCaseSensitive(root, "Tb");
    cJSON *Lev = cJSON_GetObjectItemCaseSensitive(root, "Lev");
    cJSON *pol = cJSON_GetObjectItemCaseSensitive(root, "pol");
    cJSON *conductivity_image = cJSON_GetObjectItemCaseSensitive(root, "conductivity_image");
    cJSON *capacity_image = cJSON_GetObjectItemCaseSensitive(root, "capacity_image");
    cJSON *absorption_image = cJSON_GetObjectItemCaseSensitive(root, "absorption_image");
    cJSON *conductivity_data = cJSON_GetObjectItemCaseSensitive(root, "conductivity_data");
    cJSON *capacity_data = cJSON_GetObjectItemCaseSensitive(root, "capacity_data");
    cJSON *absorption_data = cJSON_GetObjectItemCaseSensitive(root, "absorption_data");    
    
    strncpy(material.name, name->valuestring, sizeof(material.name) - 1);
    material.A = A->valuedouble;
    material.m0 = m0->valuedouble;
    material.Tb = Tb->valuedouble;
    material.Lev = Lev->valuedouble;
    material.pol = pol->valuedouble;
    strncpy(material.conductivity_image, conductivity_image->valuestring, sizeof(material.conductivity_image) - 1);
    strncpy(material.capacity_image, capacity_image->valuestring, sizeof(material.capacity_image) - 1);
    strncpy(material.absorption_image, absorption_image->valuestring, sizeof(material.absorption_image) - 1);
    strncpy(material.conductivity_data, conductivity_data->valuestring, sizeof(material.conductivity_data) - 1);
    strncpy(material.capacity_data, capacity_data->valuestring, sizeof(material.capacity_data) - 1);
    strncpy(material.absorption_data, absorption_data->valuestring, sizeof(material.absorption_data) - 1);
   
    return material;
}
// Функция для получения database материалов из каталога
MaterialDatabase load_material_database(const char *directory_path) {
    DIR *dir;
    struct dirent *entry;
    MaterialDatabase database;
    database.size = count_json_files(directory_path);
    database.materials = (Material *)malloc(database.size * sizeof(Material));
    database.material_names = gtk_string_list_new(NULL);
    // Читаем json файлы в каталоге
    dir = opendir(directory_path);
    if (dir == NULL)   g_warning("get_materials_from_directory: Failed to open directory: %s\n", directory_path);

    int count = 0;
    while ((entry = readdir(dir)) != NULL) {
        const char *filename = entry->d_name;
        // Проверяем, что файл заканчивается на ".json"
        if (strstr(filename, ".json") != NULL) {
            FILE *f;
            fopen_s(&f, g_build_filename(directory_path, filename, NULL), "r");

            if (f == NULL) g_warning("Failed to open file: %s\n", filename);  

            gchar buffer[8192];
            size_t bytes_read = fread(buffer, 1, sizeof(buffer) - 1, f);
            buffer[bytes_read] = '\0';
            // g_print("%s\n", buffer);

            if (!buffer) g_warning("get_materials_from_directory: Failed to convert to UTF-8: %s\n", filename);  

            // Парсинг JSON
            cJSON *root = cJSON_Parse(buffer);

            if (!root) g_warning("get_materials_from_directory: Failed to parse JSON: %s\n", filename);


            database.materials[count] = Material_from_json(root); 
            gtk_string_list_append(database.material_names, database.materials[count].name);
            count++;


            fclose(f);
            
        }
    }    
    closedir(dir);
    return database;
}