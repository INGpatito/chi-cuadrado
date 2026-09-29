#ifndef POKER_H
#define POKER_H

#include "common.h"
#include "uniformidad.h"

// Enumeración para las manos de póker de 3 dígitos
typedef enum {
    POKER_TODOS_DIFERENTES = 0, // TD (P = 0.72)
    POKER_UN_PAR = 1,           // 1P (P = 0.27)
    POKER_TERCIA = 2,           // 3I (P = 0.01)
    POKER_CATEGORIAS = 3
} PokerCategoria;

// Estructura de resultados de la prueba de Póker de 3 dígitos
typedef struct {
    int n;                      // Tamaño de muestra
    int observados[3];          // Conteo Oi por categoría [TD, 1P, 3I]
    double esperados[3];        // Frecuencia esperada Ei por categoría
    double prob[3];             // Probabilidades teóricas [0.72, 0.27, 0.01]
    int grados_libertad;        // 3 - 1 = 2
    double chi_calc;            // Chi-cuadrado calculado
    double chi_crit;            // Chi-cuadrado crítico (gl=2)
    double alpha;               // Nivel de significancia (ej. 0.05)
    bool aceptada;              // true si chi_calc <= chi_crit
} PokerResult;

// Prototipos
PokerCategoria clasificar_tres_digitos(double numero, int *d1, int *d2, int *d3);
PokerResult prueba_poker_tres_digitos(const DataSet *ds, double alpha, bool imprimir_tabla);

#endif // POKER_H
