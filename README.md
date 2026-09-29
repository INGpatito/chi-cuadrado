# Suite de Pruebas Estadísticas para Números Pseudoaleatorios en C

Este proyecto contiene una suite completa, modular y documentada para la verificación de secuencias de números pseudoaleatorios $R_i \in [0, 1)$, abarcando las pruebas clásicas de Simulación de Sistemas y Estadística Computacional.

Está disponible en **4 formatos complementarios**:
1. 🐍 **Versión Python con GUI (`chi.py`)**: Interfaz gráfica en Python estándar (Tkinter) para ingresar números manualmente (tecleando o pegando), ver la tabla y el histograma en tiempo real sin instalar librerías.
2. 🌐 **Versión Web Interactiva (`index.html`)**: Para abrir directamente en el navegador con un doble clic o publicar en **GitHub Pages** con acceso online público para cualquier dispositivo.
3. 🖥️ **Versión Gráfica de Escritorio en C (`pruebas_gui`)**: Aplicación nativa en C con ventana acelerada por hardware (Nuklear + SDL2), gráficos y simulación 2D.
4. 💻 **Versión de Consola en C (`pruebas_simulacion`)**: Portabilidad universal en C puro para terminal sin dependencias externas.

1. **Pruebas de Uniformidad**:
   - **Chi-Cuadrado ($\chi^2$)**: Prueba de bondad de ajuste con $k$ intervalos y frecuencias observadas vs esperadas.
   - **Kolmogorov-Smirnov (K-S)**: Comparación de la función de distribución acumulada empírica $F_N(x)$ vs la teórica uniforme $F(x) = x$.
2. **Prueba de Independencia**:
   - **Autocorrelación**: Detección de correlación entre números separados por un salto $l$ (lag) a partir de la posición $i$, mediante el estadístico normal estándar $Z_0$.
3. **Prueba de Aleatoriedad**:
   - **Póker (3 dígitos)**: Clasificación de los primeros 3 decimales en combinaciones de:
     - Todos Diferentes (TD, $P=0.72$)
     - Un Par (1P, $P=0.27$)
     - Tercia (3I, $P=0.01$)
     y contraste $\chi^2$ con 2 grados de libertad.
4. **Simulación de Monte Carlo**:
   - **Estimación de $\pi$**: Método del dardo en el cuadrante circular $x^2 + y^2 \le 1.0$, cálculo de error absoluto, error relativo porcentual e intervalo de confianza al 95%.
   - **Convergencia a gran escala**: Demostración numérica de convergencia desde 100 hasta 100,000 puntos.

---

## Estructura del Proyecto

```
chi-cuadrado/
├── Makefile                # Reglas de compilación automatizada
├── README.md               # Documentación general
├── datos_ejemplo.txt       # Archivo de prueba con números en [0, 1)
└── src/
    ├── common.h            # Estructuras de datos base (DataSet) y utilidades
    ├── generator.c         # Generadores (LCG, rand) y lector de archivos .txt
    ├── uniformidad.h       # Prototipos de Chi-cuadrado y Kolmogorov-Smirnov
    ├── uniformidad.c       # Implementación y tablas críticas de Chi-cuadrado y K-S
    ├── independencia.h     # Prototipos de prueba de Autocorrelación
    ├── independencia.c     # Implementación del estimador de autocorrelación y Z0
    ├── poker.h             # Prototipos de la prueba de Póker (3 dígitos)
    ├── poker.c             # Extracción de dígitos y prueba de Póker
    ├── montecarlo.h        # Prototipos de Monte Carlo y estimación de Pi
    ├── montecarlo.c        # Simulación geométrica de Monte Carlo y métricas
    └── main.c              # Menú interactivo y ejecución de batería completa
```

---

## 🌐 Cómo usar la Versión Web y Publicarla en GitHub Pages

### Uso Local (Sin Servidor)
Solo haz doble clic sobre el archivo [`index.html`](file:///mnt/windows/Linux/chi-cuadrado/index.html) o ábrelo con cualquier navegador web (Chrome, Firefox, Edge, Safari). Funciona al 100% de forma autónoma sin internet.

### Publicar en la Web Gratis con GitHub Pages (En 1 Minuto)
Para que cualquier persona (incluido tu maestro) pueda entrar desde su computadora o celular mediante un enlace público de internet:
1. Sube tu proyecto a GitHub (`git add .`, `git commit`, `git push origin main`).
2. En tu repositorio de GitHub, dirígete a: **Settings** (Configuración) > **Pages** (en el menú lateral izquierdo).
3. En **Build and deployment > Source**, selecciona: **Deploy from a branch**.
4. En **Branch**, elige: `main` y la carpeta `/(root)`.
5. Haz clic en **Save** (Guardar).
6. En un par de minutos, GitHub te dará una URL pública tipo:
   `https://TU_USUARIO.github.io/TU_REPOSITORIO/`

---

## Compilación y Ejecución en C (Consola y Escritorio)
1. **Versión de Consola (`pruebas_simulacion`)**: Portabilidad universal (cero dependencias externas).
2. **Versión con Interfaz Gráfica (`pruebas_gui`)**: Interfaz de escritorio nativa con Nuklear y SDL2 (gráficos interactivos y simulación 2D).

### Requisitos
- Compilador de C estándar (`gcc` o `clang`).
- Biblioteca matemática estándar (`-lm`).
- Para la versión GUI: biblioteca `SDL2` (`libsdl2-dev` en Ubuntu/Debian, `sdl2` en Arch).

### Compilar todo (Consola + GUI)
```bash
make
```

O por separado:
```bash
make cli   # Solo versión de consola
make gui   # Solo versión con interfaz gráfica
```

### Ejecutar
- **Interfaz Gráfica de Escritorio**:
  ```bash
  ./pruebas_gui
  ```
- **Consola Interactiva**:
  ```bash
  ./pruebas_simulacion
  ```

### Limpiar binarios
```bash
make clean
```

---

## Características de la Interfaz Gráfica (GUI)

La aplicación de escritorio cuenta con un tema oscuro moderno y pestañas superiores interactivas:
- 📊 **Datos**: Controles para generar números mediante LCG o `rand()`, cargar archivos de texto `.txt` y visor de datos con sus dígitos decimales.
- 📈 **Chi² (Chi-Cuadrado)**: Histograma de barras visual en tiempo real de frecuencias observadas ($O_i$) vs esperadas ($E_i$), tabla completa y badge de veredicto.
- 📉 **K-S (Kolmogorov-Smirnov)**: Cálculo de diferencias $D^+$ y $D^-$, tabla de ordenamiento $R_{(i)}$ y detección de máximos.
- 🔗 **Autocorr. (Autocorrelación)**: Configuración dinámica de retardo $l$ y posición inicial $i$, tabla de pares evaluados y cálculo de $Z_0$.
- 🃏 **Póker 3D**: Gráfico comparativo de las 3 manos (Todos Diferentes, Un Par, Tercia) y contraste $\chi^2$ con $gl=2$.
- 🎯 **Monte Carlo**: **Lienzo 2D interactivo** del cuadrante circular unitario donde se grafican los puntos aleatorios (verdes si caen dentro de $x^2+y^2 \le 1$, rojos si caen fuera) junto con la estimación en vivo de $\pi$ y su intervalo de confianza al 95%.
- 📋 **Batería**: Botón para evaluar las 5 pruebas de golpe y desplegar el cuadro resumen con badges de `[ ACEPTADA ]` / `[ RECHAZADA ]`.

---

## Uso del Menú Interactivo

Al iniciar la aplicación, se presenta una interfaz interactiva con las siguientes opciones:

1. **Gestión de Datos**:
   - `1`: Generar $N$ números con un Generador Congruencial Lineal (LCG: $X_{n+1} = (aX_n + c) \pmod m$).
   - `2`: Generar $N$ números con el generador estándar (`rand()`).
   - `3`: Cargar una secuencia de números desde un archivo de texto (por ejemplo `datos_ejemplo.txt`).
   - `4`: Ver la secuencia actual de números tabulada en consola.
2. **Pruebas Individuales**:
   - `5`: Ejecutar prueba de **Chi-Cuadrado** (personalizando $k$ e $\alpha$).
   - `6`: Ejecutar prueba de **Kolmogorov-Smirnov** (personalizando $\alpha$).
   - `7`: Ejecutar prueba de **Autocorrelación** (personalizando $i$, $l$ e $\alpha$).
   - `8`: Ejecutar prueba de **Póker de 3 dígitos** (personalizando $\alpha$).
   - `9`: Ejecutar **Monte Carlo** para estimar $\pi$ con los números en memoria.
   - `10`: Ejecutar simulación de Monte Carlo a gran escala para ver la convergencia.
3. **Batería Completa**:
   - `11`: Ejecuta todas las pruebas en secuencia y genera un **Cuadro Resumen Global** con el estado final (Aceptada / Rechazada) de cada prueba.
