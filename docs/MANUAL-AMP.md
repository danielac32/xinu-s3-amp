# Manual del kernel Xinu AMP — ESP32-S3

Este manual describe **este árbol** (`/home/daniel/xinu-s3-amp`), no el Xinu
genérico ni el port bare-metal de `s3-xinu`. Todos los ficheros, funciones y
comandos que aparecen aquí existen en el repositorio.

El kernel se ejecuta como **payload AMP** en el core 1 del ESP32-S3. El core 0
queda para el loader de ESP-IDF. El arranque, las particiones y el contrato
entre las dos imágenes están en [MANUAL-LOADER.md](MANUAL-LOADER.md).

> **Regla de oro:** el kernel no inicializa PSRAM, relojes, MMU ni flash. Todo
> eso ya lo hizo IDF. Xinu solo valida el `bootinfo`, publica la ventana de
> PSRAM, inicializa sus tablas y arranca.

---

## 1. Arquitectura en una página

```text
                         core 0                         core 1
┌──────────────────────────────────────┐   ┌──────────────────────────────────┐
│ ESP-IDF + FreeRTOS                   │   │ Xinu AMP                         │
│  - bootloader, particiones           │   │  - scheduler, hilos, semáforos   │
│  - reloj, PSRAM, MMU, cache          │   │  - devtab, drivers               │
│  - monta SD y actualiza xinu.img     │   │  - red W5500, TCP/UDP/ICMP/DHCP  │
│  - mapea la partición en 0x42100000  │   │  - FAT sobre microSD             │
│  - salta a _start                    │   │  - shell, HTTP, Telnet, UDP      │
└──────────────────────────────────────┘   │  - Simple JVM sin GC             │
                                           └──────────────────────────────────┘
```

### 1.1 Qué es AMP

AMP significa **Asymmetric Multi-Processing**: dos núcleos ejecutan dos
sistemas distintos, no dos instancias del mismo kernel. No hay memoria
compartida gestionada por ambos, no hay un scheduler común y no hay llamadas
cruzadas.

- Core 0: IDF/FreeRTOS, dueño del arranque.
- Core 1: Xinu, dueño del payload.
- Comunicación actual: ninguna. El core 0 se queda en `vTaskDelay()`.

### 1.2 Qué NO es

- No es un kernel bare-metal que arranca desde la ROM.
- No es una app de IDF que llama a Xinu como librería.
- No es SMP: Xinu no usa el core 0.
- No es un sistema de dos imágenes con actualización A/B.

---

## 2. Mapa del árbol

| Directorio | Rol |
|---|---|
| `amp/` | Reservado; hoy está vacío |
| `apps/` | Comandos sueltos del shell (`date`, `rdate`) |
| `compile/` | Makefile, configuración, linker y artefactos de build |
| `device/` | Drivers y dispositivos de Xinu |
| `docs/` | Manuales Markdown y documentación Sphinx heredada |
| `fs/` | Sistema de archivos FAT |
| `include/` | Cabeceras públicas del kernel |
| `jvm/` | Núcleo portable de la Simple JVM |
| `lib/` | `libxc`: libc mínima de Xinu |
| `loader/` | `start.S` y reglas de arranque del payload |
| `mailbox/` | Implementación de mailboxes |
| `network/` | Pila de red: ARP, IPv4, ICMP, DHCP, UDP |
| `sd/` | Contenido de ejemplo para la microSD |
| `shell/` | Shell, comandos, HTTP, API de archivos y terminal web |
| `system/` | Núcleo: hilos, semáforos, monitores, heap, reloj, syscalls |
| `tools/` | Herramientas de empaquetado y utilidades |
| `web/` | Dashboard, manuales HTML y build web |

### 2.1 Ficheros de arranque

| Fichero | Qué hace |
|---|---|
| `loader/platforms/esp32s3/start.S` | Entrada `_start`: enmascara interrupciones, pone VECBASE, copia `.rodata`/`.data`, ceroa `.bss`/`.stacks`, salta a `nulluser` |
| `system/initialize.c` | `nulluser()`: `platforminit()`, `sysinit()`, `enable()`, crea `MAIN` |
| `system/platforms/esp32s3/platforminit.c` | Valida `bootinfo`, publica PSRAM, habilita relojes, apaga watchdogs |
| `system/main.c` | `main()`: consola, TFT, ETH0, `netup`, Telnet, HTTP, UDP y shell serie |
| `system/conf.c` | Generado desde `xinu.conf`; **no se edita** |
| `include/conf.h` | Generado desde `xinu.conf`; **no se edita** |
| `compile/platforms/esp32s3/ld-amp.script` | Layout XIP/PSRAM del payload |

### 2.2 Configuración

| Fichero | Qué hace |
|---|---|
| `compile/platforms/esp32s3/xinu.conf` | Configuración completa: SD, TFT, W5500, RGB, JVM |
| `compile/platforms/esp32s3/xinu-test.conf` | Configuración de prueba: periféricos a `null` |
| `compile/platforms/esp32s3/platformVars` | Flags, componentes y drivers del build |
| `compile/Makefile` | Reglas de compilación, `s3-compile`, `s3-flash` |
| `env.sh` | Añade el toolchain `xtensa-esp32s3-elf-*` al `PATH` |

---

## 3. Arranque, memoria y heap

### 3.1 Secuencia

```text
_start (start.S)
  ├─ enmascara interrupciones y limpia CCOMPARE
  ├─ VECBASE = _xt_vector_table
  ├─ copia .rodata desde XIP a PSRAM
  ├─ copia .data desde XIP a PSRAM
  ├─ ceroa .bss y .stacks
  ├─ xinu_bootinfo = a2
  ├─ sp = _nullstk_top
  └─ nulluser()
       ├─ platforminit()
       ├─ sysinit()
       │   ├─ heapInit(_end, fin de PSRAM)
       │   ├─ thrtab, semtab, montab, bfptab
       │   ├─ mailboxInit()
       │   ├─ devtab[i].init()
       │   └─ netInit()
       ├─ enable()
       └─ create(MAIN)
            └─ main()
                ├─ open(CONSOLE, SERIAL0)
                ├─ open(TFT0), open(ETH0)
                ├─ xsh_netup("ETH0")
                ├─ xsh_telnetserver("-n 1")
                ├─ xsh_httpd("80")
                ├─ xsh_udpserver("5000")
                └─ create(SHELL0)
```

No existe `system/boot.c` ni `bootsvc_tab[]` en este árbol. `main()` llama
directamente a los comandos de arranque. Si añades un servicio, edita
`system/main.c`.

### 3.2 Mapa de memoria

| Región | Dirección | Uso |
|---|---|---|
| XIP | `0x42100000` | `.vector`, `.init`, `.text` |
| PSRAM | `0x3D801000` | `.rodata`, `.data`, `.bss`, `.stacks`, heap |
| Pila de pánico | `_stacks_start.._panicstk_top` | 8 KiB |
| Pila del hilo nulo | `_panicstk_top.._nullstk_top` | 8 KiB |
| Heap | `_end..psram_va+psram_size` | Todo lo demás |

El heap es **uno solo**, en PSRAM. `memget`, `memfree`, `stkget`, `malloc`,
`free`, `calloc` y `realloc` usan la misma lista libre (`memlist`). No hay
heap interno de DRAM ni `psram_malloc` separado.

### 3.3 Reglas de memoria

- No pongas buffers de varios KiB en la pila de 4 KiB.
- Usa `memget`/`memfree` o `malloc`/`free` para buffers grandes.
- Libera con la **misma longitud** que pediste.
- El heap no puede solaparse con `.stacks`: `_end` está después de las pilas.
- `psram` y `free` muestran el estado real; `mem` muestra el heap del kernel.

### 3.4 Hilos y pilas

| Constante | Valor | Uso |
|---|---:|---|
| `INITSTK` | 4096 B | Hilos normales, shell |
| `JVM_STK` | 65536 B | Hilo de la JVM |
| `HTTPD_STK` | 8192 B | Servidor HTTP |
| `WEBTERM_STK` | 16384 B | Worker del terminal web |
| `NET_THR_STK` | 2048 B | Recepción y daemons de red |

`create()` asigna la pila con `stkget()`, que sale del heap de PSRAM. Si no hay
memoria, `create()` devuelve `SYSERR`.

---

## 4. Sistema de build

### 4.1 Compilar el payload

```sh
cd /home/daniel/xinu-s3-amp
make kernel S3_CONFIG=platforms/esp32s3/xinu-test.conf
```

o:

```sh
cd compile
source ../env.sh
make clean
make -j4 CONFIG=platforms/esp32s3/xinu-test.conf
```

El resultado es `compile/xinu.img` (binario crudo) y `compile/xinu.elf`.

### 4.2 Configuración normal vs prueba

| Config | Periféricos | Uso |
|---|---|---|
| `xinu.conf` | SD, TFT, W5500, RGB, JVM | Placa completa |
| `xinu-test.conf` | SD/TFT/ETH/RGB a `null` | Placa N16R8 sin periféricos |

Al cambiar de configuración hay que borrar `include/conf.h` y `system/conf.c`
para forzar la regeneración. El target `s3-compile` ya lo hace.

### 4.3 Añadir un fichero `.c`

1. Crea el fichero en el directorio correcto.
2. Añádelo a `C_FILES` en el `Makerules` de ese directorio.
3. Si es un directorio nuevo, añádelo a `APPCOMPS` o `DEVICES` en
   `compile/platforms/esp32s3/platformVars`.
4. Recompila.

### 4.4 Añadir un driver

1. Crea `device/midriver/` con `miInit.c`, `miOpen.c`, `miRead.c`,
   `miWrite.c`, `miControl.c`, `miClose.c` y `Makerules`.
2. Añade el tipo y la instancia en `xinu.conf`.
3. Añade `midriver` a `DEVICES` en `platformVars`.
4. No edites `system/conf.c`: se genera.

---

## 5. La abstracción del kernel

### 5.1 `devtab[]`: todo es un dispositivo

Xinu no llama a `w5500Read()` desde la red. Llama a `read(ETH0)`. Cada entrada
de `devtab[]` tiene un bloque de funciones:

```c
struct devsw {
    int  (*init)(device *);
    int  (*open)(device *, ...);
    int  (*close)(device *);
    int  (*read)(device *, void *, int);
    int  (*write)(device *, const void *, int);
    int  (*getc)(device *);
    int  (*putc)(device *, char);
    int  (*control)(device *, int, long, long);
};
```

Ventajas:

- el shell, la red y la JVM no conocen el hardware;
- se puede sustituir un driver por `null` en `xinu-test.conf`;
- el mismo código de alto nivel funciona en placa completa y en pruebas.

### 5.2 Syscalls

Las llamadas al kernel (`create`, `wait`, `signal`, `sleep`, `read`, `write`,
`open`, `close`, `control`, `kill`, `receive`, `send`) son funciones C con
`syscall` en la cabecera. En este port no hay cambio de anillo: todo corre en
modo supervisor. `syscall` es una marca de intención, no una instrucción de
trampa.

### 5.3 Concurrencia

| Mecanismo | Uso típico | Dato |
|---|---|---|
| Semáforo | Contar recursos o esperar evento | No |
| Monitor | Exclusión mutua recursiva | No |
| Mailbox | Cola de trabajo de un servicio | 1 `int` |
| `send`/`receive` | Mensaje punto a punto a un `tid` | 1 `int` |

Reglas:

- no duermas con un monitor cogido;
- no llames a `wait`, `sleep` ni `lock` desde una ISR;
- un `lock` por cada `unlock`;
- si el mensaje es un puntero, define quién libera.

### 5.4 Context switch

El ESP32-S3 usa ABI **CALL0**: no hay ventanas de registros. `ctxsw.S` guarda
`a0`, `a2..a15`, `PS`, `PC` y el stack pointer. Por eso el cambio de contexto
es un save/restore plano, no un spill de ventanas.

---

## 6. Archivos por módulo

### 6.1 `system/`

| Fichero | Rol |
|---|---|
| `initialize.c` | `nulluser()`, `sysinit()`, tablas del kernel |
| `main.c` | Arranque de consola, TFT, red, servidores y shell |
| `create.c` | Crear hilos |
| `ready.c`, `resched.c`, `yield.c` | Scheduler |
| `sleep.c`, `wakeup.c`, `unsleep.c` | Temporización |
| `kill.c`, `suspend.c`, `resume.c` | Ciclo de vida |
| `semcreate.c`, `wait.c`, `signal.c`, `semfree.c` | Semáforos |
| `moncreate.c`, `lock.c`, `unlock.c`, `monfree.c` | Monitores |
| `send.c`, `receive.c`, `recvtime.c`, `recvclr.c` | Mensajes |
| `open.c`, `close.c`, `read.c`, `write.c`, `getc.c`, `putc.c`, `control.c` | Syscalls de dispositivos |
| `getdev.c`, `conf.c` | Despacho a `devtab[]` |
| `heap.c`, `memget.c`, `memfree.c`, `stkget.c` | Heap único de PSRAM |
| `bfpalloc.c`, `bfpfree.c`, `bufget.c`, `buffree.c` | Pools de buffers de red |
| `clkinit.c`, `clkhandler.c` | Reloj, ticks, CPU% |
| `softfloat.c` | Coma flotante por software (sin `libgcc`) |
| `platforms/esp32s3/ctxsw.S` | Cambio de contexto CALL0 |
| `platforms/esp32s3/` | `platforminit.c`, `intc.c`, `espclk.c`, `panic.c` |

### 6.2 `device/`

| Directorio | Rol |
|---|---|
| `button/` | Botón BOOT por GPIO0, polling |
| `jvm/` | Natives y glue de la JVM en el target |
| `memtty/` | Dispositivo de captura de salida para el terminal web |
| `nam/` | Namespace manager: monta dispositivos en `/dev` |
| `raw/` | Acceso raw a tramas Ethernet |
| `rgb/` | LED RGB discreto (GPIO2/4/6) |
| `sd/` | Driver SDMMC 4 bits |
| `tcp/` | Implementación TCP |
| `telnet/` | TTY sobre TCP |
| `tft/` | ST7735 128×160 |
| `tty/` | Capa de línea sobre UART/Telnet |
| `udp/` | Implementación UDP |
| `uart-esp32s3/` | UART0 |
| `w5500/` | Ethernet W5500 por SPI2 |

### 6.3 `network/`

| Directorio | Rol |
|---|---|
| `net/` | `struct netif`, `netInit`, `netUp`, `netRecv`, `netSend` |
| `ipv4/` | Recepción, envío, checksum, demux |
| `arp/` | Tabla ARP, daemon, resolución |
| `icmp/` | Echo request/reply, ping |
| `dhcp/` | Cliente DHCP |
| `udp/` | Pila UDP (si no está en `device/udp`) |
| `dns/` | Resolución DNS mínima |

La red usa el W5500 en **MACRAW**: el chip solo mueve tramas; ARP, IPv4, ICMP,
TCP y UDP los implementa Xinu.

### 6.4 `fs/`

| Fichero | Rol |
|---|---|
| `Src/fat_access.c` | Acceso a FAT |
| `Src/fat_filelib.c` | `fopen`, `fread`, `fwrite`, `fclose`, directorios |
| `Src/fat_mount.c` | Montaje/desmontaje |
| `include/fat_port.h` | Adaptación a Xinu |
| `include/fat_filelib.h` | API de ficheros |

La FAT no es reentrante: se protege con `s_fat_mon`. El gestor de archivos y la
JVM comparten el mismo montaje.

### 6.5 `shell/`

| Fichero | Rol |
|---|---|
| `shell.c` | Intérprete, `commandtab[]`, redirecciones, background |
| `xsh_*.c` | Un comando por fichero |
| `xsh_httpd.c` | Servidor HTTP y ficheros estáticos |
| `httpapi.c` | API JSON de archivos y terminal |
| `webterm.c` | Worker del terminal web y `MEMTTY0` |
| `xsh_udpserver.c` | Servidor UDP de comandos |
| `xsh_netup.c` | Arranque de red, DHCP y espera de link |

### 6.6 `jvm/`

| Fichero | Rol |
|---|---|
| `simple_jvm.h` | Estado de la VM, pools, heap, frames |
| `class_parser.c` | Cabecera `.class`, interfaces, campos, métodos |
| `constant_pool_parser.c` | Constant pool |
| `field_pool_parser.c`, `method_pool_parser.c` | Campos y métodos |
| `bytecodes.c` | Intérprete de bytecodes |
| `heap.c` | Objetos, arrays, Strings, StringBuilder |
| `stack.c` | Pila de operandos y frames |
| `java_lib.c` | Tabla de natives |
| `java_std.c` | Natives de `java.lang` |
| `jvm_run.c` | Carga, `<clinit>`, `main`, liberación |
| `free_pool.c` | Libera atributos de métodos |
| `jvm_io.h`, `jvm_map.h` | Seam de plataforma |
| `lib/esp32p4/*.java` | Clases de librería |
| `tests/*.java` | Pruebas de la VM |
| `build-examples.sh` | Compila librerías, tests y apps |

### 6.7 `lib/`

`lib/libxc/` es la libc mínima: `printf`, `malloc`, `free`, `str*`, `mem*`,
`qsort`, `doprnt`, etc. No se enlaza `libgcc` porque el toolchain solo trae una
versión windowed, incompatible con CALL0. La coma flotante está en
`system/softfloat.c`.

### 6.8 `mailbox/`

Implementa las mailboxes: array circular, semáforo de lleno y semáforo de
vacío. Se usan en ARP e ICMP para no bloquear el hilo de recepción.

### 6.9 `loader/`

| Fichero | Rol |
|---|---|
| `loader/platforms/esp32s3/start.S` | Entrada `_start` del payload |
| `loader/platforms/esp32s3/Makerules` | Reglas del arranque |
| `compile/platforms/esp32s3/ld-amp.script` | Layout XIP/PSRAM |
| `system/platforms/esp32s3/panic.c` | Pánico y volcado |
| `system/platforms/esp32s3/intc.c` | Interrupciones Xtensa |
| `system/platforms/esp32s3/espclk.c` | Reloj desde `bootinfo` |

### 6.10 `compile/`

| Fichero | Rol |
|---|---|
| `Makefile` | Build principal |
| `platforms/esp32s3/platformVars` | Flags y componentes |
| `platforms/esp32s3/xinu.conf` | Config normal |
| `platforms/esp32s3/xinu-test.conf` | Config de prueba |
| `platforms/esp32s3/ld-amp.script` | Linker del payload |
| `config/` | Generador de `conf.h` y `conf.c` |

### 6.11 `tools/`

| Fichero | Rol |
|---|---|
| `mkxinuimg.py` | Empaquetador opcional `XIMG` (no activo) |
| `gen_inventory.py` | Genera el inventario completo del árbol |

### 6.12 `web/`

| Fichero | Rol |
|---|---|
| `index.html` | Dashboard, terminal web y gestor de archivos |
| `manual.html` | Manual del kernel en HTML |
| `loader.html` | Manual del loader en HTML |
| `amp.html` | Este manual en HTML |
| `natives.html` | Tutorial de natives |
| `inventory.html` | Inventario de archivos |
| `docs.html` | Documentación heredada |
| `build_web.py` | Genera y valida las páginas |
| `md2html.py` | Conversor Markdown → HTML |

---

## 7. Comandos del shell

El prompt es `xsh$`. La shell está disponible por UART0 y por Telnet. Los
comandos con `--help` imprimen su uso. La tabla siguiente resume **todos** los
comandos registrados en `commandtab[]`.

| Comando | Uso | Qué hace / para qué |
|---|---|---|
| `arp` | `arp` | Muestra la tabla ARP (IP ↔ MAC). Diagnóstico de red |
| `button` | `button [wait [ticks]]` | Estado del botón BOOT; `wait` bloquea hasta pulsación o timeout |
| `cat` | `cat <file>` | Vuelca un fichero de texto a la salida |
| `cd` | `cd <dir>` | Cambia el directorio actual de la FAT |
| `clear` | `clear` | Limpia la pantalla del terminal |
| `config` | `config [command]` | Muestra la configuración del kernel o si un comando es builtin/externo |
| `cp` | `cp <src> <dst>` | Copia un fichero |
| `curl` | `curl <url> [out]` | Alias de `wget` |
| `date` | `date` | Muestra la fecha/hora del RTC (puede estar en 1900) |
| `df` | `df` | Espacio libre/usado de la FAT |
| `dns` | `dns <host>` | Resuelve un nombre DNS |
| `du` | `du [path]` | Tamaño recursivo de ficheros |
| `dumpfile` | `dumpfile <file> [max]` | Volcado hexadecimal + ASCII |
| `echo` | `echo [args...]` | Imprime argumentos; útil para scripts |
| `exit` | `exit` | Cierra la shell |
| `fat` | `fat <mount|ls|cat|write|format> ...` | Operaciones directas sobre FAT |
| `free` | `free [-v]` | Resumen de imagen, pilas, heap y pools; `-v` vuelca la lista libre |
| `gpio` | `gpio read|write|mode <pin> [0|1|in|out]` | Lee/escribe/configura un GPIO |
| `head` | `head [-n N] <file>` | Primeras N líneas |
| `help` | `help [command]` | Lista comandos o ayuda de uno |
| `hexdump` | `hexdump <file> [max]` | Volcado hexadecimal |
| `httpd` | `httpd [port]` | Arranca el servidor HTTP (80 por defecto) |
| `ifconfig` | `ifconfig [-a]` | Muestra IP, máscara, gateway y MAC |
| `ip` | `ip [-a]` | Alias de `ifconfig` |
| `java` | `java <Class> [args...]` | Ejecuta `/java/<Class>.class` en la JVM |
| `kill` | `kill <tid>` | Mata un hilo |
| `ls` | `ls [path]` | Lista un directorio |
| `mem` | `mem [-k] [-q] [-t tid]` | Estado del heap del kernel |
| `memdump` | `memdump [-C] [-f] <start> <len>` | Volcado de memoria |
| `mkdir` | `mkdir <dir>` | Crea un directorio |
| `mon` | `mon` | Estado de monitores (dueño, count) |
| `mv` | `mv <src> <dst>` | Mueve/renombra |
| `nc` | `nc [opciones] <host> <port>` | Conexión TCP manual |
| `netdown` | `netdown <dev>` | Baja una interfaz de red |
| `netstat` | `netstat` | Interfaces y contadores de red |
| `netup` | `netup [dev] [ip] [mask] [gw]` | Levanta la red; DHCP si no das IP |
| `ping` | `ping <ip> [-c N] [-i ms]` | ICMP echo |
| `ps` | `ps` | Hilos, estado, prioridad, CPU% y pila |
| `psram` | `psram [-t]` | Ventana PSRAM, heap, libre; `-t` autotest |
| `pwd` | `pwd` | Directorio actual |
| `rdate` | `rdate <host>` | Hora remota por RFC 868 |
| `reset` | `reset` | Reinicio software |
| `rm` | `rm <file>...` | Borra ficheros |
| `rmdir` | `rmdir <dir>` | Borra un directorio vacío |
| `route` | `route [add|del ...]` | Tabla de rutas |
| `sd` | `sd [read <lba>|test [lba]]` | Estado/pruebas de la tarjeta SD |
| `sem` | `sem` | Estado de semáforos |
| `sleep` | `sleep <seconds>` | Duerme N segundos |
| `stack` | `stack [-r] [tid]` | Pico de pila por hilo; `-r` reinicia |
| `stat` | `stat <file>` | Metadatos de un fichero/directorio |
| `tail` | `tail [-n N] <file>` | Últimas N líneas |
| `tcpstat` | `tcpstat` | Estado de sockets TCP |
| `telnet` | `telnet <host> [port]` | Cliente Telnet |
| `telnetd` | `telnetd ...` | Alias de `telnetserver` |
| `telnetserver` | `telnetserver [-d dev] [-p port] [-n N] [-h]` | Servidor Telnet |
| `udpserver` | `udpserver [port]` | Servidor UDP de comandos (5000) |
| `udpstat` | `udpstat` | Estado de dispositivos UDP |
| `w5500` | `w5500` | Estado del controlador Ethernet |
| `wget` | `wget <url> [outfile]` | Descarga HTTP a fichero o stdout |
| `xmodem` | `xmodem recv <file>` | Recibe por XMODEM |
| `?` | `?` | Alias de `help` |

### 7.1 Comandos presentes pero no registrados

| Fichero | Comando | Estado |
|---|---|---|
| `shell/xsh_tft.c` | `tft` | La entrada está comentada en `commandtab[]`; se puede activar |
| `shell/xsh_snoop.c` | `snoop` | No está en `commandtab[]`; es una herramienta de captura |

### 7.2 Comandos de diagnóstico recomendados

```sh
ps              # hilos, CPU%, pila
stack           # pico de pila
free            # imagen, pilas, heap
psram           # ventana PSRAM y heap
mem             # heap del kernel
sem             # semáforos
mon             # monitores
netstat         # red
tcpstat         # TCP
arp             # ARP
route           # rutas
df              # FAT
```

### 7.3 Comandos de red y ficheros

```sh
netup ETH0
ping 192.168.1.1
wget http://192.168.1.1/index.html /index.html
httpd 80
telnetserver -n 1
udpserver 5000
ls /
cat /index.html
java Loop
```

---

## 8. Comandos UDP

El servidor UDP escucha en el puerto **5000** por defecto. Cada datagrama es
una línea de texto; la respuesta vuelve al mismo host/puerto. El cliente Java
y el dashboard usan este canal.

| Comando | Respuesta / uso |
|---|---|
| `PING` | `PONG <ticks>`; comprobar que el servidor vive |
| `INFO` | Versión, uptime, red, heap y tamaño de PSRAM |
| `VERSION` | Versión del kernel |
| `UPTIME` | `uptime_ms: N` |
| `MEM` | `heap_free`, `heap_used`, `heap_largest`, contadores, PSRAM |
| `PS` | Tabla de hilos con CPU% de una ventana de 1 s |
| `NET` | IP, máscara, gateway, MAC |
| `GPIO READ <pin>` | Lee un GPIO |
| `GPIO WRITE <pin> <0|1>` | Escribe un GPIO |
| `GPIO MODE <pin> IN|OUT` | Configura dirección |
| `SD INFO` | Estado de la tarjeta |
| `TFT TEST` | Patrón de prueba en la TFT |
| `BUTTON` | Estado del botón |
| `RGB SET <rrggbb>` | Color RGB |
| `RGB OFF` | Apaga el LED |
| `RGB ON` | Restaura el último color |
| `RGB GET` | Último color |
| `FW_BEGIN <size>` | Inicia subida de firmware |
| `FW_DATA <seq> <len>\n<bytes>` | Bloque binario de firmware |
| `FW_END` | Cierra, verifica y reinicia |
| `FW_ABORT` | Cancela la subida |
| `FW_STATUS` | Estado de la subida |
| `REBOOT` | Reinicia la placa |
| `HELP` | Lista de comandos |

Formato de firmware:

```text
FW_BEGIN 1048576
FW_DATA 0 960
<960 bytes binarios>
FW_DATA 1 960
<960 bytes binarios>
...
FW_END
```

Bloques de 960 bytes; máximo 2 MiB. `FW_END` verifica y reinicia. No flashees
firmware sin autorización.

---

## 9. HTTP, gestor de archivos y terminal web

### 9.1 Endpoints

| Método y ruta | Función | Qué hace |
|---|---|---|
| `GET /` y ficheros | `serve_static()` | Sirve `index.html` y ficheros de la SD |
| `GET /api/files?path=/dir` | `api_files()` | Lista un directorio en JSON |
| `GET /api/download?path=/file` | `api_download()` | Descarga un fichero |
| `PUT /api/file?path=/file` | `api_upload()` | Sube un fichero |
| `DELETE /api/file?path=/file` | `api_delete()` | Borra un fichero |
| `POST /api/mkdir?path=/dir` | `api_mkdir()` | Crea un directorio |
| `POST /api/rmdir?path=/dir[&recursive=1]` | `api_rmdir()` | Borra un directorio |
| `POST /api/exec` | `api_exec()` | Encola un comando del shell |
| `GET /api/job?id=N` | `api_job()` | Consulta el trabajo |

### 9.2 Límites

| Elemento | Valor |
|---|---:|
| Puerto HTTP | 80 |
| Buffer de petición | 2048 B |
| Pila de `httpd` | 8192 B |
| Timeout de recepción | 2000 ms |
| Comando máximo | 159 B |
| Salida capturada | 8192 B |
| Subida máxima | 16 MB |
| Buffer de subida | 1024 B |

El `httpd` atiende **una conexión a la vez**. El terminal web ejecuta un
comando en el hilo `webshell` y captura la salida en `MEMTTY0`. No hay
autenticación: úsalo solo en una LAN de confianza.

### 9.3 GAT: Gestor de Archivos y Terminal

En este árbol **no existe una librería C llamada GAT**. Lo que sí existe es el
módulo que llamamos **GAT** (Gestor de Archivos y Terminal), formado por:

- `shell/xsh_httpd.c`: servidor HTTP y ficheros estáticos;
- `shell/httpapi.c`: API JSON de archivos y ejecución;
- `shell/webterm.c`: worker, monitor, semáforo y `MEMTTY0`;
- `web/index.html`: interfaz de usuario con gestor de archivos y terminal.

Flujo del terminal:

```text
navegador
  └─ POST /api/exec
       └─ api_exec() → webtermSubmit() → signal(jobsem)
            └─ webshell → webtermRun()
                 ├─ builtin: ejecuta en webshell
                 └─ externo: create(hijo) con stdout/stderr en MEMTTY0
  └─ GET /api/job?id=N
       └─ api_job() → webtermStatus()/webtermSnapshot()
```

Si por "GAT" te referías a otra librería, no está en este repositorio. La capa
de soporte del kernel es `lib/libxc`; la abstracción de dispositivos es
`devtab[]`.

---

## 10. Modificar el kernel

### 10.1 Añadir un comando

1. Crea `shell/xsh_midemo.c`:

```c
#include <shell.h>
#include <stdio.h>

shellcmd xsh_midemo(int nargs, char *args[])
{
    printf("hola desde midemo\n");
    return SHELL_OK;
}
```

2. Añade el prototipo en `include/shell.h`.
3. Añade la entrada en `commandtab[]` (`shell/shell.c`).
4. Añade `xsh_midemo.c` a `C_FILES` en `shell/Makerules`.
5. Recompila.

### 10.2 Añadir un servicio de arranque

Edita `system/main.c` y llama al comando antes de crear `SHELL0`:

```c
char *mi_argv[] = {"micomando", "arg", NULL};
xsh_micomando(2, mi_argv);
```

Si el servicio debe correr en su propio hilo, crea un hilo con `create()` y
`ready()`.

### 10.3 Añadir una llamada al sistema

1. Implementa `system/misyscall.c`.
2. Declara `syscall misyscall(...)` en `include/kernel.h` o en una cabecera
   propia.
3. Añade el `.c` a `system/Makerules`.
4. Si la llamada usa una tabla nueva, añádela a `xinu.conf`.

### 10.4 Cambiar la configuración

Edita `compile/platforms/esp32s3/xinu.conf`:

```c
#define NTHREAD   50
#define NSEM      128
#define NMON      16
#define NMAILBOX  10
#define NPOOL     8
#define NTCP      10
```

Después borra `include/conf.h` y `system/conf.c` y recompila.

### 10.5 Errores típicos

| Error | Causa | Solución |
|---|---|---|
| `undefined reference` | Falta el `.c` en `Makerules` | Añádelo a `C_FILES` |
| `implicit declaration` | Falta el prototipo | Incluye la cabecera correcta |
| `dram overflowed` | Demasiadas globales | Mueve buffers a PSRAM/heap |
| `iram overflowed` | Demasiado código crítico | Saca código de IRAM |
| `conflict` en `conf.h` | Configuración vieja | Borra `conf.h` y `conf.c` |
| `invalid AMP bootinfo` | Contrato loader/Xinu mal | Sincroniza `bootinfo.h` |

---

## 11. Portar el kernel

### 11.1 Qué está abstraído

| Capa | Abstracción | Dónde |
|---|---|---|
| Dispositivos | `devtab[]` y `struct devsw` | `system/`, `include/device.h` |
| Plataforma | `platforminit()`, `platform` | `system/platforms/<plat>/` |
| Contexto | `ctxsw.S` | `system/platforms/<plat>/` |
| Linker | `ld-*.script` | `compile/platforms/<plat>/` |
| Arranque | `start.S` | `loader/platforms/<plat>/` |
| Build | `platformVars`, `Makerules` | `compile/` y cada módulo |
| Red | `struct netif`, `netSend/netRecv` | `network/net/` |
| JVM | `jvm_io.h` | `jvm/` y `device/jvm/` |

### 11.2 Pasos para una placa nueva

1. Copia `compile/platforms/esp32s3` a `compile/platforms/miplaca`.
2. Ajusta `platformVars`: toolchain, flags, `APPCOMPS`, `DEVICES`.
3. Ajusta `ld-amp.script`: direcciones XIP/PSRAM y `.stacks`.
4. Ajusta `start.S`: reloj, VECBASE, copia de secciones, pila.
5. Ajusta `platforminit.c`: PSRAM, UART, relojes, watchdogs.
6. Ajusta `xinu.conf`: dispositivos y números de tabla.
7. Ajusta los drivers de UART, SD, TFT, Ethernet y GPIO.
8. Ajusta el loader IDF: partición, direcciones, `bootinfo`.
9. Compila, flashea y prueba consola, `free`, `ps`, red y FAT.

### 11.3 Qué no se puede cambiar a la ligera

- El ABI CALL0 y el layout de `ctxsw.S`.
- El contrato `bootinfo` sin sincronizar loader y payload.
- La dirección XIP `0x42100000` sin cambiar el loader.
- El alias PSRAM `0x3D801000` sin cambiar el linker y el loader.
- El heap único: si añades otro allocator, rompes `free`/`malloc`.

---

## 12. Simple JVM

### 12.1 Qué es

Es un intérprete de bytecodes Java, no una JVM completa. Carga un `.class`,
parsea constant pool, campos y métodos, ejecuta `<clinit>` y `main(String[])`.
Conserva el paquete `esp32p4.*` para que las clases del P4 corran sin
recompilar.

### 12.2 Estado y memoria

| Elemento | Valor |
|---|---:|
| Pila del hilo | `JVM_STK` = 65536 B |
| Slots de heap | `JVM_HEAP_SLOTS` = 2048 |
| Locals por frame | `JVM_MAX_LOCALS` = 16 |
| Profundidad de llamadas | `JVM_MAX_CALLS` = 32 |
| Estáticos | `JVM_MAX_STATICS` = 64 |
| Pila de operandos | 500 entradas |

Cada ejecución crea una instancia `Jvm` en PSRAM con `jvm_state_new()`. El
estado incluye pools, locals, heap, estáticos y frames. La VM es reentrante:
varias tareas pueden ejecutar Java a la vez, cada una con su instancia.

### 12.3 No hay GC

**No hay recolector de basura.** `heap_new_slot()` busca un slot libre, pero
nada los libera durante la ejecución:

- objetos, arrays, Strings y StringBuilders ocupan slots hasta el final;
- `jvm_state_free()` libera todo al terminar la tarea;
- `free_pools()` libera atributos de métodos al final;
- no hay `System.gc()`, finalizadores, compactación ni referencias débiles.

Consecuencia: un bucle que crea objetos sin parar agota los 2048 slots y llama
a `exit(6)` (en el target, mata la tarea). Para apps largas:

- reutiliza objetos;
- evita `new` dentro de bucles;
- usa arrays de tamaño fijo;
- no confíes en que la memoria se recupere.

### 12.4 Natives

Los natives son funciones C registradas en `jvm/java_lib.c`:

```c
{"esp32p4/RGB", "set", esp32p4_rgb_set}
```

La firma es:

```c
int mi_native(StackFrame *stack, SimpleConstantPool *p, char *type);
```

Los argumentos están en la pila de operandos; el último argumento está en la
cima. Se leen con `popInt(stack)` y se devuelve con `pushInt(stack, valor)`.
El tutorial completo está en [MANUAL-NATIVES.md](MANUAL-NATIVES.md).

### 12.5 Limitaciones

- No hay `Thread` real; `Thread.sleep` se mapea a `Sys.sleep`.
- No hay PMIC en el S3; `esp32p4.PMIC` no existe.
- LVGL está en fase 2.
- `Sys.run()` crea otra tarea JVM; no está probado en todas las apps.
- La JVM necesita FAT/SD para cargar `.class`.
- `date` puede mostrar 1900 porque el RTC no se inicializa.

---

## 13. Flujos de trabajo

### 13.1 Compilar y flashear

```sh
cd /home/daniel/xinu-s3-amp
make kernel S3_CONFIG=platforms/esp32s3/xinu.conf
make flash S3_PORT=/dev/ttyACM0
```

### 13.2 Compilar el loader

```sh
make loader
make loader-flash S3_PORT=/dev/ttyACM0
```

### 13.3 Compilar Java

```sh
make java
# o
./jvm/build-examples.sh --to /ruta/a/la/sd
```

### 13.4 Generar la web

```sh
make web
make docs
```

`make web` valida `web/index.html`, genera `web/loader.html`, `web/amp.html`,
`web/natives.html`, `web/inventory.html` y copia el paquete a `web/dist/`.

### 13.5 Pruebas mínimas

```sh
ps
free
psram
mem
netstat
ping 192.168.1.1
df
ls /
java Loop
```

---

## 14. Problemas típicos

| Síntoma | Causa | Solución |
|---|---|---|
| No arranca | `xinu.img` no está en `0x100000` | Reflashea el payload |
| `invalid AMP bootinfo` | Contrato desincronizado | Sincroniza `bootinfo.h` |
| Se reinicia | Watchdog IDF | Revisa `platforminit` y `sdkconfig` |
| No hay red | Cable, link o DHCP | `netup ETH0`, `w5500`, `netstat` |
| `httpd` no responde | FAT no montada | `fat mount`, `df` |
| `java` no encuentra clase | `.class` fuera de `/java` | Copia a `/java` |
| JVM muere | Slots agotados | Reduce asignaciones |
| `free` negativo | Bug antiguo | Usa `psram`/`mem` |
| `make` no encuentra toolchain | `env.sh` no cargado | `make kernel` ya lo carga |

---

## 15. Inventario completo

El inventario generado desde el árbol está en
[INVENTARIO-AMP.md](INVENTARIO-AMP.md) y en `web/inventory.html`. Se regenera
con:

```sh
make inventory
```

Incluye todos los ficheros fuente, cabeceras, scripts, configuración y
documentación, con su módulo y rol. Los ficheros clave tienen explicación
detallada en las secciones anteriores.

---

## 16. Referencias

- [MANUAL-LOADER.md](MANUAL-LOADER.md): loader, particiones, `bootinfo`, salto.
- [MANUAL-NATIVES.md](MANUAL-NATIVES.md): librerías Java y natives.
- [MANUAL-KERNEL.md](MANUAL-KERNEL.md): C, concurrencia y estudio de caso.
- `README_AMP.md`: estado del port AMP.
- `README_BUILD.md`: build y configuración.
- `README_ESP32S3.md`: hardware y resultados.
- `web/README.md`: dashboard, API y terminal.
