#ifndef MAIN_H
#define MAIN_H

#include <gtk/gtk.h>
#include <cairo.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include <kplot.h>
#include <dirent.h>

// Путь к каталогу материалов
const char *directory_path_materials;
// Путь к временным файлам
#define DIR_TEMP "tmp"

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
GtkWidget *image_plot_a, *image_plot_C;

typedef struct {
    double* C; 
    double* T; // температура
    size_t size;
} Data_TC;

typedef struct {
    double* t; // время
    double* T; // температура
    size_t size;
} Data_Tt;

void freeData_Tt(Data_Tt* data);
Data_Tt fread_Tt(const char *file_name);

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

void freeData_TC(Data_TC* data);
Data_TC fread_TC(const char *file_name);
void draw_plot_TC(Data_TC data, const char *file_write_name);
void freeData_Txx(Data_Txx* data);
Data_Txx fread_Txx(const char *file_name);
Data_Txx shape_Txx(Data_Txx *data);
Data_Tt fread_Tt(const char *file_name);
void interpolate_color(double value, double min, double max, double *r, double *g, double *b);
void draw_plot_Tt(Data_Tt data, const char *file_write_name);
double dist(double x1, double y1, double x2, double y2);
double find_nearest_points(Data_Txx data, double x, double y);
void draw_heatmap(Data_Txx data, int width, int height, const char *write_file_name);
void update_plots(GtkWidget *widget, gpointer data);

double hcond(double T);
double hcap(double T);
double int_energy(double T);
int minimarker(double v, double f, double P, GtkWidget *progress_bar);

static void on_dropdown_selection_changed(GtkDropDown *dropdown, GParamSpec *pspec, gpointer user_data);
static GtkStringList *get_materials_from_directory(const char *directory_path);
void do_thread();
void click_start_button(GtkWidget *widget, gpointer data);
static void activate(GtkApplication *app, gpointer user_data);

#endif