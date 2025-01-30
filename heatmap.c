#include <cairo.h>
#include <math.h>

// Функция для интерполяции цвета
void interpolate_color(double value, double *r, double *g, double *b) {
    // Пример интерполяции цвета от синего к красному
    *r = value;
    *g = 0;
    *b = 1.0 - value;
}

int main(int argc, char *argv[]) {
    cairo_surface_t *surface;
    cairo_t *cr;

    int width = 500;
    int height = 500;
    int grid_size = 1;

    surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    cr = cairo_create(surface);

    // Заполняем фон белым цветом
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);

    // Рисуем heatmap
    for (int y = 0; y < height; y += grid_size) {
        for (int x = 0; x < width; x += grid_size) {
            // Пример значения для heatmap (можно заменить на свои данные)
            double value = 1/sqrt(pow(x-width/2, 2) + pow(y-height/2, 2))*100;

            // Интерполируем цвет
            double r, g, b;
            interpolate_color(value, &r, &g, &b);

            // Устанавливаем цвет
            cairo_set_source_rgb(cr, r, g, b);

            // Рисуем прямоугольник
            cairo_rectangle(cr, x, y, grid_size, grid_size);
            cairo_fill(cr);
        }
    }

    // Сохраняем изображение в файл
    cairo_surface_write_to_png(surface, "heatmap.png");

    // Освобождаем ресурсы
    cairo_destroy(cr);
    cairo_surface_destroy(surface);

    return 0;
}
