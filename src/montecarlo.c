#include "montecarlo.h"

// Función auxiliar para calcular métricas a partir del conteo de puntos
static MonteCarloResult procesar_metricas(int total_puntos, int puntos_adentro) {
    MonteCarloResult res;
    res.total_puntos = total_puntos;
    res.puntos_adentro = puntos_adentro;
    res.puntos_afuera = total_puntos - puntos_adentro;
    res.pi_real = M_PI;

    if (total_puntos <= 0) {
        res.pi_estimado = 0.0;
        res.error_absoluto = 0.0;
        res.error_porcentual = 0.0;
        res.error_estandar = 0.0;
        res.ic_inferior = 0.0;
        res.ic_superior = 0.0;
        res.dentro_ic = false;
        return res;
    }

    double p_hat = (double)puntos_adentro / (double)total_puntos;
    res.pi_estimado = 4.0 * p_hat;
    res.error_absoluto = fabs(res.pi_estimado - res.pi_real);
    res.error_porcentual = (res.error_absoluto / res.pi_real) * 100.0;

    // Error estándar: 4 * sqrt(p*(1-p)/N)
    double var_p = (p_hat * (1.0 - p_hat)) / (double)total_puntos;
    res.error_estandar = 4.0 * sqrt(var_p);

    // Intervalo de confianza al 95% (Z = 1.95996)
    double z = 1.95996;
    res.ic_inferior = res.pi_estimado - z * res.error_estandar;
    res.ic_superior = res.pi_estimado + z * res.error_estandar;
    res.dentro_ic = (res.pi_real >= res.ic_inferior && res.pi_real <= res.ic_superior);

    return res;
}

// Estima Pi utilizando los números ya generados en el DataSet tomados en parejas (x, y)
MonteCarloResult montecarlo_estimar_pi_dataset(const DataSet *ds, bool imprimir_detalle) {
    MonteCarloResult res;
    if (!ds || ds->count < 2) {
        printf("[Error] Se necesitan al menos 2 números en el DataSet para formar coordenadas (x, y).\n");
        return procesar_metricas(0, 0);
    }

    int total_puntos = ds->count / 2;
    int puntos_adentro = 0;

    if (imprimir_detalle) {
        printf("\n================ SIMULACIÓN DE MONTE CARLO (ESTIMACIÓN DE π) ================\n");
        printf(" Datos disponibles        : %d números -> %d puntos 2D (x, y)\n", ds->count, total_puntos);
        printf(" Región evaluada          : Cuadrante unitario [0, 1) x [0, 1)\n");
        printf(" Condición de inclusión   : x^2 + y^2 <= 1.0 (cuadrante circular)\n");
        printf("-----------------------------------------------------------------------------\n");
        printf(" Muestra de los primeros puntos evaluados:\n");
    }

    int max_mostrar = (total_puntos < 8) ? total_puntos : 8;
    for (int k = 0; k < total_puntos; k++) {
        double x = ds->data[2 * k];
        double y = ds->data[2 * k + 1];
        double dist2 = x * x + y * y;
        bool dentro = (dist2 <= 1.0);

        if (dentro) {
            puntos_adentro++;
        }

        if (imprimir_detalle && k < max_mostrar) {
            printf("  Punto %2d: (x = %7.5f, y = %7.5f) -> x^2 + y^2 = %7.5f [%s]\n",
                   k + 1, x, y, dist2, dentro ? "DENTRO" : "FUERA ");
        }
    }

    if (imprimir_detalle && total_puntos > max_mostrar) {
        printf("  ... (se omiten %d puntos restantes para brevedad) ...\n", total_puntos - max_mostrar);
    }

    res = procesar_metricas(total_puntos, puntos_adentro);

    if (imprimir_detalle) {
        printf("-----------------------------------------------------------------------------\n");
        printf(" Total puntos simulados     : %d\n", res.total_puntos);
        printf(" Puntos dentro del círculo  : %d (%.2f%%)\n",
               res.puntos_adentro, ((double)res.puntos_adentro / res.total_puntos) * 100.0);
        printf(" Puntos fuera del círculo   : %d (%.2f%%)\n",
               res.puntos_afuera, ((double)res.puntos_afuera / res.total_puntos) * 100.0);
        printf("-----------------------------------------------------------------------------\n");
        printf(" Valor teórico de π         : %.8f\n", res.pi_real);
        printf(" Valor aproximado de π (π̂)  : %.8f (4 * %d / %d)\n",
               res.pi_estimado, res.puntos_adentro, res.total_puntos);
        printf(" Error absoluto             : %.8f\n", res.error_absoluto);
        printf(" Error relativo porcentual  : %.4f%%\n", res.error_porcentual);
        printf(" Error estándar estimado    : %.6f\n", res.error_estandar);
        printf(" Intervalo de confianza 95%%: [%.6f, %.6f]\n", res.ic_inferior, res.ic_superior);
        printf("-----------------------------------------------------------------------------\n");
        printf(" CONCLUSIÓN:\n");
        if (res.dentro_ic) {
            printf("  [✓] El valor real de π (%.5f) se encuentra DENTRO del IC del 95%%.\n", res.pi_real);
            printf("  -> La simulación de Monte Carlo es estadísticamente CONSISTENTE y VÁLIDA.\n");
        } else {
            printf("  [!] El valor real de π (%.5f) quedó fuera del IC del 95%%.\n", res.pi_real);
            printf("  -> Sugerencia: Incrementar el tamaño de muestra (N) para mayor precisión.\n");
        }
        printf("=============================================================================\n\n");
    }

    return res;
}

// Simulación de gran escala para demostrar convergencia con N puntos
MonteCarloResult montecarlo_estimar_pi_simulacion(int num_puntos, unsigned int seed, bool imprimir_progreso) {
    if (num_puntos <= 0) return procesar_metricas(0, 0);

    srand(seed);
    int adentro = 0;

    if (imprimir_progreso) {
        printf("\n================ MONTE CARLO: CONVERGENCIA A GRAN ESCALA ================\n");
        printf(" Simulando %d puntos aleatorios para observar la aproximación...\n", num_puntos);
        printf("-------------------------------------------------------------------------\n");
        printf(" %-12s | %-12s | %-14s | %-14s\n",
               "Puntos (N)", "Dentro", "π Estimado", "Error Relativo");
        printf("-------------------------------------------------------------------------\n");
    }

    int checkpoint = 100;
    for (int i = 1; i <= num_puntos; i++) {
        double x = (double)rand() / ((double)RAND_MAX + 1.0);
        double y = (double)rand() / ((double)RAND_MAX + 1.0);
        if (x * x + y * y <= 1.0) {
            adentro++;
        }

        if (imprimir_progreso && (i == checkpoint || i == num_puntos)) {
            double pi_aprox = 4.0 * ((double)adentro / (double)i);
            double err = (fabs(pi_aprox - M_PI) / M_PI) * 100.0;
            printf(" %-12d | %-12d | %-14.6f | %-13.4f%%\n", i, adentro, pi_aprox, err);
            checkpoint *= 10;
            if (checkpoint > num_puntos && i < num_puntos) {
                checkpoint = num_puntos;
            }
        }
    }

    MonteCarloResult res = procesar_metricas(num_puntos, adentro);
    if (imprimir_progreso) {
        printf("-------------------------------------------------------------------------\n");
        printf(" Resultado Final: π ≈ %.6f (Error: %.4f%%)\n", res.pi_estimado, res.error_porcentual);
        printf("=========================================================================\n\n");
    }

    return res;
}
