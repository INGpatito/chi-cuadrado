#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include <SDL2/SDL.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_SDL_RENDERER_IMPLEMENTATION
#define NK_SDL_RENDERER_SDL_H <SDL2/SDL.h>
#include "nuklear.h"
#include "nuklear_sdl_renderer.h"

#include "../common.h"
#include "../uniformidad.h"
#include "../independencia.h"
#include "../poker.h"
#include "../montecarlo.h"

#define WINDOW_WIDTH 1160
#define WINDOW_HEIGHT 740

typedef enum {
    TAB_DATOS = 0,
    TAB_CHI_CUADRADO = 1,
    TAB_KOLMOGOROV = 2,
    TAB_AUTOCORR = 3,
    TAB_POKER = 4,
    TAB_MONTECARLO = 5,
    TAB_BATERIA = 6,
    TOTAL_TABS = 7
} GuiTab;

typedef struct {
    DataSet ds;
    GuiTab pestana_activa;

    // Entradas para generación
    int lcg_n;
    int lcg_seed;
    int rand_n;
    char ruta_archivo[128];
    char mensaje_estado[256];

    // Chi-Cuadrado
    int chi_k;
    double chi_alpha;
    ChiResult chi_res;

    // Kolmogorov-Smirnov
    double ks_alpha;
    KSResult ks_res;

    // Autocorrelación
    int auto_i;
    int auto_l;
    double auto_alpha;
    AutoCorrResult auto_res;

    // Póker
    double poker_alpha;
    PokerResult poker_res;

    // Monte Carlo
    MonteCarloResult mc_res;

    // Métricas globales
    int total_aprobadas;
} AppState;

static AppState app;

// Recalcula automáticamente todas las pruebas cada vez que cambian los datos
static void recalcular_todo(AppState *s) {
    if (s->ds.count == 0) return;

    // 1. Chi-Cuadrado
    if (s->chi_k < 2) s->chi_k = 10;
    s->chi_res = prueba_chi_cuadrado(&s->ds, s->chi_k, s->chi_alpha, false);

    // 2. Kolmogorov-Smirnov
    s->ks_res = prueba_kolmogorov_smirnov(&s->ds, s->ks_alpha, false);

    // 3. Autocorrelación
    if (s->auto_i < 1) s->auto_i = 1;
    if (s->auto_l < 1) s->auto_l = 2;
    if (s->auto_i + s->auto_l <= s->ds.count) {
        s->auto_res = prueba_autocorrelacion(&s->ds, s->auto_i, s->auto_l, s->auto_alpha, false);
    }

    // 4. Póker
    s->poker_res = prueba_poker_tres_digitos(&s->ds, s->poker_alpha, false);

    // 5. Monte Carlo
    if (s->ds.count >= 2) {
        s->mc_res = montecarlo_estimar_pi_dataset(&s->ds, false);
    }

    // Contar aprobadas
    s->total_aprobadas = (s->chi_res.aceptada ? 1 : 0) +
                         (s->ks_res.aceptada ? 1 : 0) +
                         (s->auto_res.aceptada ? 1 : 0) +
                         (s->poker_res.aceptada ? 1 : 0) +
                         (s->mc_res.dentro_ic ? 1 : 0);
}

static void configurar_tema_oscuro(struct nk_context *ctx) {
    struct nk_color table[NK_COLOR_COUNT];
    table[NK_COLOR_TEXT] = nk_rgba(235, 240, 245, 255);
    table[NK_COLOR_WINDOW] = nk_rgba(20, 23, 29, 255);
    table[NK_COLOR_HEADER] = nk_rgba(32, 37, 46, 255);
    table[NK_COLOR_BORDER] = nk_rgba(45, 52, 65, 255);
    table[NK_COLOR_BUTTON] = nk_rgba(35, 41, 53, 255);
    table[NK_COLOR_BUTTON_HOVER] = nk_rgba(50, 60, 78, 255);
    table[NK_COLOR_BUTTON_ACTIVE] = nk_rgba(65, 78, 102, 255);
    table[NK_COLOR_TOGGLE] = nk_rgba(35, 41, 53, 255);
    table[NK_COLOR_TOGGLE_HOVER] = nk_rgba(50, 60, 78, 255);
    table[NK_COLOR_TOGGLE_CURSOR] = nk_rgba(56, 155, 255, 255);
    table[NK_COLOR_SELECT] = nk_rgba(35, 41, 53, 255);
    table[NK_COLOR_SELECT_ACTIVE] = nk_rgba(56, 155, 255, 255);
    table[NK_COLOR_SLIDER] = nk_rgba(30, 35, 45, 255);
    table[NK_COLOR_SLIDER_CURSOR] = nk_rgba(56, 155, 255, 255);
    table[NK_COLOR_SLIDER_CURSOR_HOVER] = nk_rgba(80, 175, 255, 255);
    table[NK_COLOR_SLIDER_CURSOR_ACTIVE] = nk_rgba(100, 190, 255, 255);
    table[NK_COLOR_PROPERTY] = nk_rgba(28, 32, 42, 255);
    table[NK_COLOR_EDIT] = nk_rgba(28, 32, 42, 255);
    table[NK_COLOR_EDIT_CURSOR] = nk_rgba(235, 240, 245, 255);
    table[NK_COLOR_COMBO] = nk_rgba(35, 41, 53, 255);
    table[NK_COLOR_CHART] = nk_rgba(28, 32, 42, 255);
    table[NK_COLOR_CHART_COLOR] = nk_rgba(56, 155, 255, 255);
    table[NK_COLOR_CHART_COLOR_HIGHLIGHT] = nk_rgba(255, 110, 80, 255);
    table[NK_COLOR_SCROLLBAR] = nk_rgba(20, 23, 29, 255);
    table[NK_COLOR_SCROLLBAR_CURSOR] = nk_rgba(45, 52, 65, 255);
    table[NK_COLOR_SCROLLBAR_CURSOR_HOVER] = nk_rgba(60, 70, 88, 255);
    table[NK_COLOR_SCROLLBAR_CURSOR_ACTIVE] = nk_rgba(75, 88, 110, 255);
    table[NK_COLOR_TAB_HEADER] = nk_rgba(32, 37, 46, 255);
    nk_style_from_table(ctx, table);
}

static void init_app_state(void) {
    dataset_init(&app.ds);
    dataset_generate_lcg(&app.ds, 100, 12345, 1103515245UL, 12345UL, 2147483648UL);

    app.pestana_activa = TAB_DATOS;
    app.lcg_n = 100;
    app.lcg_seed = 12345;
    app.rand_n = 100;
    strncpy(app.ruta_archivo, "datos_ejemplo.txt", sizeof(app.ruta_archivo) - 1);
    snprintf(app.mensaje_estado, sizeof(app.mensaje_estado), "Muestra inicial de 100 números cargada.");

    app.chi_k = 10;
    app.chi_alpha = 0.05;

    app.ks_alpha = 0.05;

    app.auto_i = 1;
    app.auto_l = 2;
    app.auto_alpha = 0.05;

    app.poker_alpha = 0.05;

    recalcular_todo(&app);
}

// ============================================================================
// Vistas por Pestaña
// ============================================================================

static void vista_datos(struct nk_context *ctx) {
    nk_layout_row_dynamic(ctx, 28, 1);
    nk_label_colored(ctx, "Paso 1: Generar o Cargar Números Pseudoaleatorios Ri in [0, 1)", NK_TEXT_LEFT, nk_rgb(56, 155, 255));

    nk_layout_row_dynamic(ctx, 42, 3);
    if (nk_button_label(ctx, "⚡ Cargar Archivo 'datos_ejemplo.txt'")) {
        if (dataset_load_from_file(&app.ds, "datos_ejemplo.txt")) {
            snprintf(app.mensaje_estado, sizeof(app.mensaje_estado), "Archivo 'datos_ejemplo.txt' cargado (%d números).", app.ds.count);
            recalcular_todo(&app);
        }
    }
    if (nk_button_label(ctx, "🎲 Generar 100 con LCG")) {
        dataset_generate_lcg(&app.ds, 100, 12345, 1103515245UL, 12345UL, 2147483648UL);
        snprintf(app.mensaje_estado, sizeof(app.mensaje_estado), "100 números generados con LCG.");
        recalcular_todo(&app);
    }
    if (nk_button_label(ctx, "🎲 Generar 500 con rand()")) {
        dataset_generate_rand(&app.ds, 500, (unsigned int)SDL_GetTicks());
        snprintf(app.mensaje_estado, sizeof(app.mensaje_estado), "500 números generados con rand().");
        recalcular_todo(&app);
    }

    nk_layout_row_dynamic(ctx, 470, 2);

    // Controles personalizados
    if (nk_group_begin(ctx, "Config_Datos", NK_WINDOW_BORDER | NK_WINDOW_TITLE)) {
        nk_layout_row_dynamic(ctx, 24, 1);
        nk_label_colored(ctx, "Personalizar Generador Congruencial Lineal:", NK_TEXT_LEFT, nk_rgb(240, 190, 80));

        nk_layout_row_dynamic(ctx, 28, 2);
        nk_property_int(ctx, "Cantidad N:", 10, &app.lcg_n, 10000, 10, 1);
        nk_property_int(ctx, "Semilla X0:", 1, &app.lcg_seed, 999999, 1, 1);

        nk_layout_row_dynamic(ctx, 32, 1);
        if (nk_button_label(ctx, "Generar secuencia LCG personalizada")) {
            dataset_generate_lcg(&app.ds, app.lcg_n, (unsigned long)app.lcg_seed, 1103515245UL, 12345UL, 2147483648UL);
            snprintf(app.mensaje_estado, sizeof(app.mensaje_estado), "%d números generados con LCG.", app.ds.count);
            recalcular_todo(&app);
        }

        nk_layout_row_dynamic(ctx, 16, 1);
        nk_spacer(ctx);

        nk_layout_row_dynamic(ctx, 24, 1);
        nk_label_colored(ctx, "Cargar desde otro archivo de texto:", NK_TEXT_LEFT, nk_rgb(240, 190, 80));

        nk_layout_row_dynamic(ctx, 28, 1);
        nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, app.ruta_archivo, sizeof(app.ruta_archivo), nk_filter_default);

        nk_layout_row_dynamic(ctx, 32, 1);
        if (nk_button_label(ctx, "Cargar archivo especificado")) {
            if (dataset_load_from_file(&app.ds, app.ruta_archivo)) {
                snprintf(app.mensaje_estado, sizeof(app.mensaje_estado), "%d números cargados desde '%s'.", app.ds.count, app.ruta_archivo);
                recalcular_todo(&app);
            } else {
                snprintf(app.mensaje_estado, sizeof(app.mensaje_estado), "Error al leer '%s'.", app.ruta_archivo);
            }
        }

        nk_layout_row_dynamic(ctx, 20, 1);
        nk_spacer(ctx);

        nk_layout_row_dynamic(ctx, 40, 1);
        char info[128];
        snprintf(info, sizeof(info), "Números en memoria: %d | Pruebas listas para explorar.", app.ds.count);
        nk_label_colored(ctx, info, NK_TEXT_LEFT, nk_rgb(60, 220, 120));

        nk_group_end(ctx);
    }

    // Tabla de valores
    if (nk_group_begin(ctx, "Visor_Datos", NK_WINDOW_BORDER | NK_WINDOW_TITLE)) {
        nk_layout_row_dynamic(ctx, 24, 3);
        nk_label_colored(ctx, "Índice", NK_TEXT_CENTERED, nk_rgb(56, 155, 255));
        nk_label_colored(ctx, "Valor Ri", NK_TEXT_CENTERED, nk_rgb(56, 155, 255));
        nk_label_colored(ctx, "3 Dígitos", NK_TEXT_CENTERED, nk_rgb(56, 155, 255));

        for (int i = 0; i < app.ds.count; i++) {
            nk_layout_row_dynamic(ctx, 20, 3);
            char idx[16], val[32], dec[16];
            snprintf(idx, sizeof(idx), "[%d]", i + 1);
            snprintf(val, sizeof(val), "%.5f", app.ds.data[i]);
            int d1, d2, d3;
            clasificar_tres_digitos(app.ds.data[i], &d1, &d2, &d3);
            snprintf(dec, sizeof(dec), "%d%d%d", d1, d2, d3);

            nk_label(ctx, idx, NK_TEXT_CENTERED);
            nk_label(ctx, val, NK_TEXT_CENTERED);
            nk_label(ctx, dec, NK_TEXT_CENTERED);
        }
        nk_group_end(ctx);
    }
}

static void vista_chi_cuadrado(struct nk_context *ctx) {
    nk_layout_row_dynamic(ctx, 28, 1);
    nk_label_colored(ctx, "Prueba de Uniformidad: Chi-Cuadrado (χ²)", NK_TEXT_LEFT, nk_rgb(56, 155, 255));

    // Explicación didáctica
    nk_layout_row_dynamic(ctx, 22, 1);
    nk_label(ctx, "Objetivo: Divide el intervalo [0, 1) en k clases iguales y evalúa si las frecuencias son uniformes.", NK_TEXT_LEFT);

    // Barra de parámetros interactivos
    nk_layout_row_dynamic(ctx, 32, 4);
    int prev_k = app.chi_k;
    nk_property_int(ctx, "Intervalos (k):", 2, &app.chi_k, 30, 1, 1);
    if (prev_k != app.chi_k) {
        recalcular_todo(&app);
    }
    if (nk_button_label(ctx, "α = 0.05 (95% conf.)")) { app.chi_alpha = 0.05; recalcular_todo(&app); }
    if (nk_button_label(ctx, "α = 0.01 (99% conf.)")) { app.chi_alpha = 0.01; recalcular_todo(&app); }
    if (nk_button_label(ctx, "Recalcular")) { recalcular_todo(&app); }

    // Banner de Veredicto Grande y Claro
    nk_layout_row_dynamic(ctx, 42, 1);
    char veredicto[256];
    if (app.chi_res.aceptada) {
        snprintf(veredicto, sizeof(veredicto), "✓ [ACEPTADA] χ0² (%.4f) <= χα² (%.4f) | Grados de libertad: %d -> DISTRIBUCIÓN UNIFORME",
                 app.chi_res.chi_calc, app.chi_res.chi_crit, app.chi_res.grados_libertad);
        nk_label_colored(ctx, veredicto, NK_TEXT_LEFT, nk_rgb(60, 220, 120));
    } else {
        snprintf(veredicto, sizeof(veredicto), "✗ [RECHAZADA] χ0² (%.4f) > χα² (%.4f) | Los números NO se distribuyen uniformemente.",
                 app.chi_res.chi_calc, app.chi_res.chi_crit);
        nk_label_colored(ctx, veredicto, NK_TEXT_LEFT, nk_rgb(255, 80, 80));
    }

    // Gráfica de barras
    nk_layout_row_dynamic(ctx, 20, 1);
    char chart_info[128];
    snprintf(chart_info, sizeof(chart_info), "Frecuencias observadas en los %d intervalos (Frecuencia esperada Ei = %.2f):",
             app.chi_k, (double)app.ds.count / app.chi_k);
    nk_label(ctx, chart_info, NK_TEXT_LEFT);

    int *obs = (int *)calloc(app.chi_k, sizeof(int));
    int max_obs = 1;
    for (int i = 0; i < app.ds.count; i++) {
        int bin = (int)(app.ds.data[i] * app.chi_k);
        if (bin >= app.chi_k) bin = app.chi_k - 1;
        if (bin < 0) bin = 0;
        obs[bin]++;
        if (obs[bin] > max_obs) max_obs = obs[bin];
    }

    nk_layout_row_dynamic(ctx, 150, 1);
    if (nk_chart_begin(ctx, NK_CHART_COLUMN, app.chi_k, 0, (float)(max_obs + 2))) {
        for (int i = 0; i < app.chi_k; i++) {
            nk_chart_push(ctx, (float)obs[i]);
        }
        nk_chart_end(ctx);
    }

    // Tabla de valores
    nk_layout_row_dynamic(ctx, 210, 1);
    if (nk_group_begin(ctx, "Tabla_Chi", NK_WINDOW_BORDER)) {
        nk_layout_row_dynamic(ctx, 24, 6);
        nk_label_colored(ctx, "Intervalo", NK_TEXT_CENTERED, nk_rgb(56, 155, 255));
        nk_label_colored(ctx, "Rango [a, b)", NK_TEXT_CENTERED, nk_rgb(56, 155, 255));
        nk_label_colored(ctx, "Oi (Obs.)", NK_TEXT_CENTERED, nk_rgb(56, 155, 255));
        nk_label_colored(ctx, "Ei (Esp.)", NK_TEXT_CENTERED, nk_rgb(56, 155, 255));
        nk_label_colored(ctx, "Oi - Ei", NK_TEXT_CENTERED, nk_rgb(56, 155, 255));
        nk_label_colored(ctx, "(Oi-Ei)² / Ei", NK_TEXT_CENTERED, nk_rgb(56, 155, 255));

        double exp_val = (double)app.ds.count / (double)app.chi_k;
        for (int i = 0; i < app.chi_k; i++) {
            char no[32], inter[32], oi[16], ei[16], diff[16], term[16];
            double d = (double)obs[i] - exp_val;
            double t = (d * d) / exp_val;

            snprintf(no, sizeof(no), "Clase %d", i + 1);
            snprintf(inter, sizeof(inter), "[%.2f - %.2f)", (double)i/app.chi_k, (double)(i+1)/app.chi_k);
            snprintf(oi, sizeof(oi), "%d", obs[i]);
            snprintf(ei, sizeof(ei), "%.2f", exp_val);
            snprintf(diff, sizeof(diff), "%+.2f", d);
            snprintf(term, sizeof(term), "%.4f", t);

            nk_layout_row_dynamic(ctx, 20, 6);
            nk_label(ctx, no, NK_TEXT_CENTERED);
            nk_label(ctx, inter, NK_TEXT_CENTERED);
            nk_label(ctx, oi, NK_TEXT_CENTERED);
            nk_label(ctx, ei, NK_TEXT_CENTERED);
            nk_label(ctx, diff, NK_TEXT_CENTERED);
            nk_label(ctx, term, NK_TEXT_CENTERED);
        }
        nk_group_end(ctx);
    }
    free(obs);
}

static void vista_kolmogorov(struct nk_context *ctx) {
    nk_layout_row_dynamic(ctx, 28, 1);
    nk_label_colored(ctx, "Prueba de Uniformidad: Kolmogorov-Smirnov (K-S)", NK_TEXT_LEFT, nk_rgb(56, 155, 255));

    nk_layout_row_dynamic(ctx, 22, 1);
    nk_label(ctx, "Objetivo: Evalúa la máxima distancia vertical D entre la función empírica y la uniforme teórica.", NK_TEXT_LEFT);

    nk_layout_row_dynamic(ctx, 32, 3);
    if (nk_button_label(ctx, "α = 0.05 (95% confianza)")) { app.ks_alpha = 0.05; recalcular_todo(&app); }
    if (nk_button_label(ctx, "α = 0.01 (99% confianza)")) { app.ks_alpha = 0.01; recalcular_todo(&app); }
    if (nk_button_label(ctx, "α = 0.10 (90% confianza)")) { app.ks_alpha = 0.10; recalcular_todo(&app); }

    nk_layout_row_dynamic(ctx, 42, 1);
    char veredicto[256];
    if (app.ks_res.aceptada) {
        snprintf(veredicto, sizeof(veredicto), "✓ [ACEPTADA] D Máx (%.5f) <= D Crítico (%.5f) | D+ = %.5f, D- = %.5f -> UNIFORME",
                 app.ks_res.d_calc, app.ks_res.d_crit, app.ks_res.d_mas, app.ks_res.d_menos);
        nk_label_colored(ctx, veredicto, NK_TEXT_LEFT, nk_rgb(60, 220, 120));
    } else {
        snprintf(veredicto, sizeof(veredicto), "✗ [RECHAZADA] D Máx (%.5f) > D Crítico (%.5f) -> Desviación significativa de la uniforme.",
                 app.ks_res.d_calc, app.ks_res.d_crit);
        nk_label_colored(ctx, veredicto, NK_TEXT_LEFT, nk_rgb(255, 80, 80));
    }

    nk_layout_row_dynamic(ctx, 390, 1);
    if (nk_group_begin(ctx, "Tabla_KS", NK_WINDOW_BORDER | NK_WINDOW_TITLE)) {
        nk_layout_row_dynamic(ctx, 24, 6);
        nk_label_colored(ctx, "i", NK_TEXT_CENTERED, nk_rgb(56, 155, 255));
        nk_label_colored(ctx, "R(i) ordenado", NK_TEXT_CENTERED, nk_rgb(56, 155, 255));
        nk_label_colored(ctx, "i / N", NK_TEXT_CENTERED, nk_rgb(56, 155, 255));
        nk_label_colored(ctx, "(i - 1) / N", NK_TEXT_CENTERED, nk_rgb(56, 155, 255));
        nk_label_colored(ctx, "D+ = i/N - Ri", NK_TEXT_CENTERED, nk_rgb(56, 155, 255));
        nk_label_colored(ctx, "D- = Ri - (i-1)/N", NK_TEXT_CENTERED, nk_rgb(56, 155, 255));

        int n = app.ds.count;
        double *sorted = (double *)malloc(n * sizeof(double));
        memcpy(sorted, app.ds.data, n * sizeof(double));
        for (int i = 0; i < n - 1; i++) {
            for (int j = i + 1; j < n; j++) {
                if (sorted[i] > sorted[j]) {
                    double tmp = sorted[i]; sorted[i] = sorted[j]; sorted[j] = tmp;
                }
            }
        }

        for (int i = 0; i < n; i++) {
            double r_i = sorted[i];
            double in = (double)(i + 1) / n;
            double im1n = (double)i / n;
            double dp = in - r_i;
            double dm = r_i - im1n;

            char c_i[16], c_r[16], c_in[16], c_im[16], c_dp[16], c_dm[16];
            snprintf(c_i, sizeof(c_i), "%d", i + 1);
            snprintf(c_r, sizeof(c_r), "%.5f", r_i);
            snprintf(c_in, sizeof(c_in), "%.5f", in);
            snprintf(c_im, sizeof(c_im), "%.5f", im1n);
            snprintf(c_dp, sizeof(c_dp), "%+.5f", dp);
            snprintf(c_dm, sizeof(c_dm), "%+.5f", dm);

            nk_layout_row_dynamic(ctx, 20, 6);
            nk_label(ctx, c_i, NK_TEXT_CENTERED);
            nk_label(ctx, c_r, NK_TEXT_CENTERED);
            nk_label(ctx, c_in, NK_TEXT_CENTERED);
            nk_label(ctx, c_im, NK_TEXT_CENTERED);
            nk_label(ctx, c_dp, NK_TEXT_CENTERED);
            nk_label(ctx, c_dm, NK_TEXT_CENTERED);
        }
        free(sorted);
        nk_group_end(ctx);
    }
}

static void vista_autocorrelacion(struct nk_context *ctx) {
    nk_layout_row_dynamic(ctx, 28, 1);
    nk_label_colored(ctx, "Prueba de Independencia: Autocorrelación", NK_TEXT_LEFT, nk_rgb(56, 155, 255));

    nk_layout_row_dynamic(ctx, 22, 1);
    nk_label(ctx, "Objetivo: Verifica que no exista dependencia o correlación entre números separados por un retardo 'l'.", NK_TEXT_LEFT);

    nk_layout_row_dynamic(ctx, 32, 4);
    int prev_i = app.auto_i;
    int prev_l = app.auto_l;
    nk_property_int(ctx, "Posición inicial (i):", 1, &app.auto_i, 50, 1, 1);
    nk_property_int(ctx, "Retardo / Lag (l):", 1, &app.auto_l, 20, 1, 1);
    if (prev_i != app.auto_i || prev_l != app.auto_l) {
        recalcular_todo(&app);
    }
    if (nk_button_label(ctx, "α = 0.05 (Z = 1.96)")) { app.auto_alpha = 0.05; recalcular_todo(&app); }
    if (nk_button_label(ctx, "α = 0.01 (Z = 2.58)")) { app.auto_alpha = 0.01; recalcular_todo(&app); }

    nk_layout_row_dynamic(ctx, 42, 1);
    char veredicto[256];
    if (app.auto_res.aceptada) {
        snprintf(veredicto, sizeof(veredicto), "✓ [ACEPTADA] |Z0| = %.4f <= Z_α/2 = %.4f | Rango: [%.2f, +%.2f] -> NÚMEROS INDEPENDIENTES",
                 fabs(app.auto_res.z_calc), app.auto_res.z_crit, -app.auto_res.z_crit, app.auto_res.z_crit);
        nk_label_colored(ctx, veredicto, NK_TEXT_LEFT, nk_rgb(60, 220, 120));
    } else {
        snprintf(veredicto, sizeof(veredicto), "✗ [RECHAZADA] |Z0| = %.4f > Z_α/2 = %.4f -> Existe dependencia entre los números.",
                 fabs(app.auto_res.z_calc), app.auto_res.z_crit);
        nk_label_colored(ctx, veredicto, NK_TEXT_LEFT, nk_rgb(255, 80, 80));
    }

    nk_layout_row_dynamic(ctx, 26, 3);
    char m1[64], m2[64], m3[64];
    snprintf(m1, sizeof(m1), "Pares evaluados: %d", app.auto_res.total_pares);
    snprintf(m2, sizeof(m2), "Autocorrelación (ρ̂): %.6f", app.auto_res.rho_estimado);
    snprintf(m3, sizeof(m3), "Desviación estándar (σ_ρ): %.6f", app.auto_res.sigma_rho);
    nk_label(ctx, m1, NK_TEXT_LEFT);
    nk_label(ctx, m2, NK_TEXT_LEFT);
    nk_label(ctx, m3, NK_TEXT_LEFT);

    nk_layout_row_dynamic(ctx, 350, 1);
    if (nk_group_begin(ctx, "Tabla_Autocorr", NK_WINDOW_BORDER | NK_WINDOW_TITLE)) {
        nk_layout_row_dynamic(ctx, 24, 4);
        nk_label_colored(ctx, "Par #", NK_TEXT_CENTERED, nk_rgb(56, 155, 255));
        nk_label_colored(ctx, "Primer término", NK_TEXT_CENTERED, nk_rgb(56, 155, 255));
        nk_label_colored(ctx, "Segundo término", NK_TEXT_CENTERED, nk_rgb(56, 155, 255));
        nk_label_colored(ctx, "Producto", NK_TEXT_CENTERED, nk_rgb(56, 155, 255));

        int start_idx = app.auto_i - 1;
        for (int k = 0; k <= app.auto_res.m; k++) {
            int idx1 = start_idx + k * app.auto_l;
            int idx2 = start_idx + (k + 1) * app.auto_l;
            double prod = app.ds.data[idx1] * app.ds.data[idx2];

            char ck[16], c1[32], c2[32], cp[32];
            snprintf(ck, sizeof(ck), "%d", k + 1);
            snprintf(c1, sizeof(c1), "R[%d] = %.5f", idx1 + 1, app.ds.data[idx1]);
            snprintf(c2, sizeof(c2), "R[%d] = %.5f", idx2 + 1, app.ds.data[idx2]);
            snprintf(cp, sizeof(cp), "%.6f", prod);

            nk_layout_row_dynamic(ctx, 20, 4);
            nk_label(ctx, ck, NK_TEXT_CENTERED);
            nk_label(ctx, c1, NK_TEXT_CENTERED);
            nk_label(ctx, c2, NK_TEXT_CENTERED);
            nk_label(ctx, cp, NK_TEXT_CENTERED);
        }
        nk_group_end(ctx);
    }
}

static void vista_poker(struct nk_context *ctx) {
    nk_layout_row_dynamic(ctx, 28, 1);
    nk_label_colored(ctx, "Prueba de Aleatoriedad: Póker (3 Dígitos)", NK_TEXT_LEFT, nk_rgb(56, 155, 255));

    nk_layout_row_dynamic(ctx, 22, 1);
    nk_label(ctx, "Objetivo: Evalúa si los 3 primeros decimales forman combinaciones aleatorias: Todos Distintos, Par o Tercia.", NK_TEXT_LEFT);

    nk_layout_row_dynamic(ctx, 42, 1);
    char veredicto[256];
    if (app.poker_res.aceptada) {
        snprintf(veredicto, sizeof(veredicto), "✓ [ACEPTADA] χ0² (%.4f) <= χα² (%.4f) con 2 grados de libertad -> DÍGITOS ALEATORIOS",
                 app.poker_res.chi_calc, app.poker_res.chi_crit);
        nk_label_colored(ctx, veredicto, NK_TEXT_LEFT, nk_rgb(60, 220, 120));
    } else {
        snprintf(veredicto, sizeof(veredicto), "✗ [RECHAZADA] χ0² (%.4f) > χα² (%.4f) -> La secuencia presenta sesgo en sus dígitos.",
                 app.poker_res.chi_calc, app.poker_res.chi_crit);
        nk_label_colored(ctx, veredicto, NK_TEXT_LEFT, nk_rgb(255, 80, 80));
    }

    nk_layout_row_dynamic(ctx, 20, 1);
    nk_label(ctx, "Gráfico Comparativo de Manos Observadas vs Probabilidad Teórica:", NK_TEXT_LEFT);

    int max_cat = 1;
    for (int i = 0; i < 3; i++) {
        if (app.poker_res.observados[i] > max_cat) max_cat = app.poker_res.observados[i];
    }

    nk_layout_row_dynamic(ctx, 160, 1);
    if (nk_chart_begin(ctx, NK_CHART_COLUMN, 3, 0, (float)(max_cat + 5))) {
        nk_chart_push(ctx, (float)app.poker_res.observados[0]);
        nk_chart_push(ctx, (float)app.poker_res.observados[1]);
        nk_chart_push(ctx, (float)app.poker_res.observados[2]);
        nk_chart_end(ctx);
    }

    nk_layout_row_dynamic(ctx, 180, 1);
    if (nk_group_begin(ctx, "Tabla_Poker", NK_WINDOW_BORDER)) {
        nk_layout_row_dynamic(ctx, 24, 6);
        nk_label_colored(ctx, "Mano de Póker", NK_TEXT_LEFT, nk_rgb(56, 155, 255));
        nk_label_colored(ctx, "Probabilidad", NK_TEXT_CENTERED, nk_rgb(56, 155, 255));
        nk_label_colored(ctx, "Observados (Oi)", NK_TEXT_CENTERED, nk_rgb(56, 155, 255));
        nk_label_colored(ctx, "Esperados (Ei)", NK_TEXT_CENTERED, nk_rgb(56, 155, 255));
        nk_label_colored(ctx, "Oi - Ei", NK_TEXT_CENTERED, nk_rgb(56, 155, 255));
        nk_label_colored(ctx, "(Oi - Ei)² / Ei", NK_TEXT_CENTERED, nk_rgb(56, 155, 255));

        const char *cats[3] = {"1. Todos Diferentes (TD)", "2. Un Par (1P)", "3. Tercia (3I)"};
        for (int i = 0; i < 3; i++) {
            double diff = (double)app.poker_res.observados[i] - app.poker_res.esperados[i];
            double term = (app.poker_res.esperados[i] > 0) ? (diff * diff) / app.poker_res.esperados[i] : 0.0;

            char c_oi[16], c_ei[16], c_diff[16], c_term[16], c_prob[16];
            snprintf(c_prob, sizeof(c_prob), "%.0f%%", app.poker_res.prob[i] * 100.0);
            snprintf(c_oi, sizeof(c_oi), "%d", app.poker_res.observados[i]);
            snprintf(c_ei, sizeof(c_ei), "%.2f", app.poker_res.esperados[i]);
            snprintf(c_diff, sizeof(c_diff), "%+.2f", diff);
            snprintf(c_term, sizeof(c_term), "%.4f", term);

            nk_layout_row_dynamic(ctx, 24, 6);
            nk_label(ctx, cats[i], NK_TEXT_LEFT);
            nk_label(ctx, c_prob, NK_TEXT_CENTERED);
            nk_label(ctx, c_oi, NK_TEXT_CENTERED);
            nk_label(ctx, c_ei, NK_TEXT_CENTERED);
            nk_label(ctx, c_diff, NK_TEXT_CENTERED);
            nk_label(ctx, c_term, NK_TEXT_CENTERED);
        }
        nk_group_end(ctx);
    }
}

static void vista_montecarlo(struct nk_context *ctx) {
    nk_layout_row_dynamic(ctx, 28, 1);
    nk_label_colored(ctx, "Simulación de Monte Carlo: Estimación Geométrica de π", NK_TEXT_LEFT, nk_rgb(56, 155, 255));

    nk_layout_row_dynamic(ctx, 22, 1);
    nk_label(ctx, "Objetivo: Lanza pares (x, y) en el cuadrado unitario. Los puntos dentro de x²+y²<=1 aproximan pi/4.", NK_TEXT_LEFT);

    nk_layout_row_dynamic(ctx, 480, 2);

    // Lienzo visual interactivo
    if (nk_group_begin(ctx, "Lienzo_MonteCarlo", NK_WINDOW_BORDER | NK_WINDOW_TITLE)) {
        nk_layout_row_dynamic(ctx, 20, 1);
        nk_label(ctx, "Lienzo 2D: Puntos Verdes (Adentro) vs Rojos (Afuera)", NK_TEXT_CENTERED);

        nk_layout_row_dynamic(ctx, 400, 1);
        struct nk_rect space;
        enum nk_widget_layout_states state = nk_widget(&space, ctx);
        if (state != NK_WIDGET_INVALID) {
            struct nk_command_buffer *canvas = nk_window_get_canvas(ctx);

            nk_fill_rect(canvas, space, 4, nk_rgb(16, 19, 25));
            nk_stroke_rect(canvas, space, 4, 1.5f, nk_rgb(50, 60, 78));

            float ox = space.x;
            float oy = space.y + space.h;
            float radio = (space.w < space.h) ? space.w : space.h;

            // Dibujar arco de círculo x^2 + y^2 = 1
            int segmentos = 50;
            for (int s = 0; s < segmentos; s++) {
                float th1 = ((float)s / (float)segmentos) * (3.14159265f / 2.0f);
                float th2 = (((float)s + 1.0f) / (float)segmentos) * (3.14159265f / 2.0f);
                float x1 = ox + cosf(th1) * radio;
                float y1 = oy - sinf(th1) * radio;
                float x2 = ox + cosf(th2) * radio;
                float y2 = oy - sinf(th2) * radio;
                nk_stroke_line(canvas, x1, y1, x2, y2, 2.0f, nk_rgb(56, 155, 255));
            }

            // Puntos graficados
            int total_pts = app.ds.count / 2;
            for (int k = 0; k < total_pts; k++) {
                float px = (float)app.ds.data[2 * k];
                float py = (float)app.ds.data[2 * k + 1];
                bool dentro = (px * px + py * py <= 1.0f);

                float screen_x = ox + px * radio;
                float screen_y = oy - py * radio;

                struct nk_color pt_color = dentro ? nk_rgb(46, 213, 115) : nk_rgb(255, 71, 87);
                nk_fill_circle(canvas, nk_rect(screen_x - 3.0f, screen_y - 3.0f, 6.0f, 6.0f), pt_color);
            }
        }
        nk_group_end(ctx);
    }

    // Panel de métricas claras
    if (nk_group_begin(ctx, "Metricas_MonteCarlo", NK_WINDOW_BORDER | NK_WINDOW_TITLE)) {
        nk_layout_row_dynamic(ctx, 36, 1);
        char pi_str[64];
        snprintf(pi_str, sizeof(pi_str), "π Estimado: %.6f", app.mc_res.pi_estimado);
        nk_label_colored(ctx, pi_str, NK_TEXT_LEFT, nk_rgb(46, 213, 115));

        nk_layout_row_dynamic(ctx, 24, 1);
        char pi_real_str[64];
        snprintf(pi_real_str, sizeof(pi_real_str), "π Teórico Real: %.8f", app.mc_res.pi_real);
        nk_label(ctx, pi_real_str, NK_TEXT_LEFT);

        nk_layout_row_dynamic(ctx, 24, 1);
        char err_str[64];
        snprintf(err_str, sizeof(err_str), "Error Relativo: %.4f%%", app.mc_res.error_porcentual);
        nk_label(ctx, err_str, NK_TEXT_LEFT);

        nk_layout_row_dynamic(ctx, 24, 1);
        char ic_str[96];
        snprintf(ic_str, sizeof(ic_str), "Intervalo Confianza 95%%: [%.5f, %.5f]", app.mc_res.ic_inferior, app.mc_res.ic_superior);
        nk_label(ctx, ic_str, NK_TEXT_LEFT);

        nk_layout_row_dynamic(ctx, 16, 1);
        nk_spacer(ctx);

        nk_layout_row_dynamic(ctx, 24, 1);
        char p_tot[64], p_in[64], p_out[64];
        snprintf(p_tot, sizeof(p_tot), "Total dardos (pares x,y): %d", app.mc_res.total_puntos);
        snprintf(p_in, sizeof(p_in), "🟢 Dentro del círculo: %d (%.1f%%)",
                 app.mc_res.puntos_adentro, ((double)app.mc_res.puntos_adentro / app.mc_res.total_puntos) * 100.0);
        snprintf(p_out, sizeof(p_out), "🔴 Fuera del círculo : %d (%.1f%%)",
                 app.mc_res.puntos_afuera, ((double)app.mc_res.puntos_afuera / app.mc_res.total_puntos) * 100.0);
        nk_label(ctx, p_tot, NK_TEXT_LEFT);
        nk_label_colored(ctx, p_in, NK_TEXT_LEFT, nk_rgb(46, 213, 115));
        nk_label_colored(ctx, p_out, NK_TEXT_LEFT, nk_rgb(255, 71, 87));

        nk_layout_row_dynamic(ctx, 20, 1);
        nk_spacer(ctx);

        nk_layout_row_dynamic(ctx, 42, 1);
        if (app.mc_res.dentro_ic) {
            nk_label_colored(ctx, "✓ El valor real de π se ubica dentro del IC al 95%.\nSimulación matemáticamente coherente.",
                             NK_TEXT_LEFT, nk_rgb(46, 213, 115));
        } else {
            nk_label_colored(ctx, "! π real quedó ligeramente fuera del IC al 95%.\n(Normal en muestras pequeñas, aumenta N para mayor precisión).",
                             NK_TEXT_LEFT, nk_rgb(255, 165, 2));
        }

        nk_group_end(ctx);
    }
}

static void vista_bateria(struct nk_context *ctx) {
    nk_layout_row_dynamic(ctx, 28, 1);
    nk_label_colored(ctx, "Resumen Global de Todas las Pruebas Estadísticas", NK_TEXT_LEFT, nk_rgb(56, 155, 255));

    nk_layout_row_dynamic(ctx, 44, 2);
    char score[128];
    snprintf(score, sizeof(score), "Resultado: %d de 5 Pruebas Aceptadas", app.total_aprobadas);
    if (app.total_aprobadas == 5) {
        nk_label_colored(ctx, score, NK_TEXT_LEFT, nk_rgb(46, 213, 115));
    } else {
        nk_label_colored(ctx, score, NK_TEXT_LEFT, nk_rgb(255, 165, 2));
    }

    if (nk_button_label(ctx, "⚡ Recalcular Toda la Batería")) {
        recalcular_todo(&app);
    }

    nk_layout_row_dynamic(ctx, 420, 1);
    if (nk_group_begin(ctx, "Tabla_Resumen", NK_WINDOW_BORDER | NK_WINDOW_TITLE)) {
        nk_layout_row_dynamic(ctx, 32, 4);
        nk_label_colored(ctx, "Prueba Evaluada", NK_TEXT_LEFT, nk_rgb(56, 155, 255));
        nk_label_colored(ctx, "Estadístico Calculado", NK_TEXT_CENTERED, nk_rgb(56, 155, 255));
        nk_label_colored(ctx, "Valor Crítico", NK_TEXT_CENTERED, nk_rgb(56, 155, 255));
        nk_label_colored(ctx, "Estado Final", NK_TEXT_CENTERED, nk_rgb(56, 155, 255));

        // 1. Chi-Cuadrado
        nk_layout_row_dynamic(ctx, 40, 4);
        nk_label(ctx, "1. Uniformidad: Chi-Cuadrado", NK_TEXT_LEFT);
        char c_calc1[32], c_crit1[32];
        snprintf(c_calc1, sizeof(c_calc1), "χ0² = %.4f", app.chi_res.chi_calc);
        snprintf(c_crit1, sizeof(c_crit1), "χα² = %.4f", app.chi_res.chi_crit);
        nk_label(ctx, c_calc1, NK_TEXT_CENTERED);
        nk_label(ctx, c_crit1, NK_TEXT_CENTERED);
        nk_label_colored(ctx, app.chi_res.aceptada ? "✓ [ ACEPTADA ]" : "✗ [ RECHAZADA ]",
                         NK_TEXT_CENTERED, app.chi_res.aceptada ? nk_rgb(46, 213, 115) : nk_rgb(255, 71, 87));

        // 2. Kolmogorov-Smirnov
        nk_layout_row_dynamic(ctx, 40, 4);
        nk_label(ctx, "2. Uniformidad: Kolmogorov-Smirnov", NK_TEXT_LEFT);
        char c_calc2[32], c_crit2[32];
        snprintf(c_calc2, sizeof(c_calc2), "D = %.5f", app.ks_res.d_calc);
        snprintf(c_crit2, sizeof(c_crit2), "Dα = %.5f", app.ks_res.d_crit);
        nk_label(ctx, c_calc2, NK_TEXT_CENTERED);
        nk_label(ctx, c_crit2, NK_TEXT_CENTERED);
        nk_label_colored(ctx, app.ks_res.aceptada ? "✓ [ ACEPTADA ]" : "✗ [ RECHAZADA ]",
                         NK_TEXT_CENTERED, app.ks_res.aceptada ? nk_rgb(46, 213, 115) : nk_rgb(255, 71, 87));

        // 3. Autocorrelación
        nk_layout_row_dynamic(ctx, 40, 4);
        nk_label(ctx, "3. Independencia: Autocorrelación", NK_TEXT_LEFT);
        char c_calc3[32], c_crit3[32];
        snprintf(c_calc3, sizeof(c_calc3), "|Z0| = %.4f", fabs(app.auto_res.z_calc));
        snprintf(c_crit3, sizeof(c_crit3), "Zα/2 = %.4f", app.auto_res.z_crit);
        nk_label(ctx, c_calc3, NK_TEXT_CENTERED);
        nk_label(ctx, c_crit3, NK_TEXT_CENTERED);
        nk_label_colored(ctx, app.auto_res.aceptada ? "✓ [ ACEPTADA ]" : "✗ [ RECHAZADA ]",
                         NK_TEXT_CENTERED, app.auto_res.aceptada ? nk_rgb(46, 213, 115) : nk_rgb(255, 71, 87));

        // 4. Póker
        nk_layout_row_dynamic(ctx, 40, 4);
        nk_label(ctx, "4. Aleatoriedad: Póker 3 Dígitos", NK_TEXT_LEFT);
        char c_calc4[32], c_crit4[32];
        snprintf(c_calc4, sizeof(c_calc4), "χ0² = %.4f", app.poker_res.chi_calc);
        snprintf(c_crit4, sizeof(c_crit4), "χα² = %.4f", app.poker_res.chi_crit);
        nk_label(ctx, c_calc4, NK_TEXT_CENTERED);
        nk_label(ctx, c_crit4, NK_TEXT_CENTERED);
        nk_label_colored(ctx, app.poker_res.aceptada ? "✓ [ ACEPTADA ]" : "✗ [ RECHAZADA ]",
                         NK_TEXT_CENTERED, app.poker_res.aceptada ? nk_rgb(46, 213, 115) : nk_rgb(255, 71, 87));

        // 5. Monte Carlo
        nk_layout_row_dynamic(ctx, 40, 4);
        nk_label(ctx, "5. Simulación: Monte Carlo π", NK_TEXT_LEFT);
        char c_calc5[32];
        snprintf(c_calc5, sizeof(c_calc5), "π̂ = %.4f", app.mc_res.pi_estimado);
        nk_label(ctx, c_calc5, NK_TEXT_CENTERED);
        nk_label(ctx, "IC 95%", NK_TEXT_CENTERED);
        nk_label_colored(ctx, app.mc_res.dentro_ic ? "✓ [ COHERENTE ]" : "! [ DIVERGENTE ]",
                         NK_TEXT_CENTERED, app.mc_res.dentro_ic ? nk_rgb(46, 213, 115) : nk_rgb(255, 165, 2));

        nk_layout_row_dynamic(ctx, 24, 1);
        nk_spacer(ctx);

        nk_layout_row_dynamic(ctx, 40, 1);
        if (app.total_aprobadas == 5) {
            nk_label_colored(ctx, "★ CONCLUSIÓN: Los números pseudoaleatorios superaron TODAS las pruebas y son APTOS para simulación.",
                             NK_TEXT_CENTERED, nk_rgb(46, 213, 115));
        } else {
            nk_label_colored(ctx, "⚠ CONCLUSIÓN: Algunas pruebas no se cumplieron con el nivel de significancia seleccionado.",
                             NK_TEXT_CENTERED, nk_rgb(255, 165, 2));
        }

        nk_group_end(ctx);
    }
}

// ============================================================================
// Función principal de la GUI con Barra Lateral y Navegación Paso a Paso
// ============================================================================
int main(int argc, char *argv[]) {
    (void)argc; (void)argv;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS) != 0) {
        fprintf(stderr, "Error al inicializar SDL2: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *win = SDL_CreateWindow(
        "Suite de Simulación - Pruebas Estadísticas para Números Pseudoaleatorios",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE
    );
    if (!win) {
        fprintf(stderr, "Error al crear ventana SDL2: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!ren) {
        ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_SOFTWARE);
        if (!ren) {
            fprintf(stderr, "Error al crear renderer: %s\n", SDL_GetError());
            SDL_DestroyWindow(win);
            SDL_Quit();
            return 1;
        }
    }

    struct nk_context *ctx = nk_sdl_init(win, ren);
    if (!ctx) {
        fprintf(stderr, "Error al inicializar Nuklear SDL\n");
        return 1;
    }

    struct nk_font_atlas *atlas;
    nk_sdl_font_stash_begin(&atlas);
    nk_sdl_font_stash_end();

    configurar_tema_oscuro(ctx);
    init_app_state();

    bool running = true;
    while (running) {
        SDL_Event evt;
        nk_input_begin(ctx);
        while (SDL_PollEvent(&evt)) {
            if (evt.type == SDL_QUIT) {
                running = false;
            }
            nk_sdl_handle_event(&evt);
        }
        nk_input_end(ctx);

        int w, h;
        SDL_GetWindowSize(win, &w, &h);

        // Ventana principal Nuklear
        if (nk_begin(ctx, "AppMain", nk_rect(0, 0, (float)w, (float)h), NK_WINDOW_NO_SCROLLBAR)) {

            // Encabezado Superior
            nk_layout_row_template_begin(ctx, 42);
            nk_layout_row_template_push_static(ctx, 500);
            nk_layout_row_template_push_dynamic(ctx);
            nk_layout_row_template_end(ctx);

            nk_label_colored(ctx, "🔬 SUITE DE PRUEBAS DE SIMULACIÓN", NK_TEXT_LEFT, nk_rgb(56, 155, 255));

            char status_pill[512];
            snprintf(status_pill, sizeof(status_pill), "Muestra: %d números | %d/5 Aprobadas | %s",
                     app.ds.count, app.total_aprobadas, app.mensaje_estado);
            nk_label_colored(ctx, status_pill, NK_TEXT_RIGHT, nk_rgb(180, 190, 205));

            // Divisor
            nk_layout_row_dynamic(ctx, 4, 1);
            nk_spacer(ctx);

            // Cuerpo dividido: Barra Lateral Izquierda (230px) + Contenido Principal
            float sidebar_w = 230.0f;
            float content_w = (float)w - sidebar_w - 25.0f;
            float main_h = (float)h - 60.0f;

            nk_layout_row_template_begin(ctx, main_h);
            nk_layout_row_template_push_static(ctx, sidebar_w);
            nk_layout_row_template_push_static(ctx, content_w);
            nk_layout_row_template_end(ctx);

            // ==========================================
            // BARRA LATERAL IZQUIERDA (FLUJO GUIADO)
            // ==========================================
            if (nk_group_begin(ctx, "Sidebar", NK_WINDOW_BORDER)) {
                nk_layout_row_dynamic(ctx, 24, 1);
                nk_label_colored(ctx, "PASOS DEL PROCESO:", NK_TEXT_LEFT, nk_rgb(240, 190, 80));

                const char *tab_names[TOTAL_TABS] = {
                    "1. 📊 Cargar / Generar",
                    "2. 📈 Chi-Cuadrado (χ²)",
                    "3. 📉 Kolmogorov-S.",
                    "4. 🔗 Autocorrelación",
                    "5. 🃏 Póker 3 Dígitos",
                    "6. 🎯 Monte Carlo (π)",
                    "7. 📋 Resumen Total"
                };

                for (int t = 0; t < TOTAL_TABS; t++) {
                    nk_layout_row_dynamic(ctx, 42, 1);
                    if (t == (int)app.pestana_activa) {
                        // Resaltar la pestaña actual
                        if (nk_button_label(ctx, tab_names[t])) {
                            app.pestana_activa = (GuiTab)t;
                        }
                    } else {
                        if (nk_button_label(ctx, tab_names[t])) {
                            app.pestana_activa = (GuiTab)t;
                        }
                    }
                }

                nk_layout_row_dynamic(ctx, 20, 1);
                nk_spacer(ctx);

                nk_layout_row_dynamic(ctx, 24, 1);
                nk_label_colored(ctx, "ACCIONES RÁPIDAS:", NK_TEXT_LEFT, nk_rgb(240, 190, 80));

                nk_layout_row_dynamic(ctx, 36, 1);
                if (nk_button_label(ctx, "⚡ Evaluar Todo")) {
                    recalcular_todo(&app);
                    app.pestana_activa = TAB_BATERIA;
                }

                nk_layout_row_dynamic(ctx, 36, 1);
                if (nk_button_label(ctx, "📂 Cargar Ejemplo")) {
                    dataset_load_from_file(&app.ds, "datos_ejemplo.txt");
                    recalcular_todo(&app);
                    snprintf(app.mensaje_estado, sizeof(app.mensaje_estado), "Archivo de ejemplo cargado.");
                }

                nk_layout_row_dynamic(ctx, 36, 1);
                if (nk_button_label(ctx, "🎲 Generar 100 LCG")) {
                    dataset_generate_lcg(&app.ds, 100, 12345, 1103515245UL, 12345UL, 2147483648UL);
                    recalcular_todo(&app);
                    snprintf(app.mensaje_estado, sizeof(app.mensaje_estado), "100 números con LCG generados.");
                }

                nk_group_end(ctx);
            }

            // ==========================================
            // ÁREA DE CONTENIDO PRINCIPAL (DERECHA)
            // ==========================================
            if (nk_group_begin(ctx, "MainContent", NK_WINDOW_BORDER)) {
                switch (app.pestana_activa) {
                    case TAB_DATOS: vista_datos(ctx); break;
                    case TAB_CHI_CUADRADO: vista_chi_cuadrado(ctx); break;
                    case TAB_KOLMOGOROV: vista_kolmogorov(ctx); break;
                    case TAB_AUTOCORR: vista_autocorrelacion(ctx); break;
                    case TAB_POKER: vista_poker(ctx); break;
                    case TAB_MONTECARLO: vista_montecarlo(ctx); break;
                    case TAB_BATERIA: vista_bateria(ctx); break;
                    default: break;
                }

                // Barra de navegación inferior: Anterior / Siguiente
                nk_layout_row_dynamic(ctx, 16, 1);
                nk_spacer(ctx);

                nk_layout_row_dynamic(ctx, 38, 2);
                if (app.pestana_activa > 0) {
                    if (nk_button_label(ctx, "⬅ Paso Anterior")) {
                        app.pestana_activa = (GuiTab)((int)app.pestana_activa - 1);
                    }
                } else {
                    nk_spacer(ctx);
                }

                if (app.pestana_activa < TOTAL_TABS - 1) {
                    if (nk_button_label(ctx, "Siguiente Prueba ➡")) {
                        app.pestana_activa = (GuiTab)((int)app.pestana_activa + 1);
                    }
                } else {
                    if (nk_button_label(ctx, "🔄 Volver al Inicio (Paso 1)")) {
                        app.pestana_activa = TAB_DATOS;
                    }
                }

                nk_group_end(ctx);
            }
        }
        nk_end(ctx);

        SDL_SetRenderDrawColor(ren, 20, 23, 29, 255);
        SDL_RenderClear(ren);
        nk_sdl_render(NK_ANTI_ALIASING_ON);
        SDL_RenderPresent(ren);
    }

    nk_sdl_shutdown();
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
