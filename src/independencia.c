#include "independencia.h"

// Retorna el valor crítico Z_{\alpha/2} para una prueba bilateral
double normal_critico_bilateral(double alpha) {
    if (fabs(alpha - 0.05) < 0.005) return 1.95996; // 95% de confianza (común: 1.96)
    if (fabs(alpha - 0.01) < 0.005) return 2.57583; // 99% de confianza (común: 2.58)
    if (fabs(alpha - 0.10) < 0.005) return 1.64485; // 90% de confianza (común: 1.645)
    if (fabs(alpha - 0.02) < 0.005) return 2.32635; // 98% de confianza

    // Aproximación racional de Hastings para el inverso de la distribución normal estándar
    double p = 1.0 - (alpha / 2.0);
    double t = sqrt(-2.0 * log(1.0 - p));
    double c0 = 2.515517, c1 = 0.802853, c2 = 0.010328;
    double d1 = 1.432788, d2 = 0.189269, d3 = 0.001308;
    double z = t - ((c0 + c1 * t + c2 * t * t) / (1.0 + d1 * t + d2 * t * t + d3 * t * t * t));
    return z;
}

AutoCorrResult prueba_autocorrelacion(const DataSet *ds, int i_inicio, int lag, double alpha, bool imprimir_detalle) {
    AutoCorrResult res;
    res.i = i_inicio;
    res.l = lag;
    res.n = ds ? ds->count : 0;
    res.m = -1;
    res.total_pares = 0;
    res.rho_estimado = 0.0;
    res.sigma_rho = 0.0;
    res.z_calc = 0.0;
    res.alpha = alpha;
    res.z_crit = normal_critico_bilateral(alpha);
    res.aceptada = false;

    if (!ds || ds->count < 2) {
        printf("[Error] Tamaño de muestra insuficiente para prueba de autocorrelación.\n");
        return res;
    }
    if (i_inicio < 1 || lag < 1) {
        printf("[Error] Posición inicial (i >= 1) y retardo (lag >= 1) deben ser positivos.\n");
        return res;
    }
    if (i_inicio + lag > ds->count) {
        printf("[Error] El salto i + l (%d + %d = %d) excede el tamaño N (%d).\n",
               i_inicio, lag, i_inicio + lag, ds->count);
        return res;
    }

    int n = ds->count;
    // M es el entero más grande tal que i + (M+1)l <= N
    // M = floor((N - i - l) / l)
    int m = (n - i_inicio - lag) / lag;
    res.m = m;
    res.total_pares = m + 1;

    if (m < 0) {
        printf("[Error] No es posible formar al menos un par con los parámetros dados.\n");
        return res;
    }

    double sum_prod = 0.0;
    int start_idx = i_inicio - 1; // Conversión a 0-indexado en C

    if (imprimir_detalle) {
        printf("\n================ PRUEBA DE AUTOCORRELACIÓN ================\n");
        printf(" Tamaño de muestra (N)    : %d\n", n);
        printf(" Posición inicial (i)     : %d\n", i_inicio);
        printf(" Salto / retardo (l)      : %d\n", lag);
        printf(" Parámetro M              : %d\n", m);
        printf(" Cantidad de pares (M+1)  : %d\n", m + 1);
        printf(" Nivel significancia (α)  : %.2f (%.0f%% bilateral)\n", alpha, (1.0 - alpha) * 100.0);
        printf(" Valor crítico |Z_crit|   : %.4f\n", res.z_crit);
        printf("-----------------------------------------------------------\n");
        printf(" Muestra de productos evaluados:\n");
    }

    int max_mostrar = (m + 1 <= 10) ? (m + 1) : 5;
    for (int k = 0; k <= m; k++) {
        int idx1 = start_idx + k * lag;
        int idx2 = start_idx + (k + 1) * lag;
        double prod = ds->data[idx1] * ds->data[idx2];
        sum_prod += prod;

        if (imprimir_detalle && k < max_mostrar) {
            printf("  k = %2d: R[%3d] * R[%3d] = %7.5f * %7.5f = %8.6f\n",
                   k, idx1 + 1, idx2 + 1, ds->data[idx1], ds->data[idx2], prod);
        }
    }

    if (imprimir_detalle && m + 1 > 10) {
        printf("  ... (se omiten %d pares intermedios) ...\n", (m + 1) - max_mostrar - 2);
        for (int k = m - 1; k <= m; k++) {
            int idx1 = start_idx + k * lag;
            int idx2 = start_idx + (k + 1) * lag;
            double prod = ds->data[idx1] * ds->data[idx2];
            printf("  k = %2d: R[%3d] * R[%3d] = %7.5f * %7.5f = %8.6f\n",
                   k, idx1 + 1, idx2 + 1, ds->data[idx1], ds->data[idx2], prod);
        }
    }

    // Cálculo del estimador rho_hat: (1 / (M + 1)) * SUM - 0.25
    res.rho_estimado = (sum_prod / (double)(m + 1)) - 0.25;

    // Desviación estándar: sqrt(13M + 7) / (12 * (M + 1))
    res.sigma_rho = sqrt(13.0 * (double)m + 7.0) / (12.0 * ((double)m + 1.0));

    // Estadístico Z0: rho_estimado / sigma_rho
    res.z_calc = res.rho_estimado / res.sigma_rho;

    // Criterio bilateral: |Z0| <= Z_crit
    res.aceptada = (fabs(res.z_calc) <= res.z_crit);

    if (imprimir_detalle) {
        printf("-----------------------------------------------------------\n");
        printf(" Sumatoria de productos           = %.6f\n", sum_prod);
        printf(" Estimador de autocorrelación (ρ̂) = %.6f\n", res.rho_estimado);
        printf(" Desviación estándar (σ_ρ)        = %.6f\n", res.sigma_rho);
        printf(" Estadístico Z calculado (Z0)     = %+.4f  (|Z0| = %.4f)\n", res.z_calc, fabs(res.z_calc));
        printf(" Valor crítico normal (Z_α/2)     = %.4f\n", res.z_crit);
        printf(" Intervalo de aceptación [-Z, +Z] = [%.4f, %+.4f]\n", -res.z_crit, res.z_crit);
        printf("-----------------------------------------------------------\n");
        printf(" CONCLUSIÓN:\n");
        if (res.aceptada) {
            printf("  [✓] |Z0| (%.4f) <= Z_crit (%.4f)\n", fabs(res.z_calc), res.z_crit);
            printf("  -> SE ACEPTA H0: No se detecta autocorrelación significativa.\n");
            printf("     Los números son INDEPENDIENTES al nivel de %.0f%%.\n", (1.0 - alpha) * 100.0);
        } else {
            printf("  [X] |Z0| (%.4f) > Z_crit (%.4f)\n", fabs(res.z_calc), res.z_crit);
            printf("  -> SE RECHAZA H0: Existe dependencia entre los números (no son independientes).\n");
        }
        printf("===========================================================\n\n");
    }

    return res;
}
