"""
Prueba de Uniformidad de Chi-Cuadrado (chi-cuadrado) con Interfaz Grafica.
Permite ingresar numeros manualmente y ver los resultados en una tabla y grafico.
"""

import math  # Funciones matematicas basicas
import tkinter as tk  # Libreria estandar para crear ventanas y botones
from tkinter import ttk, messagebox  # Tablas, selectores y mensajes emergentes
from tkinter.scrolledtext import ScrolledText  # Cuadro de texto para escribir


# ============================================================================
# FUNCIONES DE CALCULO
# ============================================================================


def obtener_chi_critico(gl, alpha=0.05):
    """
    Esta funcion actua como la tabla que viene al final de los libros.
    Nos da el numero limite (valor critico). Si nuestro calculo es menor
    o igual a este limite, significa que los numeros si son uniformes.
    """
    # Valores de referencia conocidos para un 95% de confianza (alpha = 0.05)
    tabla_005 = {
        1: 3.841,   2: 5.991,   3: 7.815,   4: 9.488,   5: 11.070,
        6: 12.592,  7: 14.067,  8: 15.507,  9: 16.919, 10: 18.307,
        11: 19.675, 12: 21.026, 13: 22.362, 14: 23.685, 15: 24.996,
        16: 26.296, 17: 27.587, 18: 28.869, 19: 30.144, 20: 31.410,
        21: 32.671, 22: 33.924, 23: 35.172, 24: 36.415, 25: 37.652,
        26: 38.885, 27: 40.113, 28: 41.337, 29: 42.557, 30: 43.773
    }

    # Valores de referencia para un 99% de confianza (alpha = 0.01)
    tabla_001 = {
        1: 6.635,   2: 9.210,   3: 11.345,  4: 13.277,  5: 15.086,
        6: 16.812,  7: 18.475,  8: 20.090,  9: 21.666, 10: 23.209,
        11: 24.725, 12: 26.217, 13: 27.688, 14: 29.141, 15: 30.578,
        16: 32.000, 17: 33.409, 18: 34.805, 19: 36.191, 20: 37.566,
        21: 38.932, 22: 40.289, 23: 41.638, 24: 42.980, 25: 44.314,
        26: 45.642, 27: 46.963, 28: 48.278, 29: 49.588, 30: 50.892
    }

    if abs(alpha - 0.01) < 0.005:
        if gl in tabla_001:
            return tabla_001[gl]
        z = 2.326348
    else:
        if gl in tabla_005:
            return tabla_005[gl]
        z = 1.644853

    # Formula de apoyo por si se usan mas de 30 intervalos
    factor = 1.0 - (2.0 / (9.0 * gl)) + z * math.sqrt(2.0 / (9.0 * gl))
    return gl * (factor ** 3)


def calcular_chi_cuadrado(numeros, k=10, alpha=0.05):
    """
    Aqui se hacen las cuentas de la prueba:
    1. Dividimos el rango de 0 a 1 en 'k' partes iguales (cajitas).
    2. Contamos cuantos numeros cayeron en cada cajita (frecuencia observada).
    3. Calculamos cuantos deberian haber caido si estuvieran parejos (frecuencia esperada).
    4. Comparamos las diferencias para ver si pasan la prueba.
    """
    n = len(numeros)
    if n == 0 or k < 2:
        return None

    # Paso 1: Contar cuantos numeros caen en cada intervalo
    observadas = [0] * k
    for num in numeros:
        # Multiplicamos por k para saber a que grupo pertenece
        posicion = int(num * k)
        if posicion >= k:
            posicion = k - 1
        elif posicion < 0:
            posicion = 0
        observadas[posicion] += 1

    # Paso 2: Lo que se esperaria de forma ideal en cada grupo
    esperada = n / k
    gl = k - 1  # Grados de libertad (grupos menos uno)

    # Paso 3: Comparar lo observado contra lo esperado
    filas = []
    chi_calc = 0.0

    for i in range(k):
        lim_inf = i / k
        lim_sup = (i + 1) / k
        oi = observadas[i]
        diff = oi - esperada
        # Formula: (Observado - Esperado)^2 / Esperado
        term = (diff ** 2) / esperada
        chi_calc += term

        filas.append({
            "clase": f"Clase {i + 1}",
            "rango": f"[{lim_inf:.2f} - {lim_sup:.2f})",
            "oi": oi,
            "ei": f"{esperada:.2f}",
            "diff": f"{diff:+.2f}",
            "term": f"{term:.4f}"
        })

    # Paso 4: Comparar contra el valor de la tabla
    chi_crit = obtener_chi_critico(gl, alpha)
    aceptada = chi_calc <= chi_crit

    return {
        "n": n,
        "k": k,
        "gl": gl,
        "esperada": esperada,
        "chi_calc": chi_calc,
        "chi_crit": chi_crit,
        "aceptada": aceptada,
        "filas": filas,
        "observadas": observadas
    }


def parsear_numeros_texto(texto):
    """
    Toma lo que el usuario escribio en el cuadro de texto y lo convierte
    en una lista limpia de numeros, ignorando comas o espacios.
    """
    # Reemplazamos comas y puntos y comas por espacios para separar facil
    partes = texto.replace(",", " ").replace(";", " ").split()
    numeros = []
    errores = []

    for item in partes:
        try:
            valor = float(item)
            # Solo aceptamos numeros entre 0 y 1
            if 0.0 <= valor <= 1.0:
                numeros.append(valor)
            else:
                errores.append(f"{item} (fuera de 0 a 1)")
        except ValueError:
            errores.append(f"{item} (no es un numero)")

    return numeros, errores


# ============================================================================
# INTERFAZ GRAFICA
# ============================================================================

class AppChiCuadrado(tk.Tk):
    """
    Ventana principal del programa. Organiza los botones, el cuadro de texto
    y la zona donde se muestran los resultados.
    """
    def __init__(self):
        super().__init__()
        self.title("Prueba de Uniformidad de Chi-Cuadrado")
        self.geometry("1100x740")
        self.minsize(980, 640)

        # Aplicamos un estilo visual limpio
        self.style = ttk.Style(self)
        try:
            self.style.theme_use("clam")
        except Exception:
            pass

        self._crear_interfaz()
        # Cargamos unos datos iniciales para que la pantalla no aparezca en blanco
        self.cargar_ejemplo_100()

    def _crear_interfaz(self):
        # Barra superior con titulo
        barra_superior = tk.Frame(self, bg="#1e293b", height=50)
        barra_superior.pack(fill=tk.X, side=tk.TOP)

        lbl_titulo = tk.Label(
            barra_superior,
            text="Prueba de Uniformidad de Chi-Cuadrado",
            font=("Segoe UI", 15, "bold"),
            bg="#1e293b",
            fg="#38bdf8"
        )
        lbl_titulo.pack(side=tk.LEFT, padx=16, pady=10)

        # Etiqueta que cuenta cuantos numeros llevamos escritos
        self.lbl_contador_muestra = tk.Label(
            barra_superior,
            text="Muestra: 0 numeros",
            font=("Segoe UI", 10, "bold"),
            bg="#334155",
            fg="#f8fafc",
            padx=12,
            pady=4
        )
        self.lbl_contador_muestra.pack(side=tk.RIGHT, padx=16, pady=10)

        # Contenedor central dividido en dos lados (izquierda y derecha)
        panel_dividido = tk.PanedWindow(self, orient=tk.HORIZONTAL, sashrelief=tk.RAISED, sashwidth=4)
        panel_dividido.pack(fill=tk.BOTH, expand=True, padx=10, pady=10)

        # -------------------------------------------------------------
        # LADO IZQUIERDO: DONDE EL USUARIO ESCRIBE LOS NUMEROS
        # -------------------------------------------------------------
        panel_izq = ttk.Frame(panel_dividido, padding=10)
        panel_dividido.add(panel_izq, minsize=380, width=420)

        lbl_paso1 = tk.Label(
            panel_izq,
            text="1. Ingresa los numeros :",
            font=("Segoe UI", 11, "bold"),
            fg="#0f172a"
        )
        lbl_paso1.pack(anchor="w", pady=(0, 4))

        lbl_instruccion = tk.Label(
            panel_izq,
            text="Escribe o pega tus datos entre 0 y 1, separados por comas o espacios:",
            font=("Segoe UI", 8),
            fg="#64748b",
            wraplength=380,
            justify="left"
        )
        lbl_instruccion.pack(anchor="w", pady=(0, 6))

        # Caja de texto editable
        self.txt_entrada = ScrolledText(panel_izq, wrap=tk.WORD, height=14, font=("Consolas", 10))
        self.txt_entrada.pack(fill=tk.BOTH, expand=True, pady=(0, 6))
        # Cuenta los numeros mientras el usuario escribe
        self.txt_entrada.bind("<KeyRelease>", self._al_escribir)

        # Botones para cargar ejemplo rapido o borrar todo
        fila_botones = ttk.Frame(panel_izq)
        fila_botones.pack(fill=tk.X, pady=(0, 10))

        btn_ejemplo = ttk.Button(fila_botones, text="Cargar 100 de Ejemplo", command=self.cargar_ejemplo_100)
        btn_ejemplo.pack(side=tk.LEFT, fill=tk.X, expand=True, padx=(0, 3))

        btn_limpiar = ttk.Button(fila_botones, text="Limpiar", command=self.limpiar_datos)
        btn_limpiar.pack(side=tk.LEFT, fill=tk.X, expand=True, padx=(3, 0))

        # Opciones de la prueba
        separador = ttk.Separator(panel_izq, orient=tk.HORIZONTAL)
        separador.pack(fill=tk.X, pady=8)

        lbl_paso2 = tk.Label(
            panel_izq,
            text="2. Opciones de la prueba:",
            font=("Segoe UI", 11, "bold"),
            fg="#0f172a"
        )
        lbl_paso2.pack(anchor="w", pady=(0, 6))

        frame_opciones = ttk.Frame(panel_izq)
        frame_opciones.pack(fill=tk.X, pady=(0, 10))

        # Seleccionar cuantos grupos o intervalos queremos
        ttk.Label(frame_opciones, text="Intervalos (k):").grid(row=0, column=0, sticky="w", pady=4)
        self.var_k = tk.IntVar(value=10)
        self.spin_k = ttk.Spinbox(frame_opciones, from_=2, to=30, textvariable=self.var_k, width=8, command=self.ejecutar_prueba)
        self.spin_k.grid(row=0, column=1, sticky="w", padx=6, pady=4)

        # Seleccionar nivel de error permitido (alpha)
        ttk.Label(frame_opciones, text="Nivel alpha:").grid(row=1, column=0, sticky="w", pady=4)
        self.var_alpha = tk.DoubleVar(value=0.05)
        combo_alpha = ttk.Combobox(
            frame_opciones,
            textvariable=self.var_alpha,
            values=[0.05, 0.01],
            width=6,
            state="readonly"
        )
        combo_alpha.grid(row=1, column=1, sticky="w", padx=6, pady=4)
        combo_alpha.bind("<<ComboboxSelected>>", lambda e: self.ejecutar_prueba())

        # Boton principal para ejecutar el calculo
        btn_calcular = tk.Button(
            panel_izq,
            text="CALCULAR PRUEBA DE CHI-CUADRADO",
            font=("Segoe UI", 10, "bold"),
            bg="#2563eb",
            fg="white",
            activebackground="#1d4ed8",
            activeforeground="white",
            relief=tk.FLAT,
            padx=10,
            pady=8,
            cursor="hand2",
            command=self.ejecutar_prueba
        )
        btn_calcular.pack(fill=tk.X, pady=(6, 0))

        # -------------------------------------------------------------
        # LADO DERECHO: DONDE SE MUESTRAN LOS RESULTADOS
        # -------------------------------------------------------------
        panel_der = ttk.Frame(panel_dividido, padding=10)
        panel_dividido.add(panel_der, minsize=500)

        # Recuadro grande que muestra si se acepta o se rechaza
        self.frame_veredicto = tk.Frame(panel_der, bg="#e2e8f0", padx=12, pady=10)
        self.frame_veredicto.pack(fill=tk.X, pady=(0, 10))

        self.lbl_veredicto_titulo = tk.Label(
            self.frame_veredicto,
            text="Ingresa datos para evaluar la prueba",
            font=("Segoe UI", 12, "bold"),
            bg="#e2e8f0",
            fg="#1e293b"
        )
        self.lbl_veredicto_titulo.pack(anchor="w")

        self.lbl_veredicto_detalle = tk.Label(
            self.frame_veredicto,
            text="",
            font=("Segoe UI", 9),
            bg="#e2e8f0",
            fg="#475569"
        )
        self.lbl_veredicto_detalle.pack(anchor="w", pady=(2, 0))

        # Tabla con las columnas de calculo
        lbl_titulo_tabla = tk.Label(
            panel_der,
            text="Tabla de frecuencias observadas vs esperadas:",
            font=("Segoe UI", 10, "bold"),
            fg="#0f172a"
        )
        lbl_titulo_tabla.pack(anchor="w", pady=(0, 4))

        columnas = ("clase", "rango", "oi", "ei", "diff", "term")
        self.tree = ttk.Treeview(panel_der, columns=columnas, show="headings", height=8)

        self.tree.heading("clase", text="Clase")
        self.tree.heading("rango", text="Rango [a, b)")
        self.tree.heading("oi", text="Observada (Oi)")
        self.tree.heading("ei", text="Esperada (Ei)")
        self.tree.heading("diff", text="Diferencia (Oi - Ei)")
        self.tree.heading("term", text="(Oi - Ei)^2 / Ei")

        self.tree.column("clase", width=80, anchor="center")
        self.tree.column("rango", width=120, anchor="center")
        self.tree.column("oi", width=95, anchor="center")
        self.tree.column("ei", width=95, anchor="center")
        self.tree.column("diff", width=120, anchor="center")
        self.tree.column("term", width=110, anchor="center")

        barra_desplazar = ttk.Scrollbar(panel_der, orient=tk.VERTICAL, command=self.tree.yview)
        self.tree.configure(yscrollcommand=barra_desplazar.set)

        self.tree.pack(fill=tk.BOTH, expand=True, side=tk.TOP)
        barra_desplazar.pack(side=tk.RIGHT, fill=tk.Y, before=self.tree)

        # Grafico de barras dibujado directamente en pantalla
        lbl_titulo_grafico = tk.Label(
            panel_der,
            text="Grafico de barras (La linea roja muestra el valor esperado):",
            font=("Segoe UI", 10, "bold"),
            fg="#0f172a"
        )
        lbl_titulo_grafico.pack(anchor="w", pady=(8, 2))

        self.canvas_chart = tk.Canvas(panel_der, height=150, bg="#0f172a", highlightthickness=0)
        self.canvas_chart.pack(fill=tk.X, expand=False, pady=(0, 4))

    def _al_escribir(self, event=None):
        """Actualiza el contador de numeros en la esquina mientras se teclea."""
        texto = self.txt_entrada.get("1.0", tk.END).strip()
        nums, _ = parsear_numeros_texto(texto)
        self.lbl_contador_muestra.config(text=f"Muestra: {len(nums)} numeros")

    def cargar_ejemplo_100(self):
        """Rellena el cuadro de texto con 100 numeros de prueba automaticamente."""
        a = 1103515245
        c = 12345
        m = 2147483648
        actual = 12345
        datos = []
        for _ in range(100):
            actual = (a * actual + c) % m
            datos.append(f"{actual / m:.5f}")

        # Escribimos los numeros en la caja de texto
        self.txt_entrada.delete("1.0", tk.END)
        self.txt_entrada.insert(tk.END, ", ".join(datos))
        self._al_escribir()
        self.ejecutar_prueba()

    def limpiar_datos(self):
        """Borra el texto escrito y reinicia la pantalla."""
        self.txt_entrada.delete("1.0", tk.END)
        self._al_escribir()
        self.tree.delete(*self.tree.get_children())
        self.canvas_chart.delete("all")
        self.frame_veredicto.config(bg="#e2e8f0")
        self.lbl_veredicto_titulo.config(text="Ingresa datos para evaluar la prueba", bg="#e2e8f0", fg="#1e293b")
        self.lbl_veredicto_detalle.config(text="", bg="#e2e8f0")

    def ejecutar_prueba(self):
        """Lee los datos del cuadro de texto, hace los calculos y muestra los resultados."""
        texto = self.txt_entrada.get("1.0", tk.END).strip()
        if not texto:
            return

        numeros, errores = parsear_numeros_texto(texto)

        # Si no hay numeros validos, avisamos al usuario
        if len(numeros) == 0:
            messagebox.showerror(
                "Error en los datos",
                "No se encontraron numeros validos entre 0 y 1.\n"
                "Ejemplo de formato correcto: 0.12, 0.45, 0.78"
            )
            return

        k = self.var_k.get()
        if k < 2:
            k = 2
            self.var_k.set(2)

        alpha = self.var_alpha.get()
        res = calcular_chi_cuadrado(numeros, k=k, alpha=alpha)
        if not res:
            return

        # Actualizar el recuadro de veredicto (Verde si pasa, Rojo si no pasa)
        if res["aceptada"]:
            self.frame_veredicto.config(bg="#dcfce7")
            self.lbl_veredicto_titulo.config(
                text="[ACEPTADA] Se acepta H0: Los numeros son UNIFORMES U(0,1)",
                bg="#dcfce7",
                fg="#15803d"
            )
            self.lbl_veredicto_detalle.config(
                text=f"Chi calculado = {res['chi_calc']:.4f} <= Chi critico = {res['chi_crit']:.4f} | Grados de libertad = {res['gl']}",
                bg="#dcfce7",
                fg="#166534"
            )
        else:
            self.frame_veredicto.config(bg="#fee2e2")
            self.lbl_veredicto_titulo.config(
                text="[RECHAZADA] Se rechaza H0: Los numeros NO son uniformes",
                bg="#fee2e2",
                fg="#b91c1c"
            )
            self.lbl_veredicto_detalle.config(
                text=f"Chi calculado = {res['chi_calc']:.4f} > Chi critico = {res['chi_crit']:.4f} | Grados de libertad = {res['gl']}",
                bg="#fee2e2",
                fg="#991b1b"
            )

        # Llenar la tabla con los datos de cada clase
        self.tree.delete(*self.tree.get_children())
        for fila in res["filas"]:
            self.tree.insert("", tk.END, values=(
                fila["clase"], fila["rango"], fila["oi"], fila["ei"], fila["diff"], fila["term"]
            ))

        # Fila final de totales
        self.tree.insert(
            "",
            tk.END,
            values=("TOTAL", f"N = {res['n']}", sum(res["observadas"]), f"{res['esperada']*k:.1f}", "0.00", f"Total={res['chi_calc']:.4f}")
        )

        # Dibujar las barras en el grafico
        self._dibujar_histograma(res["observadas"], res["esperada"])

    def _dibujar_histograma(self, observadas, esperada):
        """Dibuja las barras azules y la linea punteada roja del valor esperado."""
        self.canvas_chart.delete("all")
        self.update_idletasks()

        ancho = self.canvas_chart.winfo_width()
        alto = self.canvas_chart.winfo_height()
        if ancho < 50 or alto < 50:
            ancho, alto = 500, 150

        k = len(observadas)
        max_valor = max(max(observadas), esperada * 1.25, 1)

        margen_x = 40
        margen_y = 25
        area_w = ancho - 2 * margen_x
        area_h = alto - 2 * margen_y

        ancho_barra = area_w / k

        # Dibujar cada barra
        for i, oi in enumerate(observadas):
            altura_barra = (oi / max_valor) * area_h
            x0 = margen_x + i * ancho_barra + 3
            x1 = margen_x + (i + 1) * ancho_barra - 3
            y0 = alto - margen_y - altura_barra
            y1 = alto - margen_y

            self.canvas_chart.create_rectangle(x0, y0, x1, y1, fill="#38bdf8", outline="#0284c7")

            # Numero arriba de la barra
            self.canvas_chart.create_text(
                (x0 + x1) / 2, y0 - 8,
                text=str(oi),
                fill="#f8fafc",
                font=("Segoe UI", 8, "bold")
            )

            # Nombre de la clase abajo
            self.canvas_chart.create_text(
                (x0 + x1) / 2, alto - margen_y + 12,
                text=f"C{i+1}",
                fill="#94a3b8",
                font=("Segoe UI", 7)
            )

        # Linea roja horizontal que marca la frecuencia esperada
        y_esperada = alto - margen_y - ((esperada / max_valor) * area_h)
        self.canvas_chart.create_line(
            margen_x - 5, y_esperada, ancho - margen_x + 5, y_esperada,
            fill="#ef4444", width=2, dash=(4, 3)
        )
        self.canvas_chart.create_text(
            margen_x + 35, y_esperada - 8,
            text=f"Esperada = {esperada:.1f}",
            fill="#f87171",
            font=("Segoe UI", 8, "bold")
        )


# ============================================================================
# INICIO DEL PROGRAMA
# ============================================================================
if __name__ == "__main__":
    app = AppChiCuadrado()
    app.mainloop()