#include "main.h"


void freeData_TC(Data_TC* data) { // освобождение памяти
    free(data->C);
    free(data->T);
    data->C = NULL;
    data->T = NULL;
    data->size = 0;
}

Data_TC fread_TC(const char *file_name){
    setlocale(LC_ALL, "English_United States.1252");
    // Преобразование UTF-8 в системную кодировку
    gchar *file_name_locale = g_locale_from_utf8(file_name, -1, NULL, NULL, NULL);
    if (file_name_locale == NULL) {
        g_critical("fread_TC: Ошибка преобразования кодировки\n");
        exit(EXIT_FAILURE);
    }

    FILE* file = fopen(file_name_locale, "r");
    if (file == NULL) {
        g_critical("fread_TC: Failed to open file");
        exit(EXIT_FAILURE);
    }

    Data_TC data;
    data.size = 0;
    data.C = NULL;
    data.T = NULL;

    size_t capacity = 10; // Начальная емкость массива
    data.C = (double*)malloc(capacity * sizeof(double));
    data.T = (double*)malloc(capacity * sizeof(double));

    if (data.C == NULL || data.T == NULL) {
        g_critical("fread_TC: Failed to allocate memory");
        exit(EXIT_FAILURE);
    }

    double val1, val2;
    while (fscanf(file, "%lf %lf", &val1, &val2) == 2) {
        if (data.size >= capacity) {
            capacity *= 2;
            data.C = (double*)realloc(data.C, capacity * sizeof(double));
            data.T = (double*)realloc(data.T, capacity * sizeof(double));
            if (data.C == NULL || data.T == NULL) {
                g_critical("Failed to reallocate memory");
                exit(EXIT_FAILURE);
            }
        }
        data.T[data.size] = val1;
        data.C[data.size] = val2;
        data.size++;
    }
    
    fclose(file);
    return data;
}

void freeData_Tt(Data_Tt* data) { // освобождение памяти
    free(data->t);
    free(data->T);
    data->t = NULL;
    data->T = NULL;
    data->size = 0;
}

Data_Tt fread_Tt(const char *file_name){
    setlocale(LC_ALL, "English_United States.1252");
    // Преобразование UTF-8 в системную кодировку
    gchar *file_name_locale = g_locale_from_utf8(file_name, -1, NULL, NULL, NULL);
    if (file_name_locale == NULL) {
        perror("Ошибка преобразования кодировки\n");
        exit(EXIT_FAILURE);
    }

    FILE* file = fopen(file_name_locale, "r");
    if (file == NULL) {
        perror("fread_Tt: Failed to open file");
        exit(EXIT_FAILURE);
    }

    Data_Tt data;
    data.size = 0;
    data.t = NULL;
    data.T = NULL;

    size_t capacity = 10; // Начальная емкость массива
    data.t = (double*)malloc(capacity * sizeof(double));
    data.T = (double*)malloc(capacity * sizeof(double));

    if (data.t == NULL || data.T == NULL) {
        perror("fread_Tt: Failed to allocate memory");
        exit(EXIT_FAILURE);
    }

    double val1, val2;
    while (fscanf(file, "%lf %lf %*lf %*lf", &val1, &val2) == 2) {
        if (data.size >= capacity) {
            capacity *= 2;
            data.t = (double*)realloc(data.t, capacity * sizeof(double));
            data.T = (double*)realloc(data.T, capacity * sizeof(double));
            if (data.t == NULL || data.T == NULL) {
                perror("Failed to reallocate memory");
                exit(EXIT_FAILURE);
            }
        }
        data.t[data.size] = val1;
        data.T[data.size] = val2;
        data.size++;
    }
    
    fclose(file);
    return data;
}

void freeData_Txx(Data_Txx* data) { // освобождение памяти
    free(data->x);
    free(data->y);
    free(data->T);
    data->x = NULL;
    data->y = NULL;
    data->T = NULL;
    data->size = 0;
    data->max_x = 0;
    data->min_x = 0;
    data->max_y = 0;
    data->min_y = 0;
    data->max_T = 0;
    data->min_T = 0;
}

Data_Txx fread_Txx(const char *file_name){
    setlocale(LC_ALL, "English_United States.1252");

    // Преобразование UTF-8 в системную кодировку
    gchar *file_name_locale = g_locale_from_utf8(file_name, -1, NULL, NULL, NULL);
    if (file_name_locale == NULL) {
        perror("Ошибка преобразования кодировки\n");
        // return 1;
    }

    FILE* file = fopen(file_name_locale, "r");
    if (file == NULL) {
        perror("fread_Txx: Failed to open file");
        exit(EXIT_FAILURE);
    }

    Data_Txx data;
    data.size = 0;
    data.x = NULL;
    data.y = NULL;
    data.T = NULL;

    size_t capacity = 10; // Начальная емкость массива
    data.x = (double*)malloc(capacity * sizeof(double));
    data.y = (double*)malloc(capacity * sizeof(double));
    data.T = (double*)malloc(capacity * sizeof(double));

    bool firs_read_flag = false;

    if (data.x == NULL || data.y == NULL || data.T == NULL) {
        perror("fread_Txx: Failed to allocate memory");
        exit(EXIT_FAILURE);
    }

    double val1, val2, val3;
    while (fscanf(file, "%le %le %lf", &val1, &val2, &val3) == 3) {
        if (firs_read_flag == false){
            data.max_T = val3;
            data.min_T = val3;
            data.max_x = val1;
            data.min_x = val1;
            data.max_y = val2;
            data.min_y = val2;
            firs_read_flag = true;
        }

        if (data.size >= capacity) {
            capacity *= 2;
            data.x = (double*)realloc(data.x, capacity * sizeof(double));
            data.y = (double*)realloc(data.y, capacity * sizeof(double));
            data.T = (double*)realloc(data.T, capacity * sizeof(double));
            if (data.x == NULL || data.y == NULL || data.T == NULL) {
                perror("Failed to reallocate memory");
                exit(EXIT_FAILURE);
            }
        }

        data.x[data.size] = val1;
        data.y[data.size] = val2;
        data.T[data.size] = val3;
        data.size++;

        if (data.max_x < val1)  data.max_x = val1;
        if (data.min_x > val1)  data.min_x = val1;
        if (data.max_y < val2)  data.max_y = val2;
        if (data.min_y > val2)  data.min_y = val2;
        if (data.min_T > val3)  data.min_T = val3;
        if (data.max_T < val3){
            data.max_T = val3;
            data.max_T_x = val1;
            data.max_T_y = val2;
        }
    }

    fclose(file);
    return data;
}

Data_Txx shape_Txx(Data_Txx *data){ // обрезает данные, после вызова старые данные удаляются автоматически
    setlocale(LC_ALL, "English_United States.1252");
    Data_Txx shape_data;
    shape_data.size = 0;
    shape_data.x = NULL;
    shape_data.y = NULL;
    shape_data.T = NULL;
    shape_data.min_T = DBL_MAX;    
    shape_data.max_T = -DBL_MAX;
    shape_data.min_y = DBL_MAX;
    shape_data.max_y = -DBL_MAX;
    shape_data.min_x = DBL_MAX;
    shape_data.max_x = -DBL_MAX;

    // определим размер обрезки
    double shape_up = data->min_y;
    double shape_down = data->max_y;
    double shape_left = data->max_x;
    double shape_right = data->min_x;

    double shape_T = (data->max_T - data->min_T) * 0.05 + data->min_T;

    for (int i = 0; i < data->size; i++){
        if (data->T[i] >= shape_T){
            if (shape_up < data->y[i])      shape_up = data->y[i];
            if (shape_down > data->y[i])    shape_down = data->y[i];
            if (shape_left > data->x[i])    shape_left = data->x[i];
            if (shape_right < data->x[i])   shape_right = data->x[i];
        }
    }
    // обрежим данные
    size_t capacity = 10; // Начальная емкость массива
    shape_data.x = (double*)malloc(capacity * sizeof(double));
    shape_data.y = (double*)malloc(capacity * sizeof(double));
    shape_data.T = (double*)malloc(capacity * sizeof(double));

    if (shape_data.x == NULL || shape_data.y == NULL || shape_data.T == NULL) {
        g_critical("shape_Txx: Failed to allocate memory");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < data->size; i++){
        if (shape_data.size >= capacity) {
            capacity *= 2;
            shape_data.x = (double*)realloc(shape_data.x, capacity * sizeof(double));
            shape_data.y = (double*)realloc(shape_data.y, capacity * sizeof(double));
            shape_data.T = (double*)realloc(shape_data.T, capacity * sizeof(double));
            if (shape_data.x == NULL || shape_data.y == NULL || shape_data.T == NULL) {
                g_critical("shape_Txx: Failed to reallocate memory");
                exit(EXIT_FAILURE);
            }
        }

        if (data->y[i] <= shape_up && data->y[i] >= shape_down && data->x[i] <= shape_right && data->x[i] >= shape_left){
            shape_data.x[shape_data.size] = data->x[i];
            shape_data.y[shape_data.size] = data->y[i];
            shape_data.T[shape_data.size] = data->T[i];
            shape_data.size++;

            if (shape_data.max_x < data->x[i])  shape_data.max_x = data->x[i];
            if (shape_data.min_x > data->x[i])  shape_data.min_x = data->x[i];
            if (shape_data.max_y < data->y[i])  shape_data.max_y = data->y[i];
            if (shape_data.min_y > data->y[i])  shape_data.min_y = data->y[i];
            if (shape_data.min_T > data->T[i])  shape_data.min_T = data->T[i];
            if (shape_data.max_T < data->T[i]){
                shape_data.max_T = data->T[i];
                shape_data.max_T_x = data->x[i];
                shape_data.max_T_y = data->y[i];
            }
        }
    }

    freeData_Txx(data);

    return shape_data;
}

void interpolate_color(double value, double min, double max, double *r, double *g, double *b) {
    double v = (value - min)/(max-min);

    if (v <= 0.0) {
        *r = 0.0;
        *g = 0.0;
        *b = 1.0;
    } else if (v <= 0.2) {
        double t = v / 0.2;
        *r = 0.0;
        *g = t;
        *b = 1.0;
    } else if (v <= 0.4) {
        double t = (v - 0.2) / 0.2;
        *r = 0.0;
        *g = 1.0;
        *b = 1.0 - t;
    } else if (v <= 0.6) {
        double t = (v - 0.4) / 0.2;
        *r = t;
        *g = 1.0;
        *b = 0.0;
    } else if (v <= 1.0) {
        double t = (v - 0.6) / 0.4;
        *r = 1.0;
        *g = 1.0 - t;
        *b = 0.0;
    } else {
        *r = 1.0;
        *g = 0.0;
        *b = 0.0;
    }
}

void draw_plot_TC(Data_TC data, const char *file_write_name){ // создает график kplot и сохраняет его
    struct kpair points[data.size];
    struct kdata	*d;
    struct kplot	*p;
    cairo_surface_t	*surf;
    size_t           i;
    cairo_t		    *cr;


    for (i = 0; i < data.size; i++) {
        points[i].x = data.T[i];
        points[i].y = data.C[i];
    }
    d = kdata_array_alloc(points, data.size);
    p = kplot_alloc(NULL);
    kplot_attach_data(p, d, KPLOT_LINES, NULL);
    kdata_destroy(d);
    surf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 600, 600);
    cr = cairo_create(surf);
    cairo_surface_destroy(surf);
    kplot_draw(p, 600.0, 600.0, cr);

    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_set_line_width(cr, 5.0); // Толщина линии
    cairo_rectangle(cr, 0, 0, 600, 600);
    cairo_stroke(cr);

    cairo_surface_write_to_png(cairo_get_target(cr), file_write_name);
    cairo_destroy(cr);
    kplot_free(p);
}

void draw_plot_Tt(Data_Tt data, const char *file_write_name){ // создает график kplot и сохраняет его
    struct kpair points[data.size];
    struct kdata	*d;
    struct kplot	*p;
    cairo_surface_t	*surf;
    size_t           i;
    cairo_t		    *cr;

    for (i = 0; i < data.size; i++) {
        points[i].x = data.t[i];
        points[i].y = data.T[i];
    }
    d = kdata_array_alloc(points, data.size);
    p = kplot_alloc(NULL);
    kplot_attach_data(p, d, KPLOT_LINES, NULL);
    kdata_destroy(d);
    surf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 600, 600);
    cr = cairo_create(surf);
    cairo_surface_destroy(surf);
    kplot_draw(p, 600.0, 600.0, cr);

    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_set_line_width(cr, 5.0); // Толщина линии
    cairo_rectangle(cr, 0, 0, 600, 600);
    cairo_stroke(cr);

    cairo_surface_write_to_png(cairo_get_target(cr), file_write_name);
    cairo_destroy(cr);
    kplot_free(p);
}

double dist(double x1, double y1, double x2, double y2){ // растояние между точками в квадрате
    return (x2-x1)*(x2-x1)+(y2-y1)*(y2-y1);
}

double find_nearest_points(Data_Txx data, double x, double y){

    double near_T = data.min_T;
    if (x < data.min_x || x > data.max_x || y < data.min_y || y > data.max_y) return near_T;

    double near_r = dist(data.min_x, data.min_y, data.max_x, data.max_y);

    for (int i = 0; i < data.size; i++){
        double r = dist(x, y, data.x[i], data.y[i]);
        if (near_r > r) {
            near_r = r;
            near_T = data.T[i];
        }
    }
    return near_T;
}

void draw_heatmap(Data_Txx data, int width, int height, const char *write_file_name) {
    // Устанавливаем количество потоков (например, 4)
    omp_set_num_threads(4);

    cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width , height);
    int h_width = width;
    int h_height = height - 50;
    // Получаем прямой доступ к данным пикселей
    unsigned char *pixels = cairo_image_surface_get_data(surface);
    int stride = cairo_image_surface_get_stride(surface);
    
    // Блокируем поверхность для прямого доступа к данным
    cairo_surface_flush(surface);
    
    double scale_x = (data.max_x - data.min_x)/h_width;
    double scale_y = (data.max_y - data.min_y)/h_height;
    if (scale_x > scale_y)  scale_x = scale_y;
    if (scale_x < scale_y)  scale_y = scale_x;

    double dy = data.max_T_y - h_height/2*scale_y - data.min_y;
    double dx = data.max_T_x - h_width/2*scale_x - data.min_x;

    // Распараллеливаем внешний цикл с помощью OpenMP
    #pragma omp parallel for schedule(dynamic, 16)
    for (int iy = 0; iy < h_height; iy++) {
        for (int ix = 0; ix < h_width; ix++) {
            double value = find_nearest_points(data, ix*scale_x+data.min_x+dx, iy*scale_y+data.min_y+dy);
            
            double r, g, b;
            interpolate_color(value, data.min_T, data.max_T, &r, &g, &b);
            
            // Преобразуем значения RGB (0-1) в формат ARGB32
            unsigned int alpha = 0xFF; // Полная непрозрачность
            unsigned int red = (unsigned int)(r * 255);
            unsigned int green = (unsigned int)(g * 255);
            unsigned int blue = (unsigned int)(b * 255);
            
            // Формат ARGB32: каждый пиксель - 4 байта (A,R,G,B)
            unsigned int *pixel = (unsigned int*)(pixels + iy * stride + ix * 4);
            
            // Атомарная операция записи для избежания гонок данных
            #pragma omp atomic write
            *pixel = (alpha << 24) | (red << 16) | (green << 8) | blue;
        }
    }

    // Сообщаем Cairo, что мы изменили данные
    cairo_surface_mark_dirty(surface);
    
    // Рисуем рамку вокруг изображения
    cairo_t *cr = cairo_create(surface);
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_set_line_width(cr, 2.0); // Толщина линии
    cairo_rectangle(cr, 0, 0, width, height);
    cairo_stroke(cr);
    // Рисуем рамку фокруг heatmap
    cairo_rectangle(cr, 0, 0, h_width, h_height);
    cairo_stroke(cr);
    
    // Добавляем температурную шкалу
    int scale_height = height - h_height;
    int scale_padding = 10; // отступ от краев
    int text_height = 15;   // высота для текста
    
    // Рисуем градиентную шкалу
    for (int ix = scale_padding; ix < width - scale_padding; ix++) {
        // Вычисляем значение температуры для данной позиции
        double t_value = data.min_T + (data.max_T - data.min_T) * 
                        (double)(ix - scale_padding) / (double)(width - 2 * scale_padding);
        
        double r, g, b;
        interpolate_color(t_value, data.min_T, data.max_T, &r, &g, &b);
        
        cairo_set_source_rgb(cr, r, g, b);
        cairo_rectangle(cr, ix, h_height + 5, 1, scale_height - text_height - 10);
        cairo_fill(cr);
    }
    
    // Рисуем рамку вокруг шкалы
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_rectangle(cr, scale_padding, h_height + 5, 
                   width - 2 * scale_padding, scale_height - text_height - 10);
    cairo_stroke(cr);
    
    // Добавляем текстовые метки
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, 12);
    cairo_set_source_rgb(cr, 0, 0, 0);
    
    // Минимальное значение
    char min_label[32];
    snprintf(min_label, sizeof(min_label), "%.1f", data.min_T);
    cairo_move_to(cr, scale_padding, h_height + scale_height - 5);
    cairo_show_text(cr, min_label);
    
    // Максимальное значение
    char max_label[32];
    snprintf(max_label, sizeof(max_label), "%.1f", data.max_T);
    cairo_text_extents_t extents;
    cairo_text_extents(cr, max_label, &extents);
    cairo_move_to(cr, width - scale_padding - extents.width, h_height + scale_height - 5);
    cairo_show_text(cr, max_label);
    
    // Среднее значение
    char mid_label[32];
    snprintf(mid_label, sizeof(mid_label), "%.1f", (data.min_T + data.max_T) / 2);
    cairo_text_extents(cr, mid_label, &extents);
    cairo_move_to(cr, width / 2 - extents.width / 2, h_height + scale_height - 5);
    cairo_show_text(cr, mid_label);
    
    // Сохраняем результат
    cairo_surface_write_to_png(surface, write_file_name);
    
    // Освобождаем ресурсы
    cairo_destroy(cr);
    cairo_surface_destroy(surface);
}


