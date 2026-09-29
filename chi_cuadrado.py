"""
Prueba de Uniformidad de Chi-Cuadrado (χ²) para Números Pseudoaleatorios
Lenguaje: Python (Usa únicamente librerías estándar: sin pip install)
"""

import math
import random


def generar_numeros_lcg(n=100, semilla=12345):
    """
    Genera n números pseudoaleatorios en el intervalo [0, 1) usando
    un Generador Congruencial Lineal (LCG): X_{n+1} = (a * X_n + c) mod m
    """
    a = 1103515245
    c = 12345
    m = 2147483648  # 2^31
    actual = semilla
    numeros = []

    for _ in range(n):
        actual = (a * actual + c) % m
        numeros.append(actual / m)

    return numeros


def obtener_chi_critico(gl, alpha=0.05):
    """
    Retorna el valor crítico tabulado de Chi-cuadrado para gl grados de libertad.
    Para gl <= 30 usa la tabla exacta; para gl > 30 usa la fórmula de Wilson-Hilferty.
    """
    tabla_005 = {
        1: 3.841, 2: 5.991, 3: 7.815, 4: 9.488, 5: 11.070,
        6: 12.592, 7: 14.067, 8: 15.507, 9: 16.919, 10: 18.307,
        11: 19.675, 12: 21.026, 13: 22.362, 14: 23.685, 15: 24.996,
        16: 26.296, 17: 27.587, 18: 28.869, 19: 30.144, 20: 31.410,
        21: 32.671, 22: 33.924, 23: 35.172, 24: 36.415, 25: 37.652,
        26: 38.885, 27: 40.113, 28: 41.337, 29: 42.557, 30: 43.773
    }

    if gl in tabla_005:
        return tabla_005[gl]

    # Aproximación para grados de libertad grandes (gl > 30)
    z = 1.644853  # para alpha = 0.05
    factor = 1.0 - (2.0 / (9.0 * gl)) + z * math.sqrt(2.0 / (9.0 * gl))
    return gl * (factor ** 3)


def prueba_chi_cuadrado(numeros, k=10, alpha=0.05):
    """
    Realiza la prueba de uniformidad de Chi-cuadrado sobre una lista de números en [0, 1).
    
    Parámetros:
      - numeros: lista de números flotantes en [0, 1).
      - k: cantidad de subintervalos o clases (comúnmente 10 o 5).
      - alpha: nivel de significancia (por defecto 0.05 = 95% de confianza).
    """
    N = len(numeros)
    if N == 0:
        print("[Error] La lista de números está vacía.")
        return

    # 1. Contar frecuencias observadas (Oi) por cada intervalo
    observadas = [0] * k
    for r in numeros:
        # Ubicar en qué cubeta/intervalo cae el número (entre 0 y k-1)
        intervalo = int(r * k)
        if intervalo >= k:
            intervalo = k - 1
        elif intervalo < 0:
            intervalo = 0
        observadas[intervalo] += 1

    # 2. Calcular frecuencia esperada (Ei)
    esperada = N / k
    gl = k - 1  # Grados de libertad

    # 3. Imprimir la tabla de frecuencias y calcular el estadístico
    print("\n" + "=" * 70)
    print("           PRUEBA DE UNIFORMIDAD DE CHI-CUADRADO (χ²)")
    print(f" Tamaño de muestra (N)    : {N}")
    print(f" Cantidad de clases (k)   : {k}")
    print(f" Grados de libertad (k-1) : {gl}")
    print(f" Nivel de significancia α : {alpha} ({(1 - alpha) * 100:.0f}% de confianza)")
    print(f" Frecuencia esperada (Ei) : {esperada:.2f}")
    print("-" * 70)
    print(f" {'Clase':<6} | {'Rango':<15} | {'Oi':<6} | {'Ei':<6} | {'Oi - Ei':<8} | {'(Oi-Ei)²/Ei':<12}")
    print("-" * 70)

    chi_calc = 0.0
    for i in range(k):
        lim_inf = i / k
        lim_sup = (i + 1) / k
        oi = observadas[i]
        diferencia = oi - esperada
        termino = (diferencia ** 2) / esperada
        chi_calc += termino

        print(f" {i + 1:<6} | [{lim_inf:.2f} - {lim_sup:.2f})  | {oi:<6} | {esperada:<6.2f} | {diferencia:<+8.2f} | {termino:<12.4f}")

    print("-" * 70)

    # 4. Obtener valor crítico y emitir conclusión
    chi_crit = obtener_chi_critico(gl, alpha)

    print(f" TOTAL Oi = {sum(observadas)} | Chi-cuadrado Calculado (χ0²) = {chi_calc:.4f}")
    print(f" Chi-cuadrado Crítico   (χα,gl)        = {chi_crit:.4f}")
    print("-" * 70)

    if chi_calc <= chi_crit:
        print(" [✓] CONCLUSIÓN: Se ACEPTA H0")
        print(f"     χ0² ({chi_calc:.4f}) <= χα² ({chi_crit:.4f}).")
        print(f"     No hay evidencia para rechazar que los números sigan una distribución")
        print(f"     UNIFORME continua U(0,1) al {(1 - alpha) * 100:.0f}% de confianza.")
    else:
        print(" [X] CONCLUSIÓN: Se RECHAZA H0")
        print(f"     χ0² ({chi_calc:.4f}) > χα² ({chi_crit:.4f}).")
        print("     Los números NO se distribuyen uniformemente.")
    print("=" * 70 + "\n")


# ============================================================================
# PROGRAMA PRINCIPAL
# ============================================================================
if __name__ == "__main__":
    print("Generando 100 números pseudoaleatorios con el generador LCG...")
    datos = generar_numeros_lcg(n=100, semilla=12345)

    # Ejecutar la prueba de Chi-cuadrado con 10 intervalos y alpha = 0.05
    prueba_chi_cuadrado(datos, k=10, alpha=0.05)
