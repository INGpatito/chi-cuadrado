#include "uniformidad.h"

// Tablas de valores críticos de Chi-cuadrado para gl = 1 hasta 30
static const double chi_005[30] = {
     3.841,  5.991,  7.815,  9.488, 11.070, 12.592, 14.067, 15.507, 16.919, 18.307,
    19.675, 21.026, 22.362, 23.685, 24.996, 26.296, 27.587, 28.869, 30.144, 31.410,
    32.671, 33.924, 35.172, 36.415, 37.652, 38.885, 40.113, 41.337, 42.557, 43.773
};

static const double chi_001[30] = {
     6.635,  9.210, 11.345, 13.277, 15.086, 16.812, 18.475, 20.090, 21.666, 23.209,
    24.725, 26.217, 27.688, 29.141, 30.578, 32.000, 33.409, 34.805, 36.191, 37.566,
    38.932, 40.289, 41.638, 42.980, 44.314, 45.642, 46.963, 48.278, 49.588, 50.892
};

static const double chi_010[30] = {
     2.706,  4.605,  6.251,  7.779,  9.236, 10.645, 12.017, 13.362, 14.684, 15.987,
    17.275, 18.549, 19.812, 21.064, 22.307, 23.542, 24.769, 25.989, 27.204, 28.412,
    29.615, 30.813, 32.007, 33.196, 34.382, 35.563, 36.741, 37.916, 39.087, 40.256
};

// Retorna el valor crítico para gl grados de libertad y alpha (0.05 por defecto)
double chi_cuadrado_critico(int gl, double alpha) {
    if (gl <= 0) return 0.0;

    if (gl <= 30) {
        if (fabs(alpha - 0.01) < 0.005) {
            return chi_001[gl - 1];
        } else if (fabs(alpha - 0.10) < 0.005) {
            return chi_010[gl - 1];
        } else {
            // 0.05 por defecto
            return chi_005[gl - 1];
        }
    }

    // Para gl > 30: aproximación de Wilson-Hilferty
    double z = 1.644853; // alpha = 0.05
    if (fabs(alpha - 0.01) < 0.005) z = 2.326348;
    else if (fabs(alpha - 0.10) < 0.005) z = 1.281552;

    double factor = 1.0 - (2.0 / (9.0 * gl)) + z * sqrt(2.0 / (9.0 * gl));
    return gl * factor * factor * factor;
}

ChiResult prueba_chi_cuadrado(const DataSet *ds, int k, double alpha, bool imprimir_tabla) {
    ChiResult res;
    res.k = k;
    res.grados_libertad = k - 1;
    res.alpha = alpha;
    res.chi_calc = 0.0;
    res.chi_crit = chi_cuadrado_critico(res.grados_libertad, alpha);
    res.aceptada = false;

    if (!ds || ds->count == 0 || k <= 1) {
        printf("[Error] Datos insuficientes o número de intervalos inválido.\n");
        return res;
    }

    int *observed = (int *)calloc(k, sizeof(int));
    if (!observed) {
        printf("[Error] Fallo al asignar memoria.\n");
        return res;
    }

    // Conteo de frecuencias observadas
    for (int i = 0; i < ds->count; i++) {
        int bin = (int)(ds->data[i] * k);
        if (bin >= k) bin = k - 1;
        if (bin < 0) bin = 0;
        observed[bin]++;
    }

    double expected = (double)ds->count / (double)k;
    double sum_chi = 0.0;

    if (imprimir_tabla) {
        printf("\n==================== PRUEBA DE CHI-CUADRADO ====================\n");
        printf(" Tamaño de muestra (N)    : %d\n", ds->count);
        printf(" Número de intervalos (k) : %d\n", k);
        printf(" Grados de libertad (k-1) : %d\n", res.grados_libertad);
        printf(" Nivel significancia (α)  : %.2f (%.0f%%)\n", alpha, (1.0 - alpha) * 100.0);
        printf(" Frecuencia esperada (Ei) : %.4f\n", expected);
        printf("----------------------------------------------------------------\n");
        printf(" %-4s | %-15s | %-6s | %-8s | %-8s | %-12s\n",
               "No", "Intervalo", "Oi", "Ei", "(Oi-Ei)", "(Oi-Ei)^2/Ei");
        printf("----------------------------------------------------------------\n");
    }

    for (int i = 0; i < k; i++) {
        double lim_inf = (double)i / k;
        double lim_sup = (double)(i + 1) / k;
        double diff = (double)observed[i] - expected;
        double term = (diff * diff) / expected;
        sum_chi += term;

        if (imprimir_tabla) {
            printf(" %-4d | [%.3f - %.3f) | %-6d | %-8.2f | %-+8.2f | %-12.4f\n",
                   i + 1, lim_inf, lim_sup, observed[i], expected, diff, term);
        }
    }

    res.chi_calc = sum_chi;
    res.aceptada = (res.chi_calc <= res.chi_crit);

    if (imprimir_tabla) {
        printf("----------------------------------------------------------------\n");
        printf(" TOTAL Oi = %d | Chi-cuadrado Calculado (χ0^2) = %.4f\n", ds->count, res.chi_calc);
        printf(" Chi-cuadrado Crítico  (χα,k-1)        = %.4f\n", res.chi_crit);
        printf("----------------------------------------------------------------\n");
        printf(" CONCLUSIÓN:\n");
        if (res.aceptada) {
            printf("  [✓] χ0^2 (%.4f) <= χα,k-1 (%.4f)\n", res.chi_calc, res.chi_crit);
            printf("  -> SE ACEPTA H0: No hay evidencia para rechazar que los números\n");
            printf("     sigan una distribución UNIFORME U(0,1) al nivel de %.0f%%.\n", (1.0 - alpha) * 100.0);
        } else {
            printf("  [X] χ0^2 (%.4f) > χα,k-1 (%.4f)\n", res.chi_calc, res.chi_crit);
            printf("  -> SE RECHAZA H0: Los números NO se distribuyen uniformemente.\n");
        }
        printf("================================================================\n\n");
    }

    free(observed);
    return res;
}

// Tablas críticas de Kolmogorov-Smirnov para N = 1 a 35
static const double ks_005[35] = {
    0.97500, 0.84189, 0.70760, 0.62394, 0.56328,
    0.51926, 0.48342, 0.45427, 0.43001, 0.40925,
    0.39122, 0.37543, 0.36143, 0.34890, 0.33760,
    0.32733, 0.31796, 0.30936, 0.30143, 0.29408,
    0.28724, 0.28087, 0.27490, 0.26931, 0.26404,
    0.25907, 0.25438, 0.24993, 0.24571, 0.24170,
    0.23788, 0.23424, 0.23076, 0.22743, 0.22425
};

static const double ks_001[35] = {
    0.99500, 0.92929, 0.82800, 0.73324, 0.66853,
    0.61661, 0.57581, 0.54254, 0.51478, 0.49000,
    0.46800, 0.44900, 0.43200, 0.41800, 0.40400,
    0.39200, 0.38100, 0.37100, 0.36100, 0.35200,
    0.34400, 0.33700, 0.33000, 0.32300, 0.31700,
    0.31100, 0.30500, 0.30000, 0.29500, 0.29000,
    0.28500, 0.28100, 0.27700, 0.27300, 0.26900
};

static const double ks_010[35] = {
    0.95000, 0.77639, 0.64200, 0.56400, 0.51000,
    0.47000, 0.43800, 0.41100, 0.38800, 0.36800,
    0.35200, 0.33800, 0.32500, 0.31400, 0.30400,
    0.29500, 0.28600, 0.27800, 0.27200, 0.26400,
    0.25800, 0.25300, 0.24700, 0.24200, 0.23800,
    0.23300, 0.22900, 0.22500, 0.22100, 0.21800,
    0.21400, 0.21100, 0.20800, 0.20500, 0.20200
};

// Retorna el valor crítico D(alpha, N) para Kolmogorov-Smirnov
double ks_critico(int n, double alpha) {
    if (n <= 0) return 0.0;

    if (n <= 35) {
        if (fabs(alpha - 0.01) < 0.005) {
            return ks_001[n - 1];
        } else if (fabs(alpha - 0.10) < 0.005) {
            return ks_010[n - 1];
        } else {
            return ks_005[n - 1];
        }
    }

    // Para N > 35: fórmulas asintóticas estándar
    if (fabs(alpha - 0.01) < 0.005) {
        return 1.63 / sqrt((double)n);
    } else if (fabs(alpha - 0.10) < 0.005) {
        return 1.22 / sqrt((double)n);
    } else {
        // alpha = 0.05
        return 1.36 / sqrt((double)n);
    }
}

// Función comparadora para ordenar números flotantes
static int compare_doubles(const void *a, const void *b) {
    double da = *(const double *)a;
    double db = *(const double *)b;
    if (da < db) return -1;
    if (da > db) return 1;
    return 0;
}

KSResult prueba_kolmogorov_smirnov(const DataSet *ds, double alpha, bool imprimir_tabla) {
    KSResult res;
    res.n = ds ? ds->count : 0;
    res.d_mas = 0.0;
    res.d_menos = 0.0;
    res.d_calc = 0.0;
    res.d_crit = ks_critico(res.n, alpha);
    res.alpha = alpha;
    res.aceptada = false;

    if (!ds || ds->count == 0) {
        printf("[Error] Conjunto de datos vacío para Kolmogorov-Smirnov.\n");
        return res;
    }

    int n = ds->count;
    // Copiamos los datos para no alterar el orden original (necesario para autocorrelación)
    double *sorted = (double *)malloc(n * sizeof(double));
    if (!sorted) {
        printf("[Error] Memoria insuficiente.\n");
        return res;
    }
    for (int i = 0; i < n; i++) {
        sorted[i] = ds->data[i];
    }
    qsort(sorted, n, sizeof(double), compare_doubles);

    double max_d_plus = 0.0;
    double max_d_minus = 0.0;
    int idx_max_plus = 0;
    int idx_max_minus = 0;

    for (int i = 0; i < n; i++) {
        double r_i = sorted[i];
        double i_over_n = (double)(i + 1) / n;
        double i_minus_1_over_n = (double)i / n;

        double d_plus = i_over_n - r_i;
        double d_minus = r_i - i_minus_1_over_n;

        if (d_plus > max_d_plus) {
            max_d_plus = d_plus;
            idx_max_plus = i;
        }
        if (d_minus > max_d_minus) {
            max_d_minus = d_minus;
            idx_max_minus = i;
        }
    }

    res.d_mas = max_d_plus;
    res.d_menos = max_d_minus;
    res.d_calc = (max_d_plus > max_d_minus) ? max_d_plus : max_d_minus;
    res.aceptada = (res.d_calc <= res.d_crit);

    if (imprimir_tabla) {
        printf("\n================ PRUEBA DE KOLMOGOROV-SMIRNOV ================\n");
        printf(" Tamaño de muestra (N)    : %d\n", n);
        printf(" Nivel significancia (α)  : %.2f (%.0f%%)\n", alpha, (1.0 - alpha) * 100.0);
        printf(" Valor crítico D(α, N)    : %.5f\n", res.d_crit);
        printf("--------------------------------------------------------------\n");
        printf("  i   |   R(i)    |   i/N     | (i-1)/N   |    D+     |    D-   \n");
        printf("--------------------------------------------------------------\n");

        if (n <= 30) {
            for (int i = 0; i < n; i++) {
                double r_i = sorted[i];
                double i_over_n = (double)(i + 1) / n;
                double i_minus_1_over_n = (double)i / n;
                double dp = i_over_n - r_i;
                double dm = r_i - i_minus_1_over_n;
                printf(" %-4d |  %8.5f |  %8.5f |  %8.5f |  %+8.5f |  %+8.5f\n",
                       i + 1, r_i, i_over_n, i_minus_1_over_n, dp, dm);
            }
        } else {
            // Para muestras grandes mostramos primeros 5, puntos críticos y últimos 5
            for (int i = 0; i < 5; i++) {
                double r_i = sorted[i];
                printf(" %-4d |  %8.5f |  %8.5f |  %8.5f |  %+8.5f |  %+8.5f\n",
                       i + 1, r_i, (double)(i + 1)/n, (double)i/n,
                       (double)(i + 1)/n - r_i, r_i - (double)i/n);
            }
            printf("  ... |    ...    |    ...    |    ...    |    ...    |    ...   \n");
            // Mostrar filas donde se alcanzaron los máximos si no están ya en los extremos
            if (idx_max_plus >= 5 && idx_max_plus < n - 5) {
                int i = idx_max_plus;
                double r_i = sorted[i];
                printf(" %-4d |  %8.5f |  %8.5f |  %8.5f |  %+8.5f*|  %+8.5f  (<- Max D+)\n",
                       i + 1, r_i, (double)(i + 1)/n, (double)i/n,
                       (double)(i + 1)/n - r_i, r_i - (double)i/n);
            }
            if (idx_max_minus >= 5 && idx_max_minus < n - 5 && idx_max_minus != idx_max_plus) {
                int i = idx_max_minus;
                double r_i = sorted[i];
                printf(" %-4d |  %8.5f |  %8.5f |  %8.5f |  %+8.5f |  %+8.5f* (<- Max D-)\n",
                       i + 1, r_i, (double)(i + 1)/n, (double)i/n,
                       (double)(i + 1)/n - r_i, r_i - (double)i/n);
            }
            printf("  ... |    ...    |    ...    |    ...    |    ...    |    ...   \n");
            for (int i = n - 5; i < n; i++) {
                double r_i = sorted[i];
                printf(" %-4d |  %8.5f |  %8.5f |  %8.5f |  %+8.5f |  %+8.5f\n",
                       i + 1, r_i, (double)(i + 1)/n, (double)i/n,
                       (double)(i + 1)/n - r_i, r_i - (double)i/n);
            }
        }

        printf("--------------------------------------------------------------\n");
        printf(" D+ máximo  = %.5f  (en i = %d)\n", max_d_plus, idx_max_plus + 1);
        printf(" D- máximo  = %.5f  (en i = %d)\n", max_d_minus, idx_max_minus + 1);
        printf(" Estadístico D calculado = max(D+, D-) = %.5f\n", res.d_calc);
        printf(" Valor crítico D(α, N)                 = %.5f\n", res.d_crit);
        printf("--------------------------------------------------------------\n");
        printf(" CONCLUSIÓN:\n");
        if (res.aceptada) {
            printf("  [✓] D (%.5f) <= D_crit (%.5f)\n", res.d_calc, res.d_crit);
            printf("  -> SE ACEPTA H0: No se puede rechazar que los números sigan\n");
            printf("     una distribución UNIFORME U(0,1) al nivel de %.0f%%.\n", (1.0 - alpha) * 100.0);
        } else {
            printf("  [X] D (%.5f) > D_crit (%.5f)\n", res.d_calc, res.d_crit);
            printf("  -> SE RECHAZA H0: Los números NO provienen de una distribución uniforme.\n");
        }
        printf("==============================================================\n\n");
    }

    free(sorted);
    return res;
}
