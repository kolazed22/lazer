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
#include <regex.h>
#include "cJSON.h"

// Путь к каталогу материалов
#define DIRECTORY_PATH_MATERIALS "materials"
// Путь к временным файлам
#define DIR_TEMP "tmp"

// многопоточность
GMutex mut;
GThread *thread;

// виджеты ввода
GtkWidget *entry_P;
GtkWidget *entry_V;
GtkWidget *entry_F;
GtkWidget *entry_tp;
GtkWidget *entry_r0;
GtkWidget *entry_Nx1;
GtkWidget *entry_Ny;
GtkWidget *entry_Nz;
GtkWidget *entry_Nx2;
GtkWidget *entry_Ntr;

// виджет полоски прогресса
GtkWidget *progress_bar;

// виджеты изображений для графиков
GtkWidget *image_heatmapTxy, *image_heatmapTxz, *image_heatmapTyz, *image_plotTt;
GtkWidget *image_plot_cond, *image_plot_cap, *image_plot_abs;

// виджеты для интерфейса
GtkWidget *dropdown;
GtkWidget *notebook;
GtkWidget *window;

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

typedef struct {
    double v;
    double f; 
    double P;
    double tp;
    double r0;
} LaserParams;

typedef struct {
    char name[64];
    double A;
    double m0;
    double Tb;
    double Lev;
    double pol;
    char conductivity_image[256];
    char capacity_image[256];
    char absorption_image[256];
    char conductivity_data[256];
    char capacity_data[256];
    char absorption_data[256];
} Material;

typedef struct {
    size_t size; // Количество материалов
    Material *materials; // Массив материалов
    GtkStringList *material_names; // Список названий материалов
} MaterialDatabase;

// database всех материалов
MaterialDatabase material_database;
Material *selected_material;
///////////// widgets.c /////////////
// Функции инициализации виджетов
void init_widgets(GtkApplication *app);
void init_parameters_tab(GtkWidget *notebook);
void init_area_tab(GtkWidget *notebook);
void init_plots_tab(GtkWidget *notebook);

// Функции для обработки событий интерфейса и основной логики
static void on_dropdown_selection_changed(GtkDropDown *dropdown, GParamSpec *pspec, gpointer user_data);
bool check_input_parameters(LaserParams params);
void do_thread();
void click_start_button(GtkWidget *widget, gpointer data);
static void activate(GtkApplication *app, gpointer user_data);

///////////// materials.c /////////////
MaterialDatabase load_material_database(const char *directory_path);

///////////// plots.c /////////////
// Функции для работы с данными и построения графиков
void freeData_TC(Data_TC* data);
Data_TC fread_TC(const char *file_name);
void freeData_Tt(Data_Tt* data);
Data_Tt fread_Tt(const char *file_name);
void freeData_Txx(Data_Txx* data);
Data_Txx fread_Txx(const char *file_name);
Data_Txx shape_Txx(Data_Txx *data);

void draw_plot_Tt(Data_Tt data, const char *file_write_name);
void draw_plot_TC(Data_TC data, const char *file_write_name);
void draw_heatmap(Data_Txx data, int width, int height, const char *write_file_name);
void update_plots(GtkWidget *widget, gpointer data);

///////////// minimarker.c /////////////
// Функции для моделирования лазерного воздействия
int minimarker(LaserParams params, Material *material,  GtkWidget *progress_bar);

#endif