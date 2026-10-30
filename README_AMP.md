# Xinu ESP32-S3 AMP: base de trabajo

Este árbol es una **copia de trabajo AMP-only** de `s3-xinu`. El árbol
`/home/daniel/s3-xinu` sigue siendo la referencia bare-metal y no se modifica.

Objetivo: enlazar Xinu como **payload independiente**, arrancarlo desde una app
de ESP-IDF en el **core 1** y dejar el core 0 para el loader y, más adelante,
esp-hosted.

Ya no hay camino bare-metal ni imagen `xinu.boot`: `make kernel` genera siempre
`compile/xinu.img` para el loader IDF.

## Documentación

| Documento | Contenido |
|---|---|
| [docs/MANUAL-LOADER.md](docs/MANUAL-LOADER.md) | Loader IDF, particiones, `xinu.img`, `bootinfo` v7, salto CALL0, XIP y PSRAM |
| [docs/MANUAL-AMP.md](docs/MANUAL-AMP.md) | Kernel AMP, arranque, memoria, módulos, comandos, portabilidad y JVM sin GC |
| [docs/MANUAL-NATIVES.md](docs/MANUAL-NATIVES.md) | Librerías Java, `java_lib`, natives C, apps y Makefile |
| [docs/INVENTARIO-AMP.md](docs/INVENTARIO-AMP.md) | Inventario generado de todos los ficheros |
| [web/index.html](web/index.html) | Dashboard servido desde la SD |

Los HTML se generan con `make web`.

## Estado actual

- `make kernel` genera `compile/xinu.img` (binario crudo) y `compile/xinu.elf`.
- El loader mapea la partición `xinu` en `0x42100000` y salta a
  `0x42100440` (`_start`).
- `.vector` está enlazada al principio de la ventana XIP (`0x42100000`) y
  **se ejecuta desde XIP**. El loader actual no la copia a IRAM ni publica
  `vecbase` en `bootinfo`.
- `.init/.text/.slowtext` van en XIP.
- `.rodata/.data/.bss/.stacks` van en PSRAM.
- Hay **un solo heap**, en PSRAM. `memget`, `memfree` y `stkget` son la base;
  `malloc`, `free`, `calloc` y `realloc` (libxc) usan esa misma lista libre.
- No quedan `psram_malloc`, `psram_memget`, `psram_stkget`, `psramInit` ni el
  heap interno de DRAM.
- El contrato es **bootinfo v7** (`include/bootinfo.h`), solo datos:
  `magic`, `version`, `size`, `ccount_hz`, `core`, `flags`, `psram_va`,
  `psram_size`.
- El loader IDF está en `/home/daniel/esp32-xinu-loader`.
- Probado en `/dev/ttyACM0` (ESP32-S3 N16R8, 16 MB flash, 8 MB PSRAM octal)
  con `xinu-test.conf`: shell, `help`, `free`, `mem`, `psram`, `psram -t`,
  `ps`, `sleep 2` y `echo` funcionan.
- La imagen normal compila, pero no se puede arrancar en esa placa porque no
  tiene SD/TFT/W5500.
- La JVM sigue necesitando FAT/SD; las clases no están embebidas.
- `date` sigue mostrando 1900: el RTC no se inicializa.

## Compilar

Atajos desde la raíz:

```sh
make kernel S3_CONFIG=platforms/esp32s3/xinu-test.conf
make loader
make web
```

El target `kernel` borra `include/conf.h` y `system/conf.c`, ejecuta `clean` y
recompila. El resultado es `compile/xinu.img`.

Equivalente manual:

```sh
cd /home/daniel/xinu-s3-amp
rm -f include/conf.h system/conf.c
cd compile
source ../env.sh
make clean
make -j4 CONFIG=platforms/esp32s3/xinu-test.conf
```

Configuración normal (SD/TFT/W5500/JVM):

```sh
make kernel S3_CONFIG=platforms/esp32s3/xinu.conf
```

`xinu-test.conf` mapea `SD0`, `TFT0`, `BTN0`, `ETH0` y `RGB0` a `null`. Al
cambiar de configuración hay que borrar `include/conf.h` y `system/conf.c`
para forzar la regeneración; `make kernel` ya lo hace.

## Flashear

Loader IDF, tabla de particiones y app del core 0:

```sh
make loader-flash S3_PORT=/dev/ttyACM0
```

Payload Xinu (la partición `xinu` empieza en `0x100000`):

```sh
make flash S3_PORT=/dev/ttyACM0
```

o directamente:

```sh
cd /home/daniel/xinu-s3-amp/compile
source ../env.sh
esptool.py --chip esp32s3 -p /dev/ttyACM0 -b 921600 \
  write_flash 0x100000 xinu.img
```

`/dev/ttyACM0` es un puente CH343 USB-UART conectado a UART0. El loader y Xinu
comparten esa UART0; el loader la silencia antes del handoff.

No uses `idf.py flash` para `xinu.img`: no es una app IDF.

## Memoria

| Región | Dirección | Uso |
|---|---|---|
| Flash XIP | `0x42100000` | `.vector`, `.init`, `.text`, `.slowtext` |
| PSRAM Xinu | `0x3D801000` + `0x7DF000` | `.rodata`, `.data`, `.bss`, `.stacks`, heap |
| Heap | después de `_end` | único allocator de Xinu |

El linker reserva `.stacks` justo antes de `_end`:

- 8 KiB para la pila de pánico (`_stacks_start .. _panicstk_top`),
- 8 KiB para la pila del hilo nulo (`_panicstk_top .. _nullstk_top`).

`start.S` pone `sp = _nullstk_top`; `panicstk` apunta a `_panicstk_top`. El heap
empieza en `_end`, así que no puede solaparse con ninguna de las dos pilas.

### Imagen de prueba actual (`xinu-test.conf`)

| Dato | Valor |
|---|---:|
| `_xt_vector_table` | `0x42100000` |
| `_start` | `0x42100440` |
| `_etext` | `0x4211ECF5` |
| `_stacks_start` | `0x3D818C60` |
| `_end` / heap base | `0x3D81CC60` |
| Ventana PSRAM | `0x3D801000 .. 0x3DFE0000` |
| Heap total | 8.139.680 B (`0x7C33A0`) |

Los valores cambian con la configuración. Compruébalos siempre con:

```sh
cd compile
xtensa-esp32s3-elf-nm -n xinu.elf | grep -E ' (_start|_xt_vector_table|_etext|_stacks_start|_end)$'
```

### Por qué PSRAM no está en `0x3C000000`

La ventana de **datos de flash** de IDF (`.flash.rodata`) está en
`0x3C000000`; en el build actual del loader llega hasta `0x3C0A...`. La PSRAM
se mapea en la ventana de datos externa **`0x3D800000..0x3E000000`**. Usar
`0x3C000000` para Xinu pisaría el rodata del loader.

El loader reserva casi toda la PSRAM con `heap_caps_malloc()` y deja los
metadatos del heap de IDF en `0x3D800000`. Xinu empieza una página después,
en `0x3D801000`, para no tocar ese primer bloque.

## Un solo allocator

La lista libre es `memlist`, definida en `system/heap.c`. Todos estos caminos
terminan en ella:

```text
memget() / memfree() / stkget()        (API del kernel)
        ^
        |
malloc() / free() / calloc() / realloc()   (libxc, API estándar)

JVM          -> malloc/calloc/realloc/free
red (bfp)    -> memget/memfree
memtty       -> memget
webterm      -> memget/memfree
wget         -> malloc/free
```

`memget()` solo toca el heap de PSRAM. `memfree()` comprueba que el puntero
esté dentro de `heap_base..heap_end`. No hay despacho entre DRAM y PSRAM porque
ya no existe un heap de DRAM.

La shell informa del estado con:

- `psram`: ventana PSRAM, base/tamaño del heap, uso, libre y bloque mayor.
- `psram -t`: además ejecuta un autotest de `malloc/calloc/realloc/free`.
- `free`: resumen de imagen, stacks, heap y buffer pools.
- `mem`: el antiguo `memstat`; no debe mostrar áreas negativas.
- `ps`: CPU% por tarea y pico/total de pila.
- `stack`: detalle de pila por tarea; `stack -r [tid]` reinicia los picos.

## Tabla de vectores y VECBASE

En esta revisión la tabla `.vector` está enlazada al principio de la ventana XIP
(`0x42100000`) y **se ejecuta desde flash**. `start.S` hace:

```asm
movi a2, _xt_vector_table
wsr  a2, vecbase
```

No hay copia a IRAM, no hay `bootinfo.vecbase` y el loader no reserva una
página para vectores. Ejecutar la tabla desde PSRAM requeriría mapearla en el
bus de instrucciones (`MMU_TARGET_PSRAM0`), habilitar I-cache y validar las
excepciones; no está implementado.

`.slowtext` no tiene una región aparte: el linker la coloca dentro de `.text`
en XIP.

## bootinfo v7

`struct xinu_bootinfo` es solo datos; no cruza punteros a funciones ni objetos
del ABI de IDF. Campos:

| Offset | Campo | Uso |
|---:|---|---|
| 0 | `magic` | `XINU_BOOT_MAGIC` (`0x58494E55`) |
| 4 | `version` | 7 |
| 8 | `size` | `sizeof(struct xinu_bootinfo)` |
| 12 | `ccount_hz` | frecuencia real de CCOUNT |
| 16 | `core` | core de Xinu (1) |
| 20 | `flags` | PSRAM mapeada, consola, AMP, XIP |
| 24 | `psram_va` | base de la ventana PSRAM de Xinu |
| 28 | `psram_size` | tamaño de la ventana |

El entry point **no** viaja en `bootinfo`: el loader actual salta a
`XINU_IMAGE_XIP_BASE + 0x440`. `platforminit()` valida `magic`, `version`,
`size`, `ccount_hz` y `psram_size`.

## Qué hace el loader

1. Busca la partición `xinu` (offset `0x100000`, 2 MiB).
2. Intenta actualizar desde `/xinu.img` en la SD, si existe.
3. Reserva casi toda la PSRAM dejando los metadatos de IDF.
4. Mapea la partición como instrucciones en `0x42100000`.
5. Rellena `struct xinu_bootinfo` v7.
6. Crea una tarea en el core 1 y espera a que esté lista.
7. `xinu_jump.S` desactiva interrupciones, pasa `bootinfo` en `a2` y salta al
   entry `0x42100440`.
8. `start.S` copia `.rodata`/`.data` desde XIP a PSRAM, ceroa `.bss`/`.stacks`,
   pone `sp = _nullstk_top` y llama a `nulluser()`.

El core 0 no vuelve a tocar PSRAM ni UART0. Xinu no llama a `psramInit()`:
IDF ya mapeó la die y el loader ya reservó la ventana.

## Verificación rápida de `xinu.img`

En el camino activo, `xinu.img` es un **binario crudo**:

```sh
cd /home/daniel/xinu-s3-amp/compile
ls -l xinu.img
xxd -l 32 xinu.img
```

Debe empezar con la tabla de vectores, no con `XIMG`. El formato empaquetado
`XIMG` de `tools/mkxinuimg.py` e `include/xinu_image.h` **no está activo**; el
loader actual no lo parsea.

Para comprobar el ELF:

```sh
xtensa-esp32s3-elf-nm -n xinu.elf | grep -E ' (_start|_xt_vector_table|_etext|_stacks_start|_end)$'
xtensa-esp32s3-elf-objdump -h xinu.elf | sed -n '1,25p'
```

Debe verse `_xt_vector_table` en `0x42100000`, `_start` en `0x42100440`,
`.rodata`/`.data` en `0x3D801000` o superior y `.stacks` de `0x4000` bytes antes
de `_end`.

## Compilar el loader

```sh
cd /home/daniel/esp32-xinu-loader
./build.sh build
```

`build.sh` fija:

```sh
IDF_PATH=/home/daniel/esp-idf
IDF_PYTHON_ENV_PATH=/home/daniel/esp-idf-py
IDF_TOOLS_PATH=/home/daniel/.espressif
IDF_PYTHON_CHECK_CONSTRAINTS=no
IDF_MAINTAINER=1
```

`IDF_MAINTAINER=1` es necesario porque el toolchain de `/opt` es
`esp-15.2.0_20251204` y ESP-IDF 5.5.1 espera `esp-14.2.0_20241119`.

Configuración relevante:

- `sdkconfig.defaults`: `CONFIG_SPIRAM_MODE_OCT=y`, consola UART0.
- `sdkconfig` activo para la placa de pruebas: 16 MB de flash, UART0.
- `CONFIG_SPIRAM_USE_CAPS_ALLOC=y`.
- `CONFIG_SPIRAM_MEMTEST=n`.
- `CONFIG_ESP_INT_WDT=n`.
- `CONFIG_ESP_TASK_WDT_CHECK_IDLE_TASK_CPU1=n`.

Si cambias `include/bootinfo.h` o `include/xinu_image.h`, sincroniza las copias:

```sh
make loader-sync
```

## Riesgos abiertos

- La imagen normal (SD/TFT/W5500) no se puede validar en la placa de pruebas
  porque no tiene esos periféricos; se usa `xinu-test.conf`.
- La JVM necesita FAT/SD; en la placa de pruebas no arranca.
- `date` muestra 1900 porque el RTC no se inicializa.
- La ejecución desde PSRAM no está implementada; requiere mapeo I-bus,
  I-cache y validación de excepciones.
- El mapeo XIP es fijo: si `esp_partition_mmap()` no devuelve `0x42100000`,
  el loader aborta.
- `IDF_MAINTAINER=1` oculta un desajuste de toolchain; no es una configuración
  de producción.
- El servicio core 0 ↔ core 1 no existe todavía.
- No hay actualización A/B, CRC ni rollback en la actualización por SD.
- El loader no es un repositorio git; sus cambios viven solo en el árbol de
  trabajo.
