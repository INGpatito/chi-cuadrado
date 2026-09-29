#ifndef MONTECARLO_H
#define MONTECARLO_H

#include "common.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Estructura para el resultado de la estimación de Pi por Monte Carlo
typedef struct {
    int total_puntos;           // N / 2 pares (x, y)
    int puntos_adentro;         // Puntos que cumplen x^2 + y^2 <= 1
    int puntos_afuera;          // Puntos fuera del cuadrante circular
    double pi_estimado;         // 4 * (adentro / total)
    double pi_real;             // M_PI
    double error_absoluto;      // |pi_estimado - pi_real|
    double error_porcentual;    // (|pi_estimado - pi_real| / pi_real) * 100
    double error_estandar;      // 4 * sqrt(p*(1-p)/N)
    double ic_inferior;         // Límite inferior IC 95%
    double ic_superior;         // Límite superior IC 95%
    bool dentro_ic;             // true si pi_real está dentro del IC 95%
} MonteCarloResult;

// Prototipos
MonteCarloResult montecarlo_estimar_pi_dataset(const DataSet *ds, bool imprimir_detalle);
MonteCarloResult montecarlo_estimar_pi_simulacion(int num_puntos, unsigned int seed, bool imprimir_progreso);

#endif // MONTECARLO_H
