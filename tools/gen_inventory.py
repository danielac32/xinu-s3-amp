#!/usr/bin/env python3
"""Generate a complete, human-readable inventory of the Xinu AMP tree.

The inventory is intentionally generated from the working tree: it is not a
hand-maintained list.  It excludes build artefacts and VCS metadata, but keeps
every source, header, script, configuration and documentation file.

Usage:
    python3 tools/gen_inventory.py [--output docs/INVENTARIO-AMP.md]
"""
from __future__ import annotations

import argparse
import os
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

EXCLUDE_DIRS = {
    ".git", ".hg", ".svn", "__pycache__", "node_modules", ".pio",
    ".deps", "dist", "build", "builds", ".cache", ".venv",
}
EXCLUDE_SUFFIXES = {
    ".o", ".d", ".a", ".so", ".elf", ".img", ".bin", ".map", ".class",
    ".pyc", ".pyo", ".swp", ".tmp", ".log",
}

# Specific explanations for the files a reader is most likely to open.
KEY_FILES = {
    "README_AMP.md": "Estado y contrato del port AMP",
    "README_BUILD.md": "Guía de compilación y configuración",
    "README_ESP32S3.md": "Hardware, periféricos y resultados del port",
    "env.sh": "Carga el toolchain xtensa-esp32s3-elf en PATH",
    "Makefile": "Wrapper raíz: kernel, loader, web, docs, java, flash",
    "include/bootinfo.h": "Contrato de datos loader ↔ Xinu (v7)",
    "include/xinu_image.h": "Formato empaquetado opcional XIMG",
    "include/thread.h": "Hilos, prioridades y tamaños de pila",
    "include/device.h": "devtab[], struct devsw y tipos de dispositivo",
    "include/kernel.h": "API del kernel y constantes globales",
    "include/shell.h": "Prototipos de comandos y constantes del shell",
    "system/initialize.c": "nulluser(), sysinit() y tablas del kernel",
    "system/main.c": "Arranque de consola, TFT, red, servidores y shell",
    "system/platforms/esp32s3/platforminit.c": "Valida bootinfo y publica PSRAM",
    "system/platforms/esp32s3/intc.c": "Interrupciones Xtensa del payload",
    "system/platforms/esp32s3/espclk.c": "Reloj CCOUNT desde bootinfo",
    "system/arch/xtensa/ctxsw.S": "Cambio de contexto ABI CALL0",
    "loader/platforms/esp32s3/start.S": "Entrada _start del payload AMP",
    "loader/platforms/esp32s3/ld-amp.script": "Layout XIP/PSRAM del payload",
    "loader/platforms/esp32s3/panic.c": "Pánico y volcado de registros",
    "compile/Makefile": "Build real del kernel y targets s3-*",
    "compile/platforms/esp32s3/platformVars": "Flags, componentes y drivers",
    "compile/platforms/esp32s3/xinu.conf": "Configuración completa de la placa",
    "compile/platforms/esp32s3/xinu-test.conf": "Configuración de prueba sin periféricos",
    "compile/platforms/esp32s3/ld-amp.script": "Copia de referencia del linker AMP",
    "shell/shell.c": "Intérprete y commandtab[]",
    "shell/xsh_httpd.c": "Servidor HTTP y ficheros estáticos",
    "shell/httpapi.c": "API JSON de archivos y ejecución remota",
    "shell/webterm.c": "Terminal web, MEMTTY0 y worker webshell",
    "shell/xsh_udpserver.c": "Servidor UDP de comandos y firmware",
    "shell/xsh_netup.c": "Arranque de red, DHCP y espera de link",
    "shell/xsh_ps.c": "ps: hilos, estado, prioridad y CPU%",
    "jvm/simple_jvm.h": "Estado de la VM, StackFrame, heap y límites",
    "jvm/class_parser.c": "Parser de la cabecera .class",
    "jvm/constant_pool_parser.c": "Parser del constant pool",
    "jvm/bytecodes.c": "Intérprete de bytecodes",
    "jvm/heap.c": "Objetos, arrays, String y StringBuilder",
    "jvm/stack.c": "Pila de operandos y frames",
    "jvm/java_lib.c": "Tabla de natives y registro de librerías",
    "jvm/java_std.c": "Natives de java.lang",
    "jvm/jvm_run.c": "Carga, <clinit>, main y liberación del estado",
    "jvm/build-examples.sh": "Compila librerías, tests y apps Java",
    "device/jvm/Makerules": "Fuentes del componente JVM del target",
    "device/jvm/jvm_io_target.c": "Seam de plataforma: FAT, PSRAM y kill",
    "device/jvm/jvmglue.c": "jvmState, jvmTask y jvm_run_path",
    "device/jvm/native_hw.c": "Natives esp32p4.HW y esp32p4.RGB",
    "device/jvm/native_io.c": "Natives esp32p4.IO",
    "device/jvm/native_fat.c": "Natives esp32p4.FAT",
    "device/jvm/native_sys.c": "Natives esp32p4.Sys",
    "device/jvm/native_lvgl.c": "Natives esp32p4.LVGL",
    "device/jvm/native_net.c": "Natives esp32p4.Net",
    "device/memtty/memtty.c": "Dispositivo de captura del terminal web",
    "device/w5500/w5500.c": "Driver Ethernet W5500 MACRAW",
    "device/sd/sd.c": "Driver SDMMC 4 bits",
    "device/tft/tft.c": "Driver ST7735 128x160",
    "device/rgb/rgb.c": "LED RGB discreto",
    "network/net/netInit.c": "Inicialización de la pila de red",
    "network/net/netUp.c": "Activación de interfaz de red",
    "fs/Src/fat_filelib.c": "API de ficheros FAT",
    "fs/Src/fat_mount.c": "Montaje y desmontaje FAT",
    "fs/Src/fat_access.c": "Acceso de bajo nivel a FAT",
    "docs/MANUAL-AMP.md": "Manual del kernel AMP",
    "docs/MANUAL-LOADER.md": "Manual del Xinu loader",
    "docs/MANUAL-NATIVES.md": "Tutorial de librerías Java y natives",
    "docs/MANUAL-KERNEL.md": "Manual de C, concurrencia y caso real",
    "docs/INVENTARIO-AMP.md": "Este inventario generado",
    "web/index.html": "Dashboard, terminal web y gestor de archivos",
    "web/build_web.py": "Genera y valida las páginas HTML",
    "web/md2html.py": "Conversor Markdown → HTML",
    "web/docs.html": "Documentación técnica heredada",
    "web/README.md": "Endpoints, terminal, gestor y estructura web",
    "tools/gen_inventory.py": "Generador de este inventario",
    "tools/mkxinuimg.py": "Empaquetador opcional XIMG (no activo)",
}


def role_for(rel: Path) -> str:
    p = rel.as_posix()
    if p in KEY_FILES:
        return KEY_FILES[p]
    parts = rel.parts
    top = parts[0] if parts else ""
    name = rel.name
    if top == "system":
        if "/platforms/" in p:
            return "Kernel: plataforma ESP32-S3"
        if "/arch/" in p:
            return "Kernel: arquitectura Xtensa"
        return "Kernel: núcleo, syscalls o scheduler"
    if top == "device":
        if len(parts) > 1:
            return "Driver/dispositivo: %s" % parts[1]
        return "Driver/dispositivo"
    if top == "network":
        if len(parts) > 1:
            return "Pila de red: %s" % parts[1]
        return "Pila de red"
    if top == "fs":
        return "Sistema de archivos FAT"
    if top == "shell":
        if name.startswith("xsh_"):
            return "Comando shell: %s" % name[4:-2]
        return "Shell, HTTP o terminal web"
    if top == "jvm":
        if name.startswith("native_"):
            return "Native JVM"
        return "Simple JVM: intérprete o soporte"
    if top == "lib":
        return "libxc: libc mínima"
    if top == "mailbox":
        return "Mailbox del kernel"
    if top == "loader":
        return "Arranque, linker o plataforma del payload"
    if top == "compile":
        return "Build, configuración o linker"
    if top == "tools":
        return "Herramienta de build o empaquetado"
    if top == "web":
        return "Interfaz web o build web"
    if top == "docs":
        return "Documentación"
    if top == "apps":
        return "App o comando auxiliar"
    if top == "sd":
        return "Contenido de ejemplo para la SD"
    if top == "amp":
        return "Reservado para AMP"
    if top == "include":
        return "Cabecera pública del kernel"
    if name in ("Makefile", "Makerules"):
        return "Reglas de build"
    if name.endswith(".md"):
        return "Documentación"
    if name.endswith((".sh", ".py")):
        return "Script de build o herramienta"
    return "Fichero de configuración o soporte"


def iter_files() -> list[Path]:
    out = []
    for base, dirs, files in os.walk(ROOT):
        dirs[:] = sorted(d for d in dirs if d not in EXCLUDE_DIRS)
        for f in sorted(files):
            rel = Path(base, f).relative_to(ROOT)
            if rel.suffix in EXCLUDE_SUFFIXES:
                continue
            if rel.name.startswith(".") and rel.name not in (".gitignore",):
                continue
            out.append(rel)
    return out


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--output", default="docs/INVENTARIO-AMP.md")
    args = ap.parse_args()
    files = iter_files()
    out = Path(args.output)
    if not out.is_absolute():
        out = ROOT / out
    out.parent.mkdir(parents=True, exist_ok=True)
    lines = [
        "# Inventario del árbol Xinu AMP",
        "",
        "Generado por `tools/gen_inventory.py`. No editar a mano: se regenera con",
        "`make inventory`.",
        "",
        "El inventario excluye `.git`, artefactos de build (`*.o`, `*.d`, `*.elf`,",
        "`*.img`, `*.class`) y directorios temporales. Incluye todo el código fuente,",
        "cabeceras, scripts, configuración y documentación.",
        "",
        "| Fichero | Tipo | Rol |",
        "|---|---|---|",
    ]
    for rel in files:
        kind = rel.suffix[1:] if rel.suffix else "sin extensión"
        lines.append("| `%s` | `%s` | %s |" % (rel.as_posix(), kind, role_for(rel)))
    lines += [
        "",
        "---",
        "",
        "Total: **%d ficheros**." % len(files),
        "",
        "Los módulos y los ficheros clave se explican en",
        "[MANUAL-AMP.md](MANUAL-AMP.md), [MANUAL-LOADER.md](MANUAL-LOADER.md) y",
        "[MANUAL-NATIVES.md](MANUAL-NATIVES.md).",
        "",
    ]
    text = "\n".join(lines)
    out.write_text(text, encoding="utf-8")
    print("wrote %s (%d files, %d bytes)" % (out, len(files), len(text.encode("utf-8"))))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
