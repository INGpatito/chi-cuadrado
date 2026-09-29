#include "common.h"

void dataset_init(DataSet *ds) {
    if (ds) {
        ds->count = 0;
    }
}

bool dataset_add(DataSet *ds, double val) {
    if (!ds || ds->count >= MAX_NUMBERS) {
        return false;
    }
    ds->data[ds->count++] = val;
    return true;
}

void dataset_print(const DataSet *ds, int per_line) {
    if (!ds || ds->count == 0) {
        printf("[Aviso] No hay números en el conjunto de datos.\n");
        return;
    }
    printf("\n=== Conjunto de Datos (%d números) ===\n", ds->count);
    for (int i = 0; i < ds->count; i++) {
        printf("%8.5f ", ds->data[i]);
        if ((i + 1) % per_line == 0) {
            printf("\n");
        }
    }
    if (ds->count % per_line != 0) {
        printf("\n");
    }
    printf("=========================================\n\n");
}

// Generador Congruencial Lineal: X_{n+1} = (a * X_n + c) mod m
void dataset_generate_lcg(DataSet *ds, int n, unsigned long seed, unsigned long a, unsigned long c, unsigned long m) {
    dataset_init(ds);
    if (n > MAX_NUMBERS) n = MAX_NUMBERS;
    if (m == 0) return;

    unsigned long current = seed;
    for (int i = 0; i < n; i++) {
        current = (a * current + c) % m;
        double r = (double)current / (double)m;
        dataset_add(ds, r);
    }
}

// Generador estándar con rand()
void dataset_generate_rand(DataSet *ds, int n, unsigned int seed) {
    dataset_init(ds);
    if (n > MAX_NUMBERS) n = MAX_NUMBERS;

    srand(seed);
    for (int i = 0; i < n; i++) {
        double r = (double)rand() / ((double)RAND_MAX + 1.0);
        dataset_add(ds, r);
    }
}

// Carga números desde archivo (separados por espacios, comas o saltos de línea)
bool dataset_load_from_file(DataSet *ds, const char *filepath) {
    FILE *f = fopen(filepath, "r");
    if (!f) {
        return false;
    }

    dataset_init(ds);
    double val;
    while (ds->count < MAX_NUMBERS && fscanf(f, "%lf", &val) == 1) {
        dataset_add(ds, val);
    }

    fclose(f);
    return true;
}
