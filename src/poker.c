#include "poker.h"

// Clasifica los primeros 3 decimales de un número en [0, 1)
PokerCategoria clasificar_tres_digitos(double numero, int *d1, int *d2, int *d3) {
    if (numero < 0.0) numero = 0.0;
    if (numero >= 1.0) numero = 0.999999;

    int val = (int)(numero * 1000.0 + 1e-9);
    if (val >= 1000) val = 999;
    if (val < 0) val = 0;

    int a = (val / 100) % 10;
    int b = (val / 10) % 10;
    int c = val % 10;

    if (d1) *d1 = a;
    if (d2) *d2 = b;
    if (d3) *d3 = c;

    // Tres dígitos iguales (Tercia)
    if (a == b && b == c) {
        return POKER_TERCIA;
    }
    // Dos dígitos iguales y uno diferente (Un Par)
    if (a == b || a == c || b == c) {
        return POKER_UN_PAR;
    }
    // Los tres dígitos son distintos (Todos Diferentes)
    return POKER_TODOS_DIFERENTES;
}

PokerResult prueba_poker_tres_digitos(const DataSet *ds, double alpha, bool imprimir_tabla) {
    PokerResult res;
    res.n = ds ? ds->count : 0;
    res.grados_libertad = 2; // 3 categorías - 1
    res.alpha = alpha;
    res.chi_calc = 0.0;
    res.chi_crit = chi_cuadrado_critico(2, alpha);
    res.aceptada = false;

    // Probabilidades teóricas para 3 dígitos
    res.prob[POKER_TODOS_DIFERENTES] = 0.72; // (10 * 9 * 8) / 1000
    res.prob[POKER_UN_PAR]           = 0.27; // (comb(3,2) * 10 * 9) / 1000
    res.prob[POKER_TERCIA]           = 0.01; // 10 / 1000

    for (int i = 0; i < 3; i++) {
        res.observados[i] = 0;
        res.esperados[i] = (double)res.n * res.prob[i];
    }

    if (!ds || ds->count == 0) {
        printf("[Error] Datos insuficientes para la prueba de Póker.\n");
        return res;
    }

    // Clasificación de cada número de la muestra
    for (int i = 0; i < ds->count; i++) {
        int d1, d2, d3;
        PokerCategoria cat = clasificar_tres_digitos(ds->data[i], &d1, &d2, &d3);
        res.observados[cat]++;
    }

    // Cálculo del estadístico Chi-cuadrado
    double sum_chi = 0.0;
    for (int i = 0; i < 3; i++) {
        if (res.esperados[i] > 0.0) {
            double diff = (double)res.observados[i] - res.esperados[i];
            sum_chi += (diff * diff) / res.esperados[i];
        }
    }
    res.chi_calc = sum_chi;
    res.aceptada = (res.chi_calc <= res.chi_crit);

    if (imprimir_tabla) {
        const char *nombres[3] = {
            "Todos Diferentes (TD)",
            "Un Par           (1P)",
            "Tercia           (3I)"
        };

        printf("\n=================== PRUEBA DE PÓKER (3 DÍGITOS) ===================\n");
        printf(" Tamaño de muestra (N)    : %d\n", ds->count);
        printf(" Grados de libertad (gl)  : %d (3 categorías - 1)\n", res.grados_libertad);
        printf(" Nivel significancia (α)  : %.2f (%.0f%%)\n", alpha, (1.0 - alpha) * 100.0);
        printf("-------------------------------------------------------------------\n");
        printf(" Muestra de clasificación de los primeros números:\n");
        int muestra_poker = (ds->count < 5) ? ds->count : 5;
        for (int i = 0; i < muestra_poker; i++) {
            int d1, d2, d3;
            PokerCategoria cat = clasificar_tres_digitos(ds->data[i], &d1, &d2, &d3);
            const char *etiqueta = (cat == POKER_TODOS_DIFERENTES) ? "TD" :
                                   (cat == POKER_UN_PAR)           ? "1P" : "3I";
            printf("  R[%2d] = %7.5f -> Dígitos: [%d, %d, %d] -> %s\n",
                   i + 1, ds->data[i], d1, d2, d3, etiqueta);
        }
        printf("-------------------------------------------------------------------\n");
        printf(" %-22s |  P(E) |   Oi   |    Ei    |  (Oi-Ei)  | (Oi-Ei)^2/Ei\n", "Categoría (Mano)");
        printf("-------------------------------------------------------------------\n");

        for (int i = 0; i < 3; i++) {
            double diff = (double)res.observados[i] - res.esperados[i];
            double term = (res.esperados[i] > 0.0) ? (diff * diff) / res.esperados[i] : 0.0;
            printf(" %-22s |  %.2f |  %4d  |  %7.2f |  %+8.2f |  %-10.4f\n",
                   nombres[i], res.prob[i], res.observados[i], res.esperados[i], diff, term);
        }

        printf("-------------------------------------------------------------------\n");
        printf(" TOTAL: N = %d | χ0^2 Calculado = %.4f | χα,gl=2 Crítico = %.4f\n",
               ds->count, res.chi_calc, res.chi_crit);

        if (res.esperados[POKER_TERCIA] < 5.0) {
            printf(" [Nota]: La frecuencia esperada de Tercia (Ei = %.2f) es < 5.\n",
                   res.esperados[POKER_TERCIA]);
            printf("         (Para rigor asintótico estricto de Chi-cuadrado se sugiere N >= 500).\n");
        }
        printf("-------------------------------------------------------------------\n");
        printf(" CONCLUSIÓN:\n");
        if (res.aceptada) {
            printf("  [✓] χ0^2 (%.4f) <= χα,2 (%.4f)\n", res.chi_calc, res.chi_crit);
            printf("  -> SE ACEPTA H0: La frecuencia de dígitos es estadísticamente ALEATORIA\n");
            printf("     según la prueba de Póker de 3 dígitos al nivel de %.0f%%.\n", (1.0 - alpha) * 100.0);
        } else {
            printf("  [X] χ0^2 (%.4f) > χα,2 (%.4f)\n", res.chi_calc, res.chi_crit);
            printf("  -> SE RECHAZA H0: Los dígitos NO presentan el comportamiento aleatorio esperado.\n");
        }
        printf("===================================================================\n\n");
    }

    return res;
}
