#ifndef INDEPENDENCIA_H
#define INDEPENDENCIA_H

#include "common.h"

// Estructura para almacenar el resultado de la prueba de Autocorrelación
typedef struct {
    int i;                  // Posición inicial (1-indexada para el usuario)
    int l;                  // Salto o retardo (lag)
    int n;                  // Tamaño de la muestra
    int m;                  // Parámetro M de la prueba
    int total_pares;        // M + 1 (cantidad de pares evaluados)
    double rho_estimado;    // Estimador de autocorrelación (\hat{\rho})
    double sigma_rho;       // Desviación estándar (\sigma_{\rho})
    double z_calc;          // Estadístico Z0 calculado
    double z_crit;          // Valor crítico Z_{\alpha/2}
    double alpha;           // Nivel de significancia
    bool aceptada;          // true si |z_calc| <= z_crit
} AutoCorrResult;

// Prototipos
double normal_critico_bilateral(double alpha);
AutoCorrResult prueba_autocorrelacion(const DataSet *ds, int i_inicio, int lag, double alpha, bool imprimir_detalle);

#endif // INDEPENDENCIA_H
