# Manual del Xinu Loader — ESP32-S3 AMP

Este manual describe el **loader de ESP-IDF** que arranca Xinu en el núcleo 1
del ESP32-S3. Es el contrato entre dos imágenes distintas:

- una app de ESP-IDF, que se ejecuta en el **core 0** y hace de cargador;
- un payload Xinu, que se ejecuta en el **core 1** y no vuelve nunca al
  cargador.

El objetivo es que Xinu siga siendo un kernel propio, con su propio scheduler,
su propia pila TCP/IP y su propio heap, pero sin tener que inicializar otra vez
el reloj, la flash, la PSRAM ni el MMU. De eso se encarga ESP-IDF.

> **Importante:** este árbol es AMP-only. `compile/xinu.img` es un binario
> crudo, no una app de ESP-IDF. No se flashea con `idf.py flash` ni se arranca
> desde la ROM como una imagen bare-metal.

---

## 1. Dos imágenes, dos núcleos

```text
Flash
├── 0x000000  bootloader de ESP-IDF
├── 0x008000  tabla de particiones
├── 0x010000  app del loader (factory)
├── 0x100000  payload Xinu (xinu.img)
└── 0x300000  assets (reservado)

Core 0                              Core 1
┌──────────────────────────┐        ┌──────────────────────────┐
│ ESP-IDF / FreeRTOS       │        │ Xinu                     │
│ - relojes y PSRAM        │        │ - scheduler, red, FS     │
│ - MMU y cache            │        │ - JVM, shell, HTTP       │
│ - monta SD (opcional)    │        │ - heap único en PSRAM    │
│ - mapea xinu.img         │        │                          │
│ - salta a _start ────────┼───────►│ _start → nulluser()      │
└──────────────────────────┘        └──────────────────────────┘
```

El core 0 no toca la PSRAM ni la UART0 después del salto. Se queda en un
`vTaskDelay()` permanente para que el sistema no se reinicie.

---

## 2. Tabla de particiones

La tabla real está en `/home/daniel/esp32-xinu-loader/partitions.csv`:

| Offset | Tamaño | Nombre | Tipo | Uso |
|---:|---:|---|---|---|
| `0x000000` | — | bootloader | IDF | Bootloader de segunda etapa |
| `0x008000` | 4 KiB | partition-table | IDF | Tabla de particiones |
| `0x009000` | 16 KiB | `nvs` | data nvs | NVS de IDF |
| `0x00D000` | 4 KiB | `phy_init` | data phy | Calibración PHY |
| `0x010000` | 960 KiB | `factory` | app | App del loader (FreeRTOS) |
| `0x100000` | 2 MiB | `xinu` | data `0x40` | Payload Xinu |
| `0x300000` | 1 MiB | `assets` | data `0x41` | Reservado para la JVM |

El offset `0x100000` no es casual: el MMU de IDF mapea una partición en
`0x42000000 + offset`. Xinu está enlazado en `0x42100000`, así que el offset
debe ser exactamente `0x100000`. Si cambias la partición, cambia también el
`ORIGIN` del linker y la constante `XINU_IMAGE_XIP_BASE` del loader.

La partición `assets` todavía no se usa. Está pensada para clases Java,
recursos de la JVM o imágenes, y evita mezclar datos con el código ejecutable.

---

## 3. Ficheros del loader

| Fichero | Qué hace |
|---|---|
| `main/main.c` | `app_main()`; solo llama a `xinu_loader_boot()` |
| `main/xinu_loader.c` | Busca la partición, reserva PSRAM, mapea XIP, rellena `bootinfo`, crea la tarea del core 1 y hace el handoff |
| `main/xinu_jump.S` | Trampolín en ensamblador: desactiva interrupciones y salta al entry de Xinu con el `bootinfo` en `a2` |
| `main/xinu_sd_update.c` | Actualización opcional: si existe `/xinu.img` en la SD, lo escribe en la partición `xinu` antes de arrancar |
| `main/bootinfo.h` | Contrato de datos loader ↔ Xinu (copia de `include/bootinfo.h`) |
| `main/xinu_image.h` | Formato empaquetado opcional `XIMG` (no es el camino activo) |
| `main/CMakeLists.txt` | Componente IDF con los cuatro fuentes y las dependencias |
| `partitions.csv` | Layout de flash |
| `sdkconfig.defaults` | PSRAM octal, consola UART0, watchdog, etc. |
| `build.sh` | Envoltorio de `idf.py` con el entorno correcto |
| `tools/sync-contracts.sh` | Copia `bootinfo.h` y `xinu_image.h` desde Xinu al loader |

### 3.1 `main.c`

Es deliberadamente mínimo:

```c
void app_main(void)
{
    xinu_loader_boot();
}
```

Toda la lógica está en `xinu_loader.c` para que `app_main()` no dependa de
detalles del handoff.

### 3.2 `xinu_loader.c`

Responsabilidades, en orden:

1. Comprobar que la PSRAM tiene al menos 8 MiB.
2. Buscar la partición `xinu` por tipo `DATA`, subtipo `0x40`, nombre `xinu`.
3. Intentar la actualización desde SD (`/xinu.img`), antes de reservar PSRAM.
4. Reservar el mayor bloque contiguo de PSRAM con `heap_caps_malloc()`.
5. Crear un alias fijo en `0x3D801000` para esa PSRAM.
6. Mapear la partición `xinu` como instrucciones en `0x42100000`.
7. Rellenar `struct xinu_bootinfo` v7.
8. Crear una tarea en el core 1, esperar a que esté lista y dar la señal de
   salida.
9. Saltar a `XINU_IMAGE_XIP_BASE + 0x440` mediante `xinu_jump_to_payload()`.

El entry `+0x440` es el offset actual de `_start` dentro de `xinu.img`.
Puedes comprobarlo con:

```sh
cd /home/daniel/xinu-s3-amp/compile
xtensa-esp32s3-elf-nm -n xinu.elf | grep ' _start$'
```

Si cambia el linker o el prólogo de `start.S`, cambia también el offset del
loader. No es un valor mágico: es el símbolo `_start` menos `0x42100000`.

### 3.3 `xinu_jump.S`

El loader está compilado con el ABI **windowed** de ESP-IDF. Xinu usa el ABI
**CALL0**. No se puede llamar a `_start` como una función C normal.

La secuencia es:

```asm
xinu_jump_to_payload:
    movi a5, 0
    wsr  a5, intenable       /* ninguna interrupción */
    movi a5, -1
    wsr  a5, intclear
    rsync
    mov  a2, a10             /* bootinfo: a10 (windowed) -> a2 (CALL0) */
    jx   a11                 /* entry en a11 */
```

No hay `ENTRY` ni `RET` porque no se vuelve. Xinu instala su propio stack y su
propio `VECBASE` en `start.S`.

### 3.4 `xinu_sd_update.c`

Antes de reservar la PSRAM, el loader monta la SD con SDMMC 4 bits y busca
`/xinu.img`. Si existe:

1. Comprueba que no esté vacío y que quepa en la partición.
2. Borra solo los sectores que va a escribir.
3. Copia el fichero en bloques de 4 KiB a la partición `xinu`.
4. Cierra el fichero y lo borra de la SD.
5. Si algo falla, deja el fichero para reintentar en el siguiente arranque.

No hay slots A/B, ni CRC, ni rollback. Es una actualización simple. Para
producción conviene añadir un slot de respaldo y una verificación de imagen.

---

## 4. Formato de `xinu.img`

### 4.1 Camino activo: binario crudo

En esta revisión, `compile/Makefile` genera `xinu.img` con:

```sh
xtensa-esp32s3-elf-objcopy -O binary xinu.elf xinu.img
```

Es un **binario crudo** que empieza en la primera sección enlazada
(`.vector`, en `0x42100000`). El loader no parsea cabecera: mapea la partición
completa en `0x42100000` y salta a `0x42100440`.

Ventajas:

- simple;
- no hay que empaquetar ni parsear;
- el loader no necesita conocer el número de segmentos.

Limitaciones:

- el entry está fijo en el loader;
- no hay checksum;
- no se puede cambiar el layout sin recompilar el loader;
- no hay validación de segmentos.

### 4.2 Formato empaquetado opcional `XIMG`

`include/xinu_image.h`, `main/xinu_image.h` y `tools/mkxinuimg.py` definen un
formato con cabecera y tabla de segmentos:

```text
struct xinu_image_header {
    uint32_t magic;        /* 'XIMG' */
    uint32_t version;
    uint32_t header_size;
    uint32_t entry;
    uint32_t nseg;
    uint32_t xip_base;
    uint32_t image_size;
    uint32_t crc;
};
```

Cada segmento lleva `vaddr`, `paddr`, `size`, `offset` y `flags` (`XIP`, `COPY`,
`ZERO`, `EXEC`, `WRITE`, `VECTOR`). El formato reserva los primeros `0x1000`
bytes para la cabecera.

> **No está activo.** El loader actual no lee esta cabecera. Si quieres usarlo,
> hay que cambiar `xinu_loader.c` para parsear `xinu_image_header`, validar
> `magic/version/xip_base` y copiar los segmentos. Mientras tanto, el camino
> real es el binario crudo.

---

## 5. Contrato `bootinfo` v7

`struct xinu_bootinfo` es **solo datos**. No cruza punteros a funciones ni
objetos del ABI de IDF. El loader lo rellena y lo pasa en `a2`.

| Offset | Campo | Uso |
|---:|---|---|
| 0 | `magic` | `0x58494E55` (`"XINU"`) |
| 4 | `version` | `7` |
| 8 | `size` | `sizeof(struct xinu_bootinfo)` |
| 12 | `ccount_hz` | Frecuencia real de CCOUNT, en Hz |
| 16 | `core` | Core donde corre Xinu (1) |
| 20 | `flags` | `PSRAM_MAPPED`, `CONSOLE_OWNED`, `AMP`, `XIP` |
| 24 | `psram_va` | Base virtual de la ventana PSRAM de Xinu |
| 28 | `psram_size` | Bytes utilizables desde `psram_va` |

`start.S` guarda el puntero en `xinu_bootinfo`. `platforminit()` valida:

- `magic`;
- `version`;
- `size`;
- `ccount_hz != 0`;
- `psram_size != 0`.

Si algo no cuadra, imprime `xinu: invalid AMP bootinfo` y se queda en un bucle
infinito. No hay recuperación automática.

### 5.1 Qué NO va en `bootinfo`

- El entry point: está fijo en el loader (`+0x440`) o en la cabecera `XIMG`.
- La tabla de vectores: `start.S` usa `_xt_vector_table`, enlazada en XIP.
- El heap: Xinu lo calcula desde `_end` hasta `psram_va + psram_size`.
- El estado de la UART: el loader la silencia antes del salto.

---

## 6. Mapa de memoria

### 6.1 Ventanas

| Región | Dirección | Uso |
|---|---|---|
| XIP de la partición | `0x42100000` | `.vector`, `.init`, `.text` |
| PSRAM alias de Xinu | `0x3D801000` | `.rodata`, `.data`, `.bss`, `.stacks`, heap |
| PSRAM física de IDF | variable | La reserva el loader y la mapea al alias |
| DRAM interna | variable | Buffers pequeños de IDF |

El linker de Xinu (`compile/platforms/esp32s3/ld-amp.script`) declara:

```ld
MEMORY
{
    xip   (rx) : ORIGIN = 0x42100000, LENGTH = 0x00200000
    psram (rw) : ORIGIN = 0x3D801000, LENGTH = 0x007DF000
}
```

La PSRAM no está en `0x3C000000` porque esa ventana la usa IDF para
`.flash.rodata`. El loader instala un alias privado en `0x3D800000` y Xinu
empieza una página después, en `0x3D801000`, para no pisar los metadatos del
heap de IDF.

### 6.2 Secciones

| Sección | Destino | Contenido |
|---|---|---|
| `.vector` | XIP | Tabla de vectores Xtensa |
| `.init` | XIP | `_start` |
| `.text` | XIP | Código y literales |
| `.slowtext` | XIP | Código marcado como lento |
| `.rodata` | PSRAM | Constantes (se copia desde XIP) |
| `.data` | PSRAM | Variables inicializadas (se copia desde XIP) |
| `.bss` | PSRAM | Variables a cero |
| `.stacks` | PSRAM | 8 KiB pánico + 8 KiB hilo nulo |
| heap | PSRAM | Desde `_end` hasta el final de la ventana |

`start.S` copia `.rodata` y `.data` leyendo desde la ventana de datos de flash
(`XIP - 0x06000000`). Después ceroa `.bss` y `.stacks`, publica `xinu_bootinfo`,
pone `sp = _nullstk_top` y salta a `nulluser`.

---

## 7. Secuencia completa de arranque

```text
1. ROM → bootloader IDF → app del loader (core 0)
2. app_main() → xinu_loader_boot()
3. xinu_sd_update()  (opcional, si hay /xinu.img en la SD)
4. Comprueba PSRAM >= 8 MiB
5. Reserva el mayor bloque PSRAM y lo mapea en 0x3D801000
6. Mapea la partición xinu en 0x42100000 (instrucciones)
7. Rellena bootinfo v7
8. Crea tarea en core 1 y espera ready
9. core1_task: espera go, silencia logs, espera 100 ms
10. xinu_jump_to_payload(bootinfo, entry, stack_top)
11. start.S: enmascara interrupciones, VECBASE, copia .rodata/.data,
    ceroa .bss/.stacks, guarda bootinfo, sp = _nullstk_top
12. nulluser() → platforminit() → sysinit() → enable()
13. create(main) → main() → netup, telnet, httpd, udpserver, shell
14. core 0 queda en vTaskDelay() permanente
```

El `sleep(100000)` del core 1 (en realidad `esp_rom_delay_us(100000)`) da
tiempo a que la UART del core 0 termine de vaciar el último mensaje antes de
que Xinu tome la UART0.

---

## 8. Cómo compilar el loader

El loader vive fuera del repositorio de Xinu, en
`/home/daniel/esp32-xinu-loader`.

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

`IDF_MAINTAINER=1` es un override temporal: el toolchain de `/opt` es
`esp-15.2.0_20251204` y ESP-IDF 5.5.1 espera `esp-14.2.0_20241119`. Para
producción conviene instalar el toolchain correcto y quitar el override.

### 8.1 Sincronizar contratos

Si cambias `include/bootinfo.h` o `include/xinu_image.h` en Xinu:

```sh
cd /home/daniel/esp32-xinu-loader
./tools/sync-contracts.sh
```

Los dos árboles deben tener exactamente el mismo layout. Un cambio de tamaño o
de orden de campos rompe el handoff.

### 8.2 Compilar el payload

```sh
cd /home/daniel/xinu-s3-amp
make kernel S3_CONFIG=platforms/esp32s3/xinu-test.conf
```

o, desde el directorio `compile`:

```sh
cd compile
source ../env.sh
make clean
make -j4 CONFIG=platforms/esp32s3/xinu-test.conf
```

`xinu-test.conf` mapea SD/TFT/ETH/RGB a `null`; es la configuración que arranca
en la placa N16R8 sin periféricos. Para la placa completa usa
`platforms/esp32s3/xinu.conf`.

### 8.3 Flashear

Loader (bootloader + tabla + app IDF):

```sh
cd /home/daniel/esp32-xinu-loader
./build.sh -p /dev/ttyACM0 flash
```

Payload Xinu:

```sh
cd /home/daniel/xinu-s3-amp/compile
source ../env.sh
esptool.py --chip esp32s3 -p /dev/ttyACM0 -b 921600 \
  write_flash 0x100000 xinu.img
```

No uses `idf.py flash` para `xinu.img`: no es una app IDF. La consola del
loader es USB-Serial/JTAG; Xinu usa UART0. Si la placa solo tiene USB nativo,
hace falta un adaptador USB-UART en GPIO43/GPIO44 para ver a Xinu.

---

## 9. Verificación rápida

### 9.1 Comprobar el ELF

```sh
cd /home/daniel/xinu-s3-amp/compile
xtensa-esp32s3-elf-nm -n xinu.elf | grep -E ' (_start|_xt_vector_table|_end)$'
xtensa-esp32s3-elf-objdump -h xinu.elf | sed -n '1,25p'
```

Debe verse:

- `_xt_vector_table` en `0x42100000`;
- `_start` en `0x42100440` (o el offset que use el loader);
- `.rodata`/`.data` en `0x3D801000` o superior;
- `.stacks` de `0x4000` bytes antes de `_end`.

### 9.2 Comprobar la imagen

```sh
ls -l xinu.img
xxd -l 32 xinu.img
```

El binario debe empezar con la tabla de vectores, no con `XIMG`. Si empieza
con `XIMG`, estás usando el formato empaquetado y el loader actual no lo
entiende.

### 9.3 Comprobar el arranque

Por UART0:

```text
xinu: invalid AMP bootinfo   -> contrato mal / bootinfo no pasó
Xinu ...                     -> arrancó
```

Si el loader imprime `partition 'xinu' not found`, revisa `partitions.csv` y
el offset. Si imprime `cannot reserve ... PSRAM`, revisa `sdkconfig` y el
tamaño de PSRAM.

---

## 10. Problemas típicos

| Síntoma | Causa probable | Solución |
|---|---|---|
| `partition 'xinu' not found` | Tabla de particiones incorrecta | Reflashea la tabla y el loader |
| `PSRAM size ... smaller than required` | Placa sin 8 MiB o `sdkconfig` mal | Usa N8R8/N16R8 y `CONFIG_SPIRAM_MODE_OCT` |
| `cannot translate reserved PSRAM block` | El bloque no es PSRAM o IDF lo movió | Revisa `CONFIG_SPIRAM_USE_CAPS_ALLOC` |
| `XIP map ... mapped only` | La partición no cae en la ventana esperada | Offset `0x100000` y linker `0x42100000` |
| `xinu: invalid AMP bootinfo` | `bootinfo.h` desincronizado | `./tools/sync-contracts.sh` |
| Arranca y se queda mudo | UART0 no conectada o loader la silenció | Usa UART0 GPIO43/44 |
| Reinicio en bucle | Watchdog IDF o fallo de PSRAM | Revisa `sdkconfig` y `xt_disable_watchdogs()` |
| `xinu.img` no arranca | Es una imagen empaquetada `XIMG` | Usa el binario crudo o actualiza el loader |

---

## 11. Límites y riesgos

- El mapeo XIP es fijo. Si `mmu_hal_map_region()` no devuelve la ventana
  esperada, el loader aborta.
- No hay servicio core 0 ↔ core 1. El core 0 queda parkeado.
- No hay actualización A/B ni rollback.
- La actualización por SD no verifica CRC.
- La PSRAM se reserva como un bloque único; para WiFi/esp-hosted habrá que
  cambiar el layout.
- El loader no es un repositorio git en esta máquina; sus cambios viven solo
  en el árbol de trabajo.
- `IDF_MAINTAINER=1` oculta un desajuste de toolchain; no es producción.

---

## 12. Relación con los comandos del Makefile

Desde la raíz de `xinu-s3-amp`:

```sh
make kernel          # compila compile/xinu.img
make loader          # compila el loader IDF
make loader-flash    # flashea bootloader + tabla + loader
make flash           # flashea solo el payload en 0x100000
make web             # genera/valida las páginas web
make docs            # genera los manuales HTML
make java            # compila las clases Java de ejemplo
make check           # valida HTML y contratos
```

Los detalles de cada target están en el `Makefile` de la raíz y en
`README_BUILD.md`.
