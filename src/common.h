#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#define MAX_NUMBERS 10000

// Estructura para almacenar la secuencia de números pseudoaleatorios
typedef struct {
    double data[MAX_NUMBERS];
    int count;
} DataSet;

// Utilidades para manejo de datos
void dataset_init(DataSet *ds);
bool dataset_add(DataSet *ds, double val);
void dataset_print(const DataSet *ds, int per_line);

// Generación y lectura
void dataset_generate_lcg(DataSet *ds, int n, unsigned long seed, unsigned long a, unsigned long c, unsigned long m);
void dataset_generate_rand(DataSet *ds, int n, unsigned int seed);
bool dataset_load_from_file(DataSet *ds, const char *filepath);

#endif // COMMON_H
