# pro_2_taller_digitales

Sistema embebido multijugador sobre FPGA — EL3313 Taller de Diseño Digital, I Semestre 2026.

**Repositorio:** [github.com/Jesushuertas237/pro_2_taller_digitales](https://github.com/Jesushuertas237/pro_2_taller_digitales)
**Video demostrativo:** [youtu.be/mONFn-cRt2o](https://youtu.be/mONFn-cRt2o?si=y0LoSSXbgSsJOHT9)

Autores:
Andrea García borges
Milagro Rojas Sanches
Jesús Huertas Rojas
Brayan Solis Rojas 

Videojuego Pong para dos jugadores implementado en C sobre un procesador MicroBlaze RISC-V
en modo bare-metal, corriendo en una Nexys A7. Incluye salida VGA, comunicación SPI entre
dos FPGAs, y carga de recursos gráficos desde microSD.

**Herramientas:** Vivado 2025.1 · Nexys A7 (Artix-7 xc7a100t) · [Hog](https://hog.web.cern.ch/) (gestión de proyecto Vivado)

---

## Diagrama de bloques del sistema

El diagrama completo del sistema en Vivado IP Integrator puede verse en:
[`docs/block_diagram.pdf`](docs/block_diagram.pdf)

## Descripción del proyecto

El sistema implementa el videojuego Pong sobre un procesador MicroBlaze RISC-V corriendo en
modo bare-metal en una FPGA Nexys A7. El firmware escribe directamente a una VRAM ubicada en
BRAM, la cual es leída por un controlador VGA personalizado para generar la señal de video a
640×480 @ 60 Hz. Las imágenes del menú se cargan desde una microSD al inicio. La comunicación
multijugador se realiza mediante SPI bit-bang por GPIO sobre el conector PMOD JA, permitiendo
que dos FPGAs intercambien el estado del juego en tiempo real.

A diferencia de un flujo manual de Vivado, este repositorio usa **Hog** (CERN) como framework
de gestión del proyecto: el hardware (block design, IPs, constraints) se reconstruye de forma
reproducible a partir de listas de archivos versionadas en Git, en lugar de depender de un
`.xpr` o `.xsa` binario.

---

## Estructura del repositorio

```
pro_2_taller_digitales/
├── Hog/                          submódulo git (framework Hog, CERN)
├── Top/
│   └── pro_2_taller_digitales/
│       ├── hog.conf              parte/dispositivo objetivo (xc7a100tcsg324-1)
│       └── list/
│           ├── design.src        lista de fuentes HDL/IP del block design
│           └── constraints.con   lista de archivos de constraints (XDC)
├── src/                          firmware en C (aplicación bare-metal MicroBlaze)
│   ├── main.c                    máquina de estados principal del juego
│   ├── vga.c / vga.h             escritura al framebuffer en DDR2
│   ├── font.c / font.h           renderizado de texto y sprites
│   ├── sd_image.c / sd_image.h   carga de imágenes desde microSD al inicio
│   ├── input.c / input.h         lectura de switches y botones
│   ├── ui_text.c / ui_text.h     menús y pantallas de texto
│   ├── pong_game.c               lógica del juego single-player
│   ├── pong_paddles.c            física de paletas y pelota (modo local)
│   ├── pong_mp.c                 modo multijugador vía SPI
│   ├── spi.c / spi.h             driver SPI bit-bang sobre GPIO (PMOD JA)
│   └── lscript.ld                linker script de la aplicación
├── Imagenes_interfaz/
│   ├── MENU.BIN                  imagen del menú (480×480, 4bpp), va en la microSD
│   └── FIELD.BIN                 imagen del campo de juego (480×480, 4bpp), va en la microSD
├── docs/                         documentación TerosHDL del IP de VGA
├── .gitmodules                   referencia al submódulo Hog
└── .gitignore
```

---
## Reporte de utilización de recursos

Dispositivo: `xc7a100tcsg324-1` (Nexys A7-100T)

| Recurso | Usado | Disponible | Utilización |
|---|---|---|---|
| Slice LUTs | 16,829 | 63,400 | 26.5% |
| Slice Registers | 20,351 | 126,800 | 16.1% |
| Block RAM (tiles) | 68 | 135 | 50.4% |
| DSP Slices | 1 | 240 | 0.4% |
| IOBs | 108 | 210 | 51.4% |

> El uso elevado de BRAM (50.4%) corresponde principalmente a la VRAM del controlador VGA
> (64 de los 68 tiles utilizados).

---

## Reporte de timing

Reporte generado desde el diseño implementado (`impl_1`).

| Métrica | Valor |
|---|---|
| WNS (Worst Negative Slack) | +1.272 ns |
| TNS Failing Endpoints | 0 |
| WHS (Worst Hold Slack) | +0.052 ns |
| THS Failing Endpoints | 0 |
| Frecuencia principal (AXI/CPU) | 100 MHz |
| Frecuencia dominio VGA | 25 MHz |

**Todos los constraints de timing se cumplen.**
## Componentes principales del sistema

| Bloque | Función |
|---|---|
| MicroBlaze RISC-V | Procesador principal, ejecuta el firmware desde DDR2 |
| MIG 7 Series (DDR2) | Memoria externa — firmware, framebuffer, variables del juego |
| top_vga (IP custom) | Controlador VGA con VRAM AXI4, genera señal 640×480 @ 60 Hz |
| AXI GPIO ×4 | LEDs, botones, switches, SPI bit-bang (PMOD JA) |
| AXI Quad SPI | Comunicación con microSD |
| Clocking Wizard | Genera 100 MHz (AXI) y 25 MHz (dominio VGA) |

### Arquitectura del firmware

```
main.c
├── vga.c          escritura al framebuffer en DDR2
├── font.c         renderizado de texto y sprites
├── sd_image.c     carga de imágenes desde microSD al inicio
├── input.c        lectura de switches y botones
├── ui_text.c      menús y pantallas de texto
├── pong_game.c    lógica del juego single-player (IA)
├── pong_paddles.c física de paletas y pelota
├── pong_mp.c      modo multijugador vía SPI
└── spi.c          driver SPI bit-bang sobre GPIO PMOD JA
```

---

## Documentación del IP de VGA

La documentación generada con TerosHDL de cada módulo del IP personalizado
está disponible en la carpeta [`docs/`](docs/):

- [`top.html`](docs/top.html) — módulo top del IP VGA
- [`axi_vram_ctrl.html`](docs/axi_vram_ctrl.html) — controlador AXI de la VRAM
- [`vram.html`](docs/vram.html) — memoria de video (BRAM)
- [`vga_controller.html`](docs/vga_controller.html) — generador de señal VGA
- [`h_sync_gen.html`](docs/h_sync_gen.html) — sincronización horizontal
- [`v_sync_gen.html`](docs/v_sync_gen.html) — sincronización vertical
- [`pixel_mux.html`](docs/pixel_mux.html) — multiplexor de píxeles RGB

---

## Instrucciones de reproducibilidad

### Prerrequisitos

- Vivado 2025.1
- Nexys A7-100T con cable USB-JTAG
- MicroSD con las imágenes [`MENU.BIN`](Imagenes_interfaz/MENU.BIN) y [`FIELD.BIN`](Imagenes_interfaz/FIELD.BIN) (incluidas en este repo, en `Imagenes_interfaz/`)
- Git (con soporte para submódulos)

### Paso 1 — Clonar el repositorio (incluyendo el submódulo Hog)

```bash
git clone --recurse-submodules https://github.com/Jesushuertas237/pro_2_taller_digitales.git
cd pro_2_taller_digitales
```

> Si ya clonaste sin `--recurse-submodules`, inicializa el submódulo aparte:
> ```bash
> git submodule update --init --recursive
> ```

### Paso 2 — Generar el proyecto de Vivado con Hog

Hog reconstruye el proyecto Vivado a partir de las listas de archivos en
[`Top/pro_2_taller_digitales/list/`](Top/pro_2_taller_digitales/list/) (`design.src`,
`constraints.con`) y la configuración de parte en
[`hog.conf`](Top/pro_2_taller_digitales/hog.conf) (`xc7a100tcsg324-1`).

Desde la raíz del repositorio:

```bash
./Hog/CreateProject.sh pro_2_taller_digitales
```

Esto crea el proyecto Vivado dentro de `Projects/` (ignorado por `.gitignore`) con el block
design, los IPs y los constraints ya cargados.

> **Nota:** el IP de VGA (`top_vga`) es parte del block design y se reconstruye junto con el
> resto del proyecto al correr el script de Hog.

### Paso 3 — Generar el bitstream

1. Abrir el proyecto generado en `Projects/pro_2_taller_digitales/` con Vivado
2. `Flow > Generate Bitstream`

### Paso 4 — Compilar y cargar el firmware

1. Exportar el hardware (XSA con bitstream incluido) desde Vivado, o usar el flujo de
   Vitis equivalente para crear la plataforma a partir del diseño exportado
2. Crear el *application component* en Vitis y copiar todos los archivos de
   [`src/`](src/) (excepto `README.txt`) al componente de la aplicación
3. Compilar (**Build**) y programar (**Run**) — Vitis programa el bitstream, descarga el
   ELF al MicroBlaze y ejecuta automáticamente

### Paso 5 — Preparar la microSD

Copiar [`MENU.BIN`](Imagenes_interfaz/MENU.BIN) y [`FIELD.BIN`](Imagenes_interfaz/FIELD.BIN)
a la raíz de una microSD e insertarla en la Nexys A7 antes de encender la placa.

---

## Controles y flujo del juego

Al encender la FPGA aparece el menú principal con dos opciones:

### Modo dos jugadores local (una sola FPGA)

Activar **SW0** desde el menú para iniciar.

| Jugador | Arriba | Abajo |
|---|---|---|
| Jugador 1 (paleta izquierda) | BTNL | BTND |
| Jugador 2 (paleta derecha) | BTNU | BTNR |

**BTNC** — salir al menú en cualquier momento.

### Modo multijugador SPI (dos FPGAs)

Activar **SW1** desde el menú para entrar al modo multijugador.

1. Cada FPGA selecciona su rol con **SW15**: `ON` = Maestro · `OFF` = Esclavo
2. **Primero** el Esclavo presiona **BTNC** para quedar en espera
3. **Luego** el Maestro presiona **BTNC** — se realiza el handshake SPI automáticamente
4. Si el handshake falla, ambas FPGAs regresan al menú

Durante el juego multijugador cada jugador controla su paleta con **BTNU** (arriba) y **BTND** (abajo). **BTNC** cancela y regresa al menú.

> **Conexión física PMOD JA:** directa pin a pin — JA1↔JA1, JA2↔JA2, JA3↔JA3, JA4↔JA4, GND↔GND.
> Esto funciona porque el GPIO es bidireccional: el software asigna la dirección de cada pin
> (entrada o salida) según el rol seleccionado con SW15, sin que haya conflicto en el cable.

---

## Curso

**EL3313 Taller de Diseño Digital** · I Semestre 2026
Profesor: Luis G. León-Vega, Ph.D
Instituto Tecnológico de Costa Rica


## Uso de IA CHATS


Se permitió el uso de herramientas de IA como apoyo para comprensión, generación de ideas y mejora de redacción. La implementación, validación y resultados son responsabilidad del estudiante.

En caso de uso, se adjuntan las conversaciones como evidencia. El funcionamiento del sistema puede demostrarse mediante pruebas, garantizando la comprensión de los conceptos y la correcta operación de la solución.



https://claude.ai/share/47a18f3e-206b-4914-80dc-3497f4c43172
https://claude.ai/share/979474ea-2fcb-4cd4-9cf8-ce7f98b9a18b
https://claude.ai/share/11125d84-e35a-4d4f-898d-753fbe913bbb
https://claude.ai/share/07b96225-a7ce-48d3-b2bb-f3e8e11b6109
https://claude.ai/share/f6872f14-054a-406c-a57f-67909a4b624f
https://claude.ai/share/51b78f57-b6e2-4805-b641-179f4138e1f5
https://claude.ai/share/2724954d-40fd-48d1-8792-1a4e72a1d6ba
https://claude.ai/share/ad5a239d-40d1-47b3-b579-d5d5dd8e109c
