typedef struct {
    double* x; // x
    double* y; // y
    double* T; // температура
    size_t size_x;
    size_t size_y;
} Data_Txx;

///////////////////////// Доделать //// Тут T двумерный массив
void freeData_Txx(Data_Txx* data) { // освобождение памяти
    free(data->x);
    free(data->y);
    free(data->T);
    data->x = NULL;
    data->y = NULL;
    data->T = NULL;
    data->size_x = 0;
    data->size_y = 0;
}

Data_Txx fread_Txx(const char *file_name){
    FILE* file = fopen(file_name, "r");
    if (file == NULL) {
        perror("fread_Tt: Failed to open file");
        exit(EXIT_FAILURE);
    }

    Data_Txx data;
    data.size_x = 0;
    data.size_y = 0;
    data.x = NULL;
    data.y = NULL;
    data.T = NULL;

    size_t capacity = 10; // Начальная емкость массива
    data.x = (double*)malloc(capacity * sizeof(double));
    data.y = (double*)malloc(capacity * sizeof(double));
    data.T = (double*)malloc(capacity * sizeof(double));

    if (data.x == NULL || data.y == NULL || data.T == NULL) {
        perror("fread_Txx: Failed to allocate memory");
        exit(EXIT_FAILURE);
    }

    double val1, val2, val3;
    while (fscanf(file, "%le %le %lf", &val1, &val2, &val3) == 3) {
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
    }
    
    fclose(file);
    return data;
}
