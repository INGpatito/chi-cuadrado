#ifndef UNIFORMIDAD_H
#define UNIFORMIDAD_H

#include "common.h"

// Estructura para el resultado de la prueba de Chi-Cuadrado
typedef struct {
    int k;                      // Número de intervalos
    int grados_libertad;        // k - 1
    double chi_calc;            // Estadístico chi-cuadrado calculado
    double chi_crit;            // Valor crítico de la tabla
    double alpha;               // Nivel de significancia (ej. 0.05)
    bool aceptada;              // true si chi_calc <= chi_crit (se acepta H0)
} ChiResult;

// Estructura para el resultado de la prueba de Kolmogorov-Smirnov
typedef struct {
    int n;                      // Tamaño de muestra
    double d_mas;               // D+
    double d_menos;             // D-
    double d_calc;              // D = max(D+, D-)
    double d_crit;              // Valor crítico de la tabla
    double alpha;               // Nivel de significancia (ej. 0.05)
    bool aceptada;              // true si d_calc <= d_crit (se acepta H0)
} KSResult;

// Prototipos para la prueba de Chi-Cuadrado
double chi_cuadrado_critico(int gl, double alpha);
ChiResult prueba_chi_cuadrado(const DataSet *ds, int k, double alpha, bool imprimir_tabla);

// Prototipos para la prueba de Kolmogorov-Smirnov
double ks_critico(int n, double alpha);
KSResult prueba_kolmogorov_smirnov(const DataSet *ds, double alpha, bool imprimir_tabla);

#endif // UNIFORMIDAD_H
