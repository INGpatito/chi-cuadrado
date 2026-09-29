CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2
LDFLAGS = -lm
GUI_LDFLAGS = -lSDL2 -lm

# Fuentes comunes para la lógica de pruebas
COMMON_SRC = src/generator.c src/uniformidad.c src/independencia.c src/poker.c src/montecarlo.c
COMMON_OBJ = $(COMMON_SRC:.c=.o)

# Objetivos ejecutables
CLI_TARGET = pruebas_simulacion
GUI_TARGET = pruebas_gui

all: $(CLI_TARGET) $(GUI_TARGET)

# Versión de consola interactiva (cero dependencias externas)
cli: $(CLI_TARGET)

$(CLI_TARGET): src/main.o $(COMMON_OBJ)
	$(CC) src/main.o $(COMMON_OBJ) -o $@ $(LDFLAGS)

# Versión con Interfaz Gráfica de Escritorio (Nuklear + SDL2)
gui: $(GUI_TARGET)

$(GUI_TARGET): src/gui/gui_main.o $(COMMON_OBJ)
	$(CC) src/gui/gui_main.o $(COMMON_OBJ) -o $@ $(GUI_LDFLAGS)

src/%.o: src/%.c src/common.h
	$(CC) $(CFLAGS) -c $< -o $@

src/gui/%.o: src/gui/%.c src/common.h src/gui/nuklear.h src/gui/nuklear_sdl_renderer.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f src/*.o src/gui/*.o $(CLI_TARGET) $(GUI_TARGET)

.PHONY: all cli gui clean
