# Compilar y extender Xinu ESP32-S3 AMP

Este documento es la guía corta de build. La explicación completa está en
[docs/MANUAL-AMP.md](docs/MANUAL-AMP.md) y
[docs/MANUAL-LOADER.md](docs/MANUAL-LOADER.md).

> **AMP-only:** este árbol genera `compile/xinu.img` para el loader de
> ESP-IDF. No genera `xinu.boot` ni se flashea en `0x0`.

## 1. Comandos rápidos

Desde `/home/daniel/xinu-s3-amp`:

```sh
make help
make kernel S3_CONFIG=platforms/esp32s3/xinu-test.conf
make loader
make web
make flash S3_PORT=/dev/ttyACM0
make loader-flash S3_PORT=/dev/ttyACM0
make java
```

| Target | Qué hace |
|---|---|
| `kernel` / `compile` / `payload` | Compila `compile/xinu.img` |
| `flash` | Escribe `xinu.img` en `0x100000` |
| `loader` | Compila la app IDF de `../esp32-xinu-loader` |
| `loader-flash` | Flashea bootloader + tabla + loader |
| `loader-sync` | Sincroniza `bootinfo.h` y `xinu_image.h` |
| `web` / `index` | Genera manuales, valida `index.html` y crea `web/dist` |
| `docs` | Genera solo los manuales HTML |
| `inventory` | Regenera `docs/INVENTARIO-AMP.md` |
| `check` | Valida enlaces locales sin generar |
| `java` | Compila librerías, tests y apps Java |
| `clean` | Limpia objetos, `xinu.img` y `web/dist` |

Variables: `S3_CONFIG`, `S3_PORT`, `S3_BAUD`, `S3_FLASH_OFFSET`,
`LOADER_DIR`, `JAVA_TO`, `PYTHON`.

## 2. Configuración

| Fichero | Uso |
|---|---|
| `compile/platforms/esp32s3/xinu.conf` | Placa completa: SD, TFT, W5500, RGB, JVM |
| `compile/platforms/esp32s3/xinu-test.conf` | Prueba: SD/TFT/ETH/RGB a `null` |

`include/conf.h` y `system/conf.c` son **generados**. No se editan a mano.
`make kernel` los borra antes de compilar.

## 3. Añadir un `.c`

1. Crea el fichero en el módulo correcto.
2. Añádelo a `C_FILES` en el `Makerules` de ese módulo.
3. Si es un módulo nuevo, añádelo a `APPCOMPS` o `DEVICES` en
   `compile/platforms/esp32s3/platformVars`.
4. Recompila con `make kernel`.

## 4. Añadir un comando

1. Crea `shell/xsh_foo.c` con `shellcmd xsh_foo(int nargs, char *args[])`.
2. Añade el prototipo en `include/shell.h`.
3. Registra `{"foo", FALSE, xsh_foo}` en `commandtab[]` de `shell/shell.c`.
4. Añade `xsh_foo.c` a `shell/Makerules`.
5. Recompila.

## 5. Añadir un driver

1. Crea `device/midriver/` con `miInit.c`, `miOpen.c`, `miRead.c`,
   `miWrite.c`, `miControl.c`, `miClose.c` y `Makerules`.
2. Añade el tipo y la instancia en `xinu.conf`.
3. Añade `midriver` a `DEVICES` en `platformVars`.
4. Recompila.

## 6. Linker y plataforma

El linker del payload es `compile/platforms/esp32s3/ld-amp.script`:

```ld
MEMORY
{
    xip   (rx) : ORIGIN = 0x42100000, LENGTH = 0x00200000
    psram (rw) : ORIGIN = 0x3D801000, LENGTH = 0x007DF000
}
```

No cambies `0x42100000` sin cambiar el loader y la partición. No cambies
`0x3D801000` sin cambiar el alias PSRAM del loader. El mapa completo está en
`docs/MANUAL-LOADER.md`.

## 7. Java

```sh
make java
# o
./jvm/build-examples.sh --to /ruta/a/la/sd
```

Las clases quedan en `jvm/dist/java/` y, con `--to`, en `DIR/java/`. El comando
`java` las busca en `/java`. El tutorial de librerías y natives está en
[docs/MANUAL-NATIVES.md](docs/MANUAL-NATIVES.md).

## 8. Web

```sh
make web
```

Genera `web/manual.html`, `web/loader.html`, `web/amp.html`,
`web/natives.html`, `web/inventory.html` y el paquete `web/dist/`. El
dashboard es `web/index.html`.

## 9. Subir ficheros a la SD

Desde la shell de Xinu:

```text
wget http://<host>:8001/App.class /java/App.class
```

O por XMODEM:

```text
xmodem recv /java/App.class
```

También se puede usar el gestor de archivos web (`PUT /api/file`).
