# Convenience wrapper for the ESP32-S3 Xinu AMP project.
#
# The real kernel build lives in compile/Makefile.  The loader is an ESP-IDF
# app in ../esp32-xinu-loader.  The web manuals are generated from docs/*.md.
#
#   make kernel          build compile/xinu.img
#   make loader          build the ESP-IDF loader
#   make web             generate manuals and package web/dist
#   make flash           write xinu.img at 0x100000
#   make loader-flash    write bootloader + partition table + loader
#   make help            list every target
#
# Variables are forwarded to the sub-makes, for example:
#
#   make kernel S3_CONFIG=platforms/esp32s3/xinu.conf
#   make flash S3_PORT=/dev/ttyUSB0 S3_BAUD=921600
#   make loader LOADER_DIR=/home/daniel/esp32-xinu-loader

.DEFAULT_GOAL := help

SHELL           := /bin/sh
PYTHON          ?= python3
S3_CONFIG       ?= platforms/esp32s3/xinu.conf
S3_PORT         ?= /dev/ttyUSB0
S3_BAUD         ?= 921600
S3_FLASH_OFFSET ?= 0x100000
LOADER_DIR      ?= ../esp32-xinu-loader
JAVA_TO         ?=

.PHONY: help all kernel compile payload s3-compile flash s3-flash \
        loader loader-build loader-flash loader-sync flash-all \
        web index docs inventory check java clean clean-web distclean status

help:
	@echo "Xinu ESP32-S3 AMP"
	@echo ""
	@echo "Kernel:"
	@echo "  make kernel          Compile compile/xinu.img (default: xinu-test.conf)"
	@echo "  make compile         Alias de kernel"
	@echo "  make payload         Alias de kernel"
	@echo "  make flash           Flash compile/xinu.img en 0x100000"
	@echo ""
	@echo "Loader (ESP-IDF, core 0):"
	@echo "  make loader          Compile $(LOADER_DIR)"
	@echo "  make loader-flash    Flash bootloader + tabla + loader"
	@echo "  make loader-sync     Sincroniza bootinfo.h/xinu_image.h"
	@echo "  make flash-all       loader-flash + flash"
	@echo ""
	@echo "Web y documentación:"
	@echo "  make web             Genera manuales, valida index y crea web/dist"
	@echo "  make index           Alias de web"
	@echo "  make docs            Genera solo los manuales HTML"
	@echo "  make inventory       Regenera docs/INVENTARIO-AMP.md"
	@echo "  make check           Valida enlaces y páginas sin generar"
	@echo ""
	@echo "Java:"
	@echo "  make java            Compila librerías, tests y apps de jvm/"
	@echo "                       JAVA_TO=/ruta/sd copia las clases a la SD"
	@echo ""
	@echo "Limpieza:"
	@echo "  make clean           Limpia objetos, xinu.img y web/dist"
	@echo "  make clean-web       Limpia solo web/dist y páginas generadas"
	@echo "  make distclean       clean + limpia configuración generada"
	@echo ""
	@echo "Overrides:"
	@echo "  S3_CONFIG=$(S3_CONFIG)"
	@echo "  S3_PORT=$(S3_PORT)"
	@echo "  S3_BAUD=$(S3_BAUD)"
	@echo "  S3_FLASH_OFFSET=$(S3_FLASH_OFFSET)"
	@echo "  LOADER_DIR=$(LOADER_DIR)"
	@echo "  PYTHON=$(PYTHON)"
	@echo ""
	@echo "Config normal: make kernel S3_CONFIG=platforms/esp32s3/xinu.conf"

all: kernel web

# ---------------------------------------------------------------------------
# Kernel
# ---------------------------------------------------------------------------
kernel compile payload s3-compile:
	$(MAKE) -C compile s3-compile \
		S3_CONFIG=$(S3_CONFIG) \
		S3_PORT=$(S3_PORT) \
		S3_BAUD=$(S3_BAUD) \
		S3_FLASH_OFFSET=$(S3_FLASH_OFFSET)

flash s3-flash:
	$(MAKE) -C compile s3-flash \
		S3_CONFIG=$(S3_CONFIG) \
		S3_PORT=$(S3_PORT) \
		S3_BAUD=$(S3_BAUD) \
		S3_FLASH_OFFSET=$(S3_FLASH_OFFSET)

# ---------------------------------------------------------------------------
# Loader (separate ESP-IDF app)
# ---------------------------------------------------------------------------
loader loader-build:
	@test -x "$(LOADER_DIR)/build.sh" || { \
		echo "No existe $(LOADER_DIR)/build.sh" >&2; \
		echo "Ajusta LOADER_DIR=/ruta/esp32-xinu-loader" >&2; \
		exit 1; \
	}
	cd "$(LOADER_DIR)" && ./build.sh build

loader-flash:
	@test -x "$(LOADER_DIR)/build.sh" || { \
		echo "No existe $(LOADER_DIR)/build.sh" >&2; \
		echo "Ajusta LOADER_DIR=/ruta/esp32-xinu-loader" >&2; \
		exit 1; \
	}
	cd "$(LOADER_DIR)" && ./build.sh -p "$(S3_PORT)" flash

loader-sync:
	@test -x "$(LOADER_DIR)/tools/sync-contracts.sh" || { \
		echo "No existe $(LOADER_DIR)/tools/sync-contracts.sh" >&2; \
		exit 1; \
	}
	cd "$(LOADER_DIR)" && ./tools/sync-contracts.sh

flash-all: loader-flash flash
	@echo "Loader y payload flasheados."

# ---------------------------------------------------------------------------
# Web and documentation
# ---------------------------------------------------------------------------
web index:
	$(PYTHON) web/build_web.py

docs:
	$(PYTHON) web/build_web.py --docs --no-dist

inventory:
	$(PYTHON) tools/gen_inventory.py
	$(PYTHON) web/build_web.py --docs --no-dist --skip-inventory

check:
	$(PYTHON) web/build_web.py --check

# ---------------------------------------------------------------------------
# Java examples
# ---------------------------------------------------------------------------
java:
	@test -x jvm/build-examples.sh || { \
		echo "No existe jvm/build-examples.sh" >&2; \
		exit 1; \
	}
	@if [ -n "$(JAVA_TO)" ]; then \
		./jvm/build-examples.sh --to "$(JAVA_TO)"; \
	else \
		./jvm/build-examples.sh; \
	fi

# ---------------------------------------------------------------------------
# Clean
# ---------------------------------------------------------------------------
clean:
	$(MAKE) -C compile clean
	rm -rf web/dist

clean-web:
	rm -rf web/dist
	rm -f web/manual.html web/loader.html web/amp.html \
	      web/natives.html web/inventory.html

distclean: clean
	rm -f include/conf.h system/conf.c
	rm -rf compile/.deps

status:
	@echo "S3_CONFIG=$(S3_CONFIG)"
	@echo "S3_PORT=$(S3_PORT)"
	@echo "S3_BAUD=$(S3_BAUD)"
	@echo "S3_FLASH_OFFSET=$(S3_FLASH_OFFSET)"
	@echo "LOADER_DIR=$(LOADER_DIR)"
	@echo "PYTHON=$(PYTHON)"
	@test -f compile/xinu.img && ls -l compile/xinu.img || echo "compile/xinu.img: no existe"
