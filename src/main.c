#include "common.h"
#include "uniformidad.h"
#include "independencia.h"
#include "poker.h"
#include "montecarlo.h"
#include <string.h>

// Funciones auxiliares para lectura segura de entrada por teclado
static void limpiar_buffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

static int leer_entero(const char *prompt, int valor_defecto) {
    char linea[128];
    printf("%s [%d]: ", prompt, valor_defecto);
    if (fgets(linea, sizeof(linea), stdin) != NULL) {
        int val;
        if (sscanf(linea, "%d", &val) == 1) {
            return val;
        }
    }
    return valor_defecto;
}

static double leer_double(const char *prompt, double valor_defecto) {
    char linea[128];
    printf("%s [%.2f]: ", prompt, valor_defecto);
    if (fgets(linea, sizeof(linea), stdin) != NULL) {
        double val;
        if (sscanf(linea, "%lf", &val) == 1) {
            return val;
        }
    }
    return valor_defecto;
}

static void leer_cadena(const char *prompt, char *buffer, size_t max_len) {
    printf("%s: ", prompt);
    if (fgets(buffer, (int)max_len, stdin) != NULL) {
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }
    }
}

// Ejecuta todas las pruebas en secuencia y genera un cuadro resumen global
static void ejecutar_bateria_completa(const DataSet *ds) {
    if (!ds || ds->count == 0) {
        printf("\n[Error] No hay datos cargados para evaluar la batería.\n");
        return;
    }

    printf("\n======================================================================\n");
    printf("           INICIANDO BATERÍA COMPLETA DE PRUEBAS ESTADÍSTICAS         \n");
    printf("           Tamaño de muestra: N = %d números                         \n", ds->count);
    printf("======================================================================\n");

    // 1. Chi-Cuadrado (k = 10, alpha = 0.05)
    int k = (ds->count >= 50) ? 10 : 5;
    ChiResult chi_res = prueba_chi_cuadrado(ds, k, 0.05, true);

    // 2. Kolmogorov-Smirnov (alpha = 0.05)
    KSResult ks_res = prueba_kolmogorov_smirnov(ds, 0.05, true);

    // 3. Autocorrelación (i = 1, l = 2, alpha = 0.05)
    int i_ini = 1;
    int lag = 2;
    AutoCorrResult auto_res = prueba_autocorrelacion(ds, i_ini, lag, 0.05, true);

    // 4. Póker de 3 dígitos (alpha = 0.05)
    PokerResult poker_res = prueba_poker_tres_digitos(ds, 0.05, true);

    // 5. Simulación de Monte Carlo (Estimación de Pi)
    MonteCarloResult mc_res = montecarlo_estimar_pi_dataset(ds, true);

    // Cuadro resumen final
    printf("\n================================================================================\n");
    printf("                      REPORTE RESUMEN GLOBAL DE PRUEBAS                        \n");
    printf("================================================================================\n");
    printf(" %-30s | %-16s | %-12s | %-12s\n",
           "Prueba Estadística", "Estadístico Calc.", "Valor Crítico", "Resultado");
    printf("--------------------------------------------------------------------------------\n");

    printf(" %-30s | χ0^2 = %-9.4f | χα^2 = %-7.4f| [%s]\n",
           "1. Uniformidad: Chi-Cuadrado", chi_res.chi_calc, chi_res.chi_crit,
           chi_res.aceptada ? " ACEPTADA " : "RECHAZADA");

    printf(" %-30s | D    = %-9.5f | Dα   = %-7.5f| [%s]\n",
           "2. Uniformidad: K-S", ks_res.d_calc, ks_res.d_crit,
           ks_res.aceptada ? " ACEPTADA " : "RECHAZADA");

    printf(" %-30s | |Z0| = %-9.4f | Zα/2 = %-7.4f| [%s]\n",
           "3. Independencia: Autocorr.", fabs(auto_res.z_calc), auto_res.z_crit,
           auto_res.aceptada ? " ACEPTADA " : "RECHAZADA");

    printf(" %-30s | χ0^2 = %-9.4f | χα^2 = %-7.4f| [%s]\n",
           "4. Aleatoriedad: Póker 3D", poker_res.chi_calc, poker_res.chi_crit,
           poker_res.aceptada ? " ACEPTADA " : "RECHAZADA");

    char mc_res_str[32];
    snprintf(mc_res_str, sizeof(mc_res_str), "π̂ = %.4f", mc_res.pi_estimado);
    printf(" %-30s | %-16s | IC 95%%       | [%s]\n",
           "5. Simulación: Monte Carlo", mc_res_str,
           mc_res.dentro_ic ? "COHERENTE " : "DIVERGENTE");

    printf("================================================================================\n\n");
}

int main(void) {
    DataSet ds;
    dataset_init(&ds);

    // Carga inicial por defecto de 100 números con LCG para que el programa inicie listo
    dataset_generate_lcg(&ds, 100, 12345, 1103515245, 12345, 2147483648UL);

    int opcion = -1;
    while (opcion != 0) {
        printf("\n=============================================================\n");
        printf("    SUITE DE PRUEBAS PARA NÚMEROS PSEUDOALEATORIOS EN C     \n");
        printf("=============================================================\n");
        printf(" [Estado actual]: %d números en memoria\n", ds.count);
        printf("-------------------------------------------------------------\n");
        printf("  GESTIÓN DE DATOS:\n");
        printf("   1. Generar números con Generador Congruencial Lineal (LCG)\n");
        printf("   2. Generar números con Generador Estándar (rand)\n");
        printf("   3. Cargar números desde archivo de texto (.txt)\n");
        printf("   4. Mostrar números actualmente en memoria\n");
        printf("-------------------------------------------------------------\n");
        printf("  PRUEBAS ESTADÍSTICAS INDIVIDUALES:\n");
        printf("   5. Prueba de Uniformidad: Chi-Cuadrado (χ²)\n");
        printf("   6. Prueba de Uniformidad: Kolmogorov-Smirnov (K-S)\n");
        printf("   7. Prueba de Independencia: Autocorrelación\n");
        printf("   8. Prueba de Aleatoriedad: Póker (3 dígitos)\n");
        printf("   9. Prueba de Simulación: Monte Carlo (Estimación de π)\n");
        printf("  10. Demostración de convergencia Monte Carlo a gran escala\n");
        printf("-------------------------------------------------------------\n");
        printf("  BATERÍA Y EVALUACIÓN TOTAL:\n");
        printf("  11. Ejecutar BATERÍA COMPLETA (Todas las pruebas + Resumen)\n");
        printf("-------------------------------------------------------------\n");
        printf("   0. Salir\n");
        printf("=============================================================\n");

        opcion = leer_entero("Seleccione una opción", 11);

        switch (opcion) {
            case 1: {
                int n = leer_entero("Cantidad de números a generar (N)", 100);
                int seed = leer_entero("Semilla (X0)", 12345);
                dataset_generate_lcg(&ds, n, (unsigned long)seed, 1103515245UL, 12345UL, 2147483648UL);
                printf("[✓] Se generaron %d números con LCG correctamente.\n", ds.count);
                break;
            }
            case 2: {
                int n = leer_entero("Cantidad de números a generar (N)", 100);
                int seed = leer_entero("Semilla", 42);
                dataset_generate_rand(&ds, n, (unsigned int)seed);
                printf("[✓] Se generaron %d números con rand() correctamente.\n", ds.count);
                break;
            }
            case 3: {
                char ruta[256];
                leer_cadena("Ruta o nombre del archivo de texto", ruta, sizeof(ruta));
                if (dataset_load_from_file(&ds, ruta)) {
                    printf("[✓] Se cargaron %d números desde '%s'.\n", ds.count, ruta);
                } else {
                    printf("[Error] No se pudo abrir o leer el archivo '%s'.\n", ruta);
                }
                break;
            }
            case 4: {
                int cols = leer_entero("Columnas por fila", 5);
                dataset_print(&ds, cols);
                break;
            }
            case 5: {
                int def_k = (ds.count >= 50) ? 10 : 5;
                int k = leer_entero("Número de intervalos (k)", def_k);
                double alpha = leer_double("Nivel de significancia α (0.05 / 0.01 / 0.10)", 0.05);
                prueba_chi_cuadrado(&ds, k, alpha, true);
                break;
            }
            case 6: {
                double alpha = leer_double("Nivel de significancia α (0.05 / 0.01 / 0.10)", 0.05);
                prueba_kolmogorov_smirnov(&ds, alpha, true);
                break;
            }
            case 7: {
                int i_ini = leer_entero("Posición inicial (i)", 1);
                int lag = leer_entero("Salto o retardo (l)", 2);
                double alpha = leer_double("Nivel de significancia α (0.05 / 0.01 / 0.10)", 0.05);
                prueba_autocorrelacion(&ds, i_ini, lag, alpha, true);
                break;
            }
            case 8: {
                double alpha = leer_double("Nivel de significancia α (0.05 / 0.01 / 0.10)", 0.05);
                prueba_poker_tres_digitos(&ds, alpha, true);
                break;
            }
            case 9: {
                montecarlo_estimar_pi_dataset(&ds, true);
                break;
            }
            case 10: {
                int puntos = leer_entero("Cantidad de puntos a simular", 100000);
                montecarlo_estimar_pi_simulacion(puntos, 12345, true);
                break;
            }
            case 11: {
                ejecutar_bateria_completa(&ds);
                break;
            }
            case 0:
                printf("\n¡Gracias por utilizar la Suite de Pruebas Estadísticas! Hasta pronto.\n\n");
                break;
            default:
                printf("[Error] Opción inválida. Intente de nuevo.\n");
                break;
        }

        if (opcion != 0) {
            printf("\nPresione ENTER para continuar...");
            limpiar_buffer();
        }
    }

    return 0;
}
