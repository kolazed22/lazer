#include "main.h"

void freeData_TC(Data_TC* data) { // освобождение памяти
    free(data->C);
    free(data->T);
    data->C = NULL;
    data->T = NULL;
    data->size = 0;
}

Data_TC fread_TC(const char *file_name){

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
        if (data.max_T < val3)  data.max_T = val3;
        if (data.min_T > val3)  data.min_T = val3;
    }
    
    fclose(file);
    return data;
}

Data_Txx shape_Txx(Data_Txx *data){ // обрезает данные, после вызова старые данные удаляются автоматически
    Data_Txx shape_data;
    shape_data.size = 0;
    shape_data.x = NULL;
    shape_data.y = NULL;
    shape_data.T = NULL;
    // определим размер обрезки
    double shape_up = data->min_y;
    double shape_down = data->max_y;
    double shape_left = data->max_x;
    double shape_right = data->min_x;
    for (int i = 0; i < data->size; i++){
        if (data->T[i] > data->min_T){
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
            if (shape_data.max_T < data->T[i])  shape_data.max_T = data->T[i];
            if (shape_data.min_T > data->T[i])  shape_data.min_T = data->T[i];
        }
    }

    freeData_Txx(data);

    return shape_data;
}

void interpolate_color(double value, double min, double max, double *r, double *g, double *b) {
    double v = (value - min)/(max-min);

    *r = MAX(0,  2*v - 1);
    *b = MAX(0, -2*v+ 1);
    *g = 1 - *r - *b;
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
    // double near_ix = 0;
    // double near_iy = 0;
    double near_T = 0;
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

void draw_heatmap(Data_Txx data, int width, int height, const char *write_file_name){
    cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    cairo_t *cr = cairo_create(surface);

    double scale_x = (data.max_x - data.min_x)/width;
    double scale_y = (data.max_y - data.min_y)/height;
    if (scale_x < scale_y)  scale_x = scale_y;
    if (scale_x > scale_y)  scale_y = scale_x;
    
    for (int iy = 0; iy < height; iy++){
        for (int ix = 0; ix < width; ix++){
            double value = find_nearest_points(data, ix*scale_x+data.min_x, iy*scale_y+data.min_y);
            double r, g, b;
            interpolate_color(value, data.min_T, data.max_T, &r, &g, &b);
            // Устанавливаем цвет
            cairo_set_source_rgb(cr, r, g, b);
            // Рисуем прямоугольник
            cairo_rectangle(cr, ix, iy, 1, 1);

            cairo_fill(cr);
        }
    }
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_set_line_width(cr, 2.0); // Толщина линии
    cairo_rectangle(cr, 0, 0, width, height);
    cairo_stroke(cr);

    cairo_surface_write_to_png(surface, write_file_name);
    // Освобождаем ресурсы
    cairo_destroy(cr);
    cairo_surface_destroy(surface);
}
