#include <gtk/gtk.h>
#include <cairo.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include <kplot.h>

#define Ntr 8     //количество потоков
#define pi 3.14159  //число пи
#define Nx1 102     //число узлов по оси х локальной сетки
#define Ny 64       //число узлов по оси у
#define Nz 70       //чило узловпо оси z
#define Nx2 200     //число узлов по оси х глобальной сетки

// многопоточность
GMutex mut;
GThread *thread;

// виджет ввода
GtkWidget *entry_P;
GtkWidget *entry_V;
GtkWidget *entry_F;

// виджет полоски прогресса
GtkWidget *progress_bar;

// виджет изображения для графиков
GtkWidget *image_heatmapTxy, *image_heatmapTxz, *image_heatmapTyz, *image_plotTt;

typedef struct {
    double* t; // время
    double* T; // температура
    size_t size;
} Data_Tt;

void freeData_Tt(Data_Tt* data) { // освобождение памяти
    free(data->t);
    free(data->T);
    data->t = NULL;
    data->T = NULL;
    data->size = 0;
}

Data_Tt fread_Tt(const char *file_name){
    FILE* file = fopen(file_name, "r");
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

typedef struct {
    double* x; // x
    double* y; // x
    double* T; // температура
    size_t size;

    double max_x;
    double min_x;
    double max_y;
    double min_y;
    double max_T;
    double min_T;
} Data_Txx;

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
    FILE* file = fopen(file_name, "r");
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
        perror("shape_Txx: Failed to allocate memory");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < data->size; i++){
        if (shape_data.size >= capacity) {
            capacity *= 2;
            shape_data.x = (double*)realloc(shape_data.x, capacity * sizeof(double));
            shape_data.y = (double*)realloc(shape_data.y, capacity * sizeof(double));
            shape_data.T = (double*)realloc(shape_data.T, capacity * sizeof(double));
            if (shape_data.x == NULL || shape_data.y == NULL || shape_data.T == NULL) {
                perror("shape_Txx: Failed to reallocate memory");
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

void draw_plot(Data_Tt data, const char *file_write_name){ // создает график kplot и сохраняет его
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

typedef struct{
    double x1, y1, T1;
    double x2, y2, T2;
    double x3, y3, T3;
    double x4, y4, T4;
}Points;

Points find_nearest_4points(Data_Txx data, double x, double y){ // нахождение 4 ближайших точек вокруг указанной
    int ur, ul, dr, dl;
    double ur_r, ul_r, dr_r, dl_r;
    ur_r = dist(data.min_x, data.min_y, data.max_x, data.max_y);
    ul_r = ur_r;
    dr_r = ur_r;
    dl_r = ur_r;

    Points point;
    for (int i = 0; i < data.size; i++){
        double r = dist(x, y, data.x[i], data.y[i]);
        if (ur_r > r && x < data.x[ur] && y < data.y[ur]) {
            ur_r = r;
            point.x1 = data.x[ur];
            point.y1 = data.y[ur];
            point.T1 = data.T[ur];
        }
        if (ul_r > r && data.x[ul] < x && y < data.y[ul] ) {
            ul_r = r;
            point.x2 = data.x[ul];
            point.y2 = data.y[ul];
            point.T2 = data.T[ul];
        }
        if (dr_r > r && x < data.x[dr] && data.y[dr] < y ) {
            dr_r = r;
            point.x3 = data.x[dr];
            point.y3 = data.y[dr];
            point.T3 = data.T[dr];
        }
        if (dl_r > r && data.x[dl] < x && data.y[dl] < y ) {
            dl_r = r;
            point.x4 = data.x[dl];
            point.y4 = data.y[dl];
            point.T4 = data.T[dl];
        }
    }

    return point;
}

// Функция линейной интерполяции
double lerp(double v0, double v1, double t) {
    return v0 + (v1 - v0) * t;
}

// Функция двулинейной интерполяции
double biline_interpolate(double x, double y, Points p){
    double xd = (x - p.x4) / (p.x1 - p.x4);
    double yd = (y - p.y4) / (p.y1 - p.y4);

    double c0 = lerp(p.T4, p.T3, xd);
    double c1 = lerp(p.T2, p.T1, xd);

    return lerp(c0, c1, yd);
}

void draw_hetmap_4points(Data_Txx data, int width, int height, const char *write_file_name){
    cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    cairo_t *cr = cairo_create(surface);

    double dx = 1e-9;
    double dy = 1e-9;
    double x = data.min_x + dx;
    double y = data.min_y + dy;

    double scale_x = (data.max_x - data.min_x)/width;
    double scale_y = (data.max_y - data.min_y)/height;
    if (scale_x < scale_y)  scale_y = scale_x;
    if (scale_x > scale_y)  scale_x = scale_y;

    
    int ix = 0;
    int iy = 0;
    Points points = find_nearest_4points(data, ix*scale_x+data.min_x+dx, iy*scale_y+data.min_y+dy);
    while (ix < width || iy < height)
    {
        ix = (x - data.min_x)/scale_x;
    }
    

    for (int iy = 0; iy < height; iy++){
        for (int ix = 0; ix < width; ix++){
            Points points = find_nearest_4points(data, x, y);
            double value = biline_interpolate(x, y, points);

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


void update_plots(GtkWidget *widget, gpointer data){
    Data_Tt data_Tt = fread_Tt("out_Tt.txt");
    draw_plot(data_Tt, "plotTt.png");
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

/*функция для вычисление теплопроводности*/
double hcond(double T)
{
    double ks;
    ks = 6+0.015*(T-300);
    //ks = 25;
    return ks;
}
/*функция для вычисления теплоемкости*/
double hcap(double T)
{
    double Cs,
        Tm = 1933,
        po=4300,
        Lm =290e3*po,
        dTm = 40;
    Cs = (-1.1e-4*pow(T-2000,2)+840)*po + Lm / (dTm * sqrt(pi)) * exp(-pow(T - Tm, 2) / pow(dTm, 2));
    //Cs = 2.5e6;
    return Cs;
}
/*функция для вычисления приращения тепловой энергии*/ //не нужна для основного расчета
double int_energy(double T)
{
    double Eint,
           Tm = 1933,
           po = 4300,
           Lm = 290e3*po,
           dTm = 40,
           T0 = 291;
    Eint = (3.667e-5 * (pow(2000 - T, 3) - pow(2000 - T0, 3)) + 840 * (T - T0))*po + Lm / 2 * erfc((Tm - T) / dTm);
    //Eint = 2.5e6 * (T - T0);
    return Eint;
}

int minimarker(double v, double f, double P)
{
    static double T1[Nx1][Ny][Nz], T2[Nx2][Ny][Nz];  //массивы для температуры на локальной и глобальной сетках
    static double ATz1[Nz], BTz1[Nz], ATy1[Ny], BTy1[Ny], ATx1[Nx1], BTx1[Nx1];
    static double ATx2[Nx2], BTx2[Nx2], ATy2[Ny], BTy2[Ny], ATz2[Nz], BTz2[Nz];
    static double q0[Nx1][Ny]; //массив для плотности мощности
    static double z[Nz], y[Ny];  //узлы секти по осям у и z
    static double sum_z[Nx2][Ny], sum_zy[Nx2];
    double a, b, c, d, g, JT, Jn, jn, nominx, j1, j2, gn, uevmax,Tav,Esum,Eabs=0;
    double T0 = 291,            //начальная температура
           Tm = 1933,           //температура плавления
           t0 = 40e-9,          //длительность переднего фрона импульса
           r0 = 25e-6,          //радиус пучка по уровню e^-1
           l0 = 1.06e-6,        //длина волны
           A = 0.4,             //поглощательная способность
           po=4300,             // плотность 
          //  P,                   //средняя мощность
           Q0,                  //плотность энергии в центре пучка
          //  f,                   //частота следования импульсов
          //  v,                   //скорость сканирования
           cs,
           ks0,
           ks1;
    double dx1 = r0/25, dx2 = 3 * dx1; //шаги по сетки по координате х
    double dt10, dt1, dt2;
    double time = 0, time1, time2, timestep = 10e-9, counttime;
    double delta = 0, Tmax = 0;
    FILE* f1, * f2, * f3, * f4, * f5, * f6, * f7, * f8, * f9, * f10, * f11;
    fopen_s(&f1, "out_Txy.txt", "w"); //распределение температуры в плоскости xy  
    fopen_s(&f2, "out_Tz.txt", "w");  //зависимость температуры от глубины в центре трека в момент времени достижения максимальной температуры
    fopen_s(&f3, "out_Tt.txt", "w");  // зависимость темпеарутры поверхности для точки в центре трека от времени
    fopen_s(&f4, "out_test.txt", "w"); 
    fopen_s(&f5, "out_Txz.txt", "w");  //распределение температуры в плоскости xz  
    fopen_s(&f6, "out_Tyz.txt", "w");  //распределение температуры в плоскости yz  
    int i, j, k, t, tmax1, tmax2, np, Np, np0;
    int i0, ind, i00 = 30;
    int ni;
    int Nzmax1 = 35;
    int Nymax1 = 40;
    int Nxmax2 = i00+ 3 * (int)floor(r0 / dx2);
    int Nymax2 = 40;
    int Nzmax2 = 35;


    /*ввод скорости, частоты и средней мощности с клавиатуры*/
    // printf("Input v (mm/s):");
    // scanf_s("%lf", &v);
    // printf("Input f (kHz):");
    // scanf_s("%lf", &f);
    // printf("Input P (W):");
    // scanf_s("%lf", &P);
    printf("\n\nP=%.2f W\nv=%.2f mm/s\nf=%.2f kHz\nr0=%.2f um\ntp=%.3f ns\n\n", P, v, f, r0 * 1e6, 2.5 * t0 * 1e9);


    clock_t timeprog;
    timeprog = clock();
    printf("executing...  %.2f%%\r", 0);
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress_bar), 0.0);
    f = f * 1e3;
    v = v / 1e3;
    i0 = i00;
    dt10 = 6*t0/40;
    Np = 5 * 2 * 1.33 * r0 / v * f;
    Q0 = (P / f) / (pi * pow(r0, 2));
    np0 = (Nx2 / 2 - i0) * dx2 / (v / f);

    /*генерация узлов сетки*/
    y[0] = 0;
    for (j = 0; j < Ny - 1; j++)
    {
        y[j + 1] = y[j] + (30e-9 + pow(j + 1, 2.5) * 0.2e-4 * r0);
        if (y[j + 1]-y[j] > 40e-6)
            y[j + 1] = y[j] + 40e-6;
    }
    z[0] = 0;
    for (k = 0; k < Nz - 1; k++)
    {
        z[k + 1] = z[k] + (30e-9 + pow(k + 1, 2.5) * 0.2e-4 * r0);
        if (z[k + 1]-z[k] >= 40e-6)
            z[k + 1] = z[k] + 40e-6;
    }
   
    /*задание начальных условий*/
    for (k = 0; k < Nz; k++)
        for (j = 0; j < Ny; j++)
        {
            for (i = 0; i < Nx1; i++)
             T1[i][j][k] = T0;
            for (i = 0; i < Nx2; i++)
             T2[i][j][k] = T0;
        }

   /*цикл по количенству импульсов*/
   for (np = 0; np < Np; np++)
    {
        time1 = 0;
        time2 = 0;
        ind = 1;
        counttime = 0;
        dt1 = dt10;

        /*расчет температуры в течение импульса*/
        while (time1 < 6 * t0)
        {
#pragma omp parallel for shared(T1,Eabs) private(i,j,k,ATz1,BTz1,ks0,ks1,cs,a,b,c,d,g,JT) schedule (dynamic) num_threads(Ntr)
            for (i = 0; i < Nx1; i++)
                for (j = 0; j <= Nymax1; j++)
                {
                   q0[i][j] = A * Q0 / pow(t0, 2) * time1 * exp(-time1 / t0) * exp(-(pow((i - Nx1 / 2) * dx1 - delta, 2) + pow(y[j], 2)) / pow(r0, 2));
                   JT = q0[i][j] / (z[1] - z[0]);
                   ks1 = hcond(T1[i][j][0]);
                   cs = hcap(T1[i][j][0]);
                   b = -(ks1 / pow(z[1] - z[0], 2) + cs / dt1);
                   c = ks1 / pow(z[1] - z[0], 2);
                   d = -(T1[i][j][0] * cs / dt1 + JT);
                   ATz1[0] = -c / b;
                   BTz1[0] = d / b;
                   for (k = 1; k < Nzmax1; k++)
                    {
                        JT = 0;
                        Jn = 0;
                        ks0 = hcond(T1[i][j][k]);
                        ks1 = hcond(T1[i][j][k + 1]);
                        cs = hcap(T1[i][j][k]);
                        a = ks0 / pow(z[k] - z[k - 1], 2);
                        b = -(cs / dt1 + ks0 / pow(z[k] - z[k - 1], 2) + ks1 / ((z[k + 1] - z[k]) * (z[k] - z[k - 1])));
                        c = ks1 / ((z[k + 1] - z[k]) * (z[k] - z[k - 1]));
                        d = -(T1[i][j][k] * cs / dt1 + JT);
                        g = a * ATz1[k - 1] + b;
                        ATz1[k] = -c / g;
                        BTz1[k] = (d - a * BTz1[k - 1]) / g;
                    }
                    ks1= hcond(T1[i][j][Nzmax1-1]);
                    cs= hcap(T1[i][j][Nzmax1-1]);
                    a = ks1 / pow(z[Nzmax1 - 1] - z[Nzmax1 - 2], 2);
                    b = -ks1 / pow(z[Nzmax1 - 1] - z[Nzmax1 - 2], 2) - cs / dt1;
                    d = -cs / dt1*T1[i][j][Nzmax1-1];
                    T1[i][j][Nzmax1 - 1] = (d - a * BTz1[Nzmax1 - 2]) / (b + a * ATz1[Nzmax1 - 2]);
                    for (k = Nzmax1 - 2; k >= 0; k--)
                      T1[i][j][k] = ATz1[k] * T1[i][j][k + 1] + BTz1[k];
                }
            if (r0<=10*sqrt(2.5*t0*hcond(T0)/hcap(T0))) //уловие расчета теплообмена в радиальных направлениях в течение импульса
            {
                for (i = 0; i < Nx1; i++)
                    for (k = 0; k < Nzmax1; k++)
                    {
                        JT = 0;
                        Jn = 0;
                        ks0 = hcond(T1[i][0][k]);
                        cs = hcap(T1[i][0][k]);
                        b = -(ks0 / pow(y[1] - y[0], 2) + cs / dt1);
                        c = ks0 / pow(y[1] - y[0], 2);
                        d = -(T1[i][0][k] * cs / dt1 + JT);
                        ATy1[0] = -c / b;
                        BTy1[0] = d / b;
                        for (j = 1; j < Nymax1; j++)
                        {
                            JT = 0;
                            Jn = 0;
                            ks0 = hcond(T1[i][j][k]);
                            ks1 = hcond(T1[i][j + 1][k]);
                            cs = hcap(T1[i][j][k]);
                            a = ks0 / pow(y[j] - y[j - 1], 2);
                            b = -(cs / dt1 + ks0 / pow(y[j] - y[j - 1], 2) + ks1 / ((y[j + 1] - y[j]) * (y[j] - y[j - 1])));
                            c = ks1 / ((y[j + 1] - y[j]) * (y[j] - y[j - 1]));
                            d = -(T1[i][j][k] * cs / dt1 + JT);
                            g = a * ATy1[j - 1] + b;
                            ATy1[j] = -c / g;
                            BTy1[j] = (d - a * BTy1[j - 1]) / g;
                        }
                        cs = hcap(T1[i][Nymax1][k]);
                        ks0 = hcond(T1[i][Nymax1][k]);
                        JT = 0;
                        Jn = 0;
                        a = ks0 / pow(y[Nymax1] - y[Nymax1 - 1], 2);
                        b = -(cs / dt1 + ks0 / pow(y[Nymax1] - y[Nymax1 - 1], 2));
                        d = -(T1[i][Nymax1][k] * cs / dt1 + JT);
                        T1[i][Nymax1][k] = (d - a * BTy1[Nymax1 - 1]) / (b + a * ATy1[Nymax1 - 1]);
                        for (j = Nymax1 - 1; j >= 0; j--)
                            T1[i][j][k] = ATy1[j] * T1[i][j + 1][k] + BTy1[j];
                    }
                for (k = 0; k < Nzmax1; k++)
                    for (j = 0; j <= Nymax1; j++)
                    {
                        JT = 0;
                        ks0 = hcond(T1[0][j][k]);
                        cs = hcap(T1[0][j][k]);
                        b = -(ks0 / pow(dx1, 2) + cs / dt1);
                        c = ks0 / pow(dx1, 2);
                        d = -(T1[0][j][k] * cs / dt1 + JT);
                        ATx1[0] = -c / b;
                        BTx1[0] = d / b;
                        for (i = 1; i < Nx1 - 1; i++)
                        {
                            JT = 0;
                            Jn = 0;
                            ks0 = hcond(T1[i][j][k]);
                            ks1 = hcond(T1[i + 1][j][k]);
                            cs = hcap(T1[i][j][k]);
                            a = ks0 / pow(dx1, 2);
                            b = -(cs / dt1 + (ks0 + ks1) / pow(dx1, 2));
                            c = ks1 / pow(dx1, 2);
                            d = -(T1[i][j][k] * cs / dt1 + JT);
                            g = a * ATx1[i - 1] + b;
                            ATx1[i] = -c / g;
                            BTx1[i] = (d - a * BTx1[i - 1]) / g;
                        }
                        cs = hcap(T1[Nx1 - 1][j][k]);
                        ks0 = hcond(T1[Nx1 - 1][j][k]);
                        JT = 0;
                        a = ks0 / pow(dx1, 2);
                        b = -(cs / dt1 + ks0 / pow(dx1, 2));
                        d = -(T1[Nx1 - 1][j][k] * cs / dt1 + JT);
                        T1[Nx1 - 1][j][k] = (d - a * BTx1[Nx1 - 2]) / (b + a * ATx1[Nx1 - 2]);
                        for (i = Nx1 - 2; i >= 0; i--)
                            T1[i][j][k] = ATx1[i] * T1[i + 1][j][k] + BTx1[i];
                    }
            }
            time1 = time1 + dt1;
            time = time + dt1;

            /*вывод данных в файл для зависиомсти температуры точки от времени*/
            if ((i0 * dx2 + Nx1 / 2 * dx1 <= Nx2 / 2 * dx2) || (i0 * dx2 - Nx1 / 2 * dx1 >= Nx2 / 2 * dx2))
            {
                fprintf(f3, "%e  %.3f  %e  %e\n", time, T2[Nx2 / 2][0][0]);
                counttime = counttime + dt1;
            }
            else
            {
                fprintf(f3, "%e  %.3f  %e  %e\n", time, T1[Nx1 / 2 + 3 * (Nx2 / 2 - i0)][0][0]);
                counttime = counttime + dt1;
            }


            /*определение момента времни достижения максимальной температуры в исследуемой точке и вывод данных в файлы*/
            if (np == np0)
                if (time1 > t0)
                    if (ind == 1)
                        if (T1[Nx1 / 2][0][0] < Tmax)
                        {
                            for (i = 0; i < Nx1; i++)
                            {
                                for (j = 0; j < Ny; j++)
                                {
                                    fprintf(f1, "%e   %e   %.3f\n", ((i0 * dx2 - Nx1 / 2 * dx1) + i * dx1), y[j], T1[i][j][0]);
                                    if (j != 0){
                                        fprintf(f1, "%e   %e   %.3f\n", ((i0 * dx2 - Nx1 / 2 * dx1) + i * dx1), -y[j], T1[i][j][0]);
                                    }
                                }
                                for (k = 0; k < Nz; k++)
                                    fprintf(f5, "%e   %e   %.3f\n", ((i0 * dx2 - Nx1 / 2 * dx1) + i * dx1), z[k], T1[i][0][k]);
                                
                            }
                            
                            for (i = 0; i < Nx2; i++)
                            {
                                for (j = 0; j < Ny; j++)
                                {
                                    if (i * dx2<(i0 * dx2 - Nx1 / 2 * dx1) || i * dx2>(i0 * dx2 + Nx1 / 2 * dx1))
                                    {
                                        fprintf(f1, "%e   %e   %.3f\n", i * dx2, y[j], T2[i][j][0]);
                                        if (j != 0)
                                            fprintf(f1, "%e   %e   %.3f\n", i * dx2, -y[j], T2[i][j][0]);
                                    }
                                }

                                for (k = 0; k < Nz; k++)
                                    if (i * dx2<(i0 * dx2 - Nx1 / 2 * dx1) || i * dx2>(i0 * dx2 + Nx1 / 2 * dx1))
                                     fprintf(f5, "%e   %e   %.3f\n", i * dx2, z[k], T2[i][0][k]);

                            }
                            for(k=0;k<Nz;k++)
                                for (j = 0; j < Ny; j++)
                                {
                                    fprintf(f6, "%e   %e   %.3f\n", y[j], z[k], T1[Nx1/2][j][k]);
                                    if (j != 0)
                                        fprintf(f6, "%e   %e   %.3f\n", -y[j], z[k], T1[Nx1/2][j][k]);
                                }
                                
                            
                                ind = 0;
                        }
         Tmax = T1[Nx1 / 2][0][0];
        }
        

        /*перерасчет температыр на глобальнйо сетке*/
        for (j = 0; j < Ny; j++)
            for (k = 0; k < Nz; k++)
                for (i = i0; i < Nx2; i++)
                {
                    if (3 * (i - i0) < Nx1/2)
                    {
                        T2[i][j][k] = T1[Nx1 / 2 + 3 * (i - i0)][j][k];
                        T2[2 * i0 - 1 - i][j][k] = T1[Nx1 / 2 - 3 * (i + 1 - i0)][j][k];
                       
                    }
                }
        dt2 = dt1;

        /*расчета на стадии остывания между импульсами*/
        while (time2 < (1 / f - time1))
        {
           
            if (dt2 < 0.03 / f)
                dt2 = dt2 * 1.4;
            else
                dt2 = 0.03 / f;
#pragma omp parallel for shared(T2) private(i,j,k,ATz2,BTz2,ks0,ks1,cs,a,b,c,d,g,JT) schedule (dynamic) num_threads(Ntr)
            for (i = 0; i < Nxmax2; i++)
                for (j = 0; j < Nymax2; j++)
                {
                    JT = 0;
                    ATz2[0] = 1;
                    BTz2[0] = 0;
                    for (k = 1; k < Nzmax2 - 1; k++)
                    {
                        JT = 0;
                        ks0 = hcond(T2[i][j][k]);
                        ks1 = hcond(T2[i][j][k + 1]);
                        cs = hcap(T2[i][j][k]);
                        a = ks0 / pow(z[k] - z[k - 1], 2);
                        b = -(cs / dt2 + ks0 / pow(z[k] - z[k - 1], 2) + ks1 / ((z[k + 1] - z[k]) * (z[k] - z[k - 1])));
                        c = ks1 / ((z[k + 1] - z[k]) * (z[k] - z[k - 1]));
                        d = -(T2[i][j][k] * cs / dt2 + JT);
                        g = a * ATz2[k - 1] + b;
                        ATz2[k] = -c / g;
                        BTz2[k] = (d - a * BTz2[k - 1]) / g;
                    }
                    ks1 = hcond(T2[i][j][Nzmax2 - 1]);
                    cs = hcap(T2[i][j][Nzmax2 - 1]);
                    a = ks1 / pow(z[Nzmax2 - 1] - z[Nzmax2 - 2], 2);
                    b = -ks1 / pow(z[Nzmax2 - 1] - z[Nzmax2 - 2], 2) - cs / dt2;
                    d = -cs / dt2 * T2[i][j][Nzmax2 - 1];
                    T2[i][j][Nzmax2 - 1] = (d - a * BTz2[Nzmax2 - 2]) / (b + a * ATz2[Nzmax2 - 2]);
                    for (k = Nzmax2 - 2; k >= 0; k--)
                      T2[i][j][k] = ATz2[k] * T2[i][j][k + 1] + BTz2[k];
                }
#pragma omp parallel for shared(T2) private(i,j,k,ATy2,BTy2,ks0,ks1,cs,a,b,c,d,g,JT) schedule (dynamic) num_threads(Ntr)
                for (i = 0; i < Nxmax2; i++)
                 for (k = 0; k < Nzmax2; k++)
                 {
                    JT = 0;
                    Jn = 0;
                    ATy2[0] = 1;
                    BTy2[0] = 0;
                    for (j = 1; j < Nymax2 - 1; j++)
                    {
                        JT = 0;
                        Jn = 0;
                        ks0 = hcond(T2[i][j][k]);
                        ks1 = hcond(T2[i][j + 1][k]);
                        cs = hcap(T2[i][j][k]);
                        a = ks0 / pow(y[j] - y[j - 1], 2);
                        b = -(cs / dt2 + ks0 / pow(y[j] - y[j - 1], 2) + ks1 / ((y[j + 1] - y[j]) * (y[j] - y[j - 1])));
                        c = ks1 / ((y[j + 1] - y[j]) * (y[j] - y[j - 1]));
                        d = -(T2[i][j][k] * cs / dt2 + JT);
                        g = a * ATy2[j - 1] + b;
                        ATy2[j] = -c / g;
                        BTy2[j] = (d - a * BTy2[j - 1]) / g;
                      
                    }
                    ks1 = hcond(T2[i][Nymax2 - 1][k]);
                    cs = hcap(T2[i][Nymax2 - 1][k]);
                    a = ks1 / pow(y[Nymax2 - 1] - y[Nymax2 - 2], 2);
                    b = -ks1 / pow(y[Nymax2 - 1] - y[Nymax2 - 2], 2) - cs / dt2;
                    d = -cs / dt2 * T2[i][Nymax2 - 1][k];
                    T2[i][Nymax2 - 1][k] = (d - a * BTy2[Nymax2 - 2]) / (b + a * ATy2[Nymax2 - 2]);
                    for (j = Nymax2 - 2; j >= 0; j--)
                     T2[i][j][k] = ATy2[j] * T2[i][j + 1][k] + BTy2[j];
                      
                 }
#pragma omp parallel for shared(T2) private(i,j,k,ATx2,BTx2,ks0,ks1,cs,a,b,c,d,g,JT) schedule (dynamic) num_threads(Ntr)
            for (j = 0; j < Nymax2; j++)
                for (k = 0; k < Nzmax2; k++)
                {
                    JT = 0;
                    ATx2[0] = 1;
                    BTx2[0] = 0;
                    for (i = 1; i < Nxmax2 - 1; i++)
                    {
                        JT = 0;
                       
                        ks0 = hcond(T2[i][j][k]);
                        ks1 = hcond(T2[i + 1][j][k]);
                        cs = hcap(T2[i][j][k]);
                        a = ks0 / pow(dx2, 2);
                        b = -(cs / dt2 + (ks0 + ks1) / pow(dx2, 2));
                        c = ks1 / pow(dx2, 2);
                        d = -(T2[i][j][k] * cs / dt2 + JT);
                        g = a * ATx2[i - 1] + b;
                        ATx2[i] = -c / g;
                        BTx2[i] = (d - a * BTx2[i - 1]) / g;
                      
                    }
                    ks1 = hcond(T2[Nxmax2 - 1][j][k]);
                    cs = hcap(T2[Nxmax2 - 1][j][k]);
                    a = ks1 / pow(dx2, 2);
                    b = -ks1 / pow(dx2, 2) - cs / dt2;
                    d = -cs / dt2 * T2[Nxmax2 - 1][j][k];
                    T2[Nxmax2 - 1][j][k] = (d - a * BTx2[Nxmax2 - 2]) / (b + a * ATx2[Nxmax2 - 2]);
                    for (i = Nxmax2 - 2; i >= 0; i--)
                      T2[i][j][k] = ATx2[i] * T2[i + 1][j][k] + BTx2[i];
                }
            time2 = time2 + dt2;
            time = time + dt2;
            fprintf(f3, "%e  %.3f  %e  %e\n", time, T2[Nx2 / 2][0][0]);
            
          /*проверка сохранения энергии*/
           /*Esum = 0;
             for(i=0;i<Nx2-1;i++)
                for(j=0;j<Ny-1;j++)
                    for (k = 0; k < Nz - 1; k++)
                    {
                        Tav = (T2[i][j][k] + T2[i][j][k + 1] + T2[i][j + 1][k] + T2[i][j + 1][k + 1] + T2[i + 1][j][k] + T2[i + 1][j][k + 1] + T2[i + 1][j + 1][k] + T2[i + 1][j + 1][k + 1])/8;
                        Esum = Esum + int_energy(Tav) * dx2 * (y[j + 1] - y[j]) * (z[k + 1] - z[k]);
                    }*/
           // printf("%e  %e  %e  %e\n",time, A * P / f * (np + 1), 2 * Esum, 2*Eabs);
           //fprintf(f4, "%e  %e  %e  %e  %e  %e\n", time, A* P / f * (np + 1), 2 * Esum,fabs(2 * Esum - A * P / f * (np + 1)) / (A * P / f * (np + 1)));
           if (T2[Nxmax2 - 1][0][0] > T0 + 1)
                Nxmax2 = Nxmax2 + 1;
            if (T2[i00][Nymax2 - 1][0] > T0 + 1)
                Nymax2 = Nymax2 + 1;
            if (T2[i00][0][Nzmax2-1] > T0 + 1)
                Nzmax2 = Nzmax2 + 1;
            if (Nxmax2 > Nx2)
                Nxmax2 = Nx2;
            if (Nymax2 > Ny)
                Nymax2 = Ny;
            if (Nzmax2 > Nz)
                Nzmax2 = Nz;
        }
        i0 = i00 + (np + 1) * v / f / dx2;
        delta = i00 * dx2 + (np + 1) * v / f - i0 * dx2;
        if ((i0 * dx2 + 3 * r0) > Nxmax2 * dx2)
            Nxmax2 = i0 + 3 * (int)floor(r0 / dx2);
        if (Nxmax2 > Nx2)
            Nxmax2 = Nx2;

        /*перерасчет температуры на локальную сетку*/
        for (i = Nx1 / 2; i < Nx1; i++)

            for (j = 0; j < Ny; j++)
                for (k = 0; k < Nz; k++)
                {
                    ni = i0 + (i - Nx1 / 2) / 3;
                    T1[i][j][k] = T2[ni][j][k] + (dx1 * (i - Nx1 / 2) - (ni - i0) * dx2) * (T2[ni + 1][j][k] - T2[ni][j][k]) / dx2;
                    ni = i0 - (i - Nx1 / 2) / 3 - 1;
                    T1[Nx1 - i - 1][j][k] = T2[ni][j][k] + ((i0 - ni) * dx2 - (i - Nx1 / 2 + 1) * dx1) * (T2[ni + 1][j][k] - T2[ni][j][k]) / dx2;
                }
        printf("executing...  %.2f%%\r", 100 * (double)(np + 1) / Np);
        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress_bar), (double)(np + 1) / Np);

    }
    fclose(f1);
    fclose(f2);
    fclose(f3);
    fclose(f4);
    fclose(f5);
    fclose(f6);
    timeprog = clock() - timeprog;
    printf("\nProgram running time=%f s\n", (double)timeprog / CLOCKS_PER_SEC);
    update_plots(NULL, NULL);
    system("PAUSE");

}

void do_thread(){ // поток программы minimarker
    double P = strtod(gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(entry_P))),NULL); 
    double v = strtod(gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(entry_V))),NULL); 
    double f = strtod(gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(entry_F))),NULL); 
    if (P > 0 && v > 0 && f > 0){
        minimarker(v, f, P);
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
    
    GtkWidget *grid = gtk_grid_new();
    gtk_notebook_append_page (GTK_NOTEBOOK (notebook), grid, gtk_label_new("Параметры лазера"));

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
    gtk_grid_attach(GTK_GRID(grid), progress_bar, 1,5,3,2);
    
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
