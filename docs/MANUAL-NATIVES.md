# Manual de librerías Java y natives — Xinu AMP

Este manual explica cómo añadir una **librería Java** a la Simple JVM de Xinu,
cómo implementar sus **natives** en C y cómo compilar y desplegar una app.

La JVM no es una JVM de Oracle: es un intérprete de bytecodes con una tabla de
funciones C. No hay JNI, no hay class loader, no hay GC y no hay reflexión.

---

## 1. Arquitectura de una librería

```text
Java                       C
┌────────────────────┐     ┌─────────────────────────────┐
│ esp32p4/MiLib.java │     │ device/jvm/native_milib.c   │
│ public static      │     │ int mi_suma(StackFrame *,   │
│ native int suma(...)│     │   SimpleConstantPool *,     │
│                    │     │   char *type)               │
└─────────┬──────────┘     └──────────────┬──────────────┘
          │                               │
          │ compila                       │ registra
          ▼                               ▼
   MiLib.class                    jvm/java_lib.c
   /java/esp32p4/MiLib.class      {"esp32p4/MiLib", "suma", mi_suma}
```

El bytecode `invokestatic` busca la clase y el método en la tabla
`method_table[]`. Si los encuentra, llama a la función C. Si no, intenta
resolverlo como método Java normal.

---

## 2. Convención de pila

La JVM empuja los argumentos de izquierda a derecha. El **último argumento
está en la cima** de la pila.

Para:

```java
public static native int suma(int a, int b);
```

la pila contiene, de abajo arriba:

```text
[ a ][ b ]
```

El native debe hacer:

```c
int b = popInt(stack);   /* último */
int a = popInt(stack);   /* primero */
pushInt(stack, a + b);
```

Para un método de instancia, el `this` es el primer argumento y queda en el
fondo. Para `String`, usa `jvm_pop_text()` en lugar de `popInt()`.

### 2.1 Funciones de pila

| Función | Uso |
|---|---|
| `popInt(stack)` | Saca un `int` |
| `popLong(stack)` | Saca un `long long` |
| `popFloat(stack)` | Saca un `float` |
| `popDouble(stack)` | Saca un `double` |
| `popEntry(stack)` | Saca una entrada sin convertir |
| `pushInt(stack, v)` | Empuja un `int` |
| `pushLong`, `pushFloat`, `pushDouble` | Empujan el tipo correspondiente |
| `pushRef(stack, v)` | Empuja una referencia del pool |
| `pushObj(stack, v)` | Empuja una referencia del heap |
| `jvm_pop_text(stack, p, buf, max)` | Saca y resuelve un texto (REF/OBJ/INT/FLOAT) |
| `heap_array_load(ref, idx)` | Lee un array |
| `heap_array_store(ref, idx, v)` | Escribe un array |
| `heap_new_string(s)` | Crea un String en el heap |
| `heap_string_text(ref)` | Texto de un String |

---

## 3. Ejemplo completo: `esp32p4.MiLib`

### 3.1 Java

Crea `jvm/lib/esp32p4/MiLib.java`:

```java
package esp32p4;

/**
 * Librería de ejemplo: suma, saludo y lectura de un GPIO.
 */
public class MiLib {
    public static native int suma(int a, int b);
    public static native String saludo(String nombre);
    public static native int leerGpio(int pin);
}
```

Compila:

```sh
cd /home/daniel/xinu-s3-amp/jvm
javac --release 8 -d dist/java lib/esp32p4/MiLib.java
```

El `.class` queda en `jvm/dist/java/esp32p4/MiLib.class`.

### 3.2 C

Crea `device/jvm/native_milib.c`:

```c
/*
 * native_milib.c — natives de esp32p4.MiLib.
 *
 * Convención: el último argumento está en la cima de la pila.
 */
#include <kernel.h>
#include <stdio.h>
#include <string.h>

#include "simple_jvm.h"
#include "java_lib.h"

int esp32p4_milib_suma(StackFrame *stack, SimpleConstantPool *p, char *type)
{
    int b = popInt(stack);
    int a = popInt(stack);

    (void) p;
    (void) type;
    pushInt(stack, a + b);
    return 0;
}

int esp32p4_milib_saludo(StackFrame *stack, SimpleConstantPool *p, char *type)
{
    char nombre[128];
    char salida[160];

    (void) type;
    jvm_pop_text(stack, p, nombre, (int) sizeof(nombre));
    snprintf(salida, sizeof(salida), "Hola, %s", nombre);
    pushObj(stack, heap_new_string(salida));
    return 0;
}

int esp32p4_milib_leerGpio(StackFrame *stack, SimpleConstantPool *p, char *type)
{
    int pin = popInt(stack);

    (void) p;
    (void) type;
    pushInt(stack, (int) xt_gpio_get_level((uint) pin));
    return 0;
}
```

Incluye `<xtensa.h>` si usas `xt_gpio_get_level`.

### 3.3 Registrar en `java_lib.c`

Añade los `extern`:

```c
extern int esp32p4_milib_suma(StackFrame *, SimpleConstantPool *, char *);
extern int esp32p4_milib_saludo(StackFrame *, SimpleConstantPool *, char *);
extern int esp32p4_milib_leerGpio(StackFrame *, SimpleConstantPool *, char *);
```

Y las entradas en `method_table[]`:

```c
{"esp32p4/MiLib", "suma",      esp32p4_milib_suma},
{"esp32p4/MiLib", "saludo",    esp32p4_milib_saludo},
{"esp32p4/MiLib", "leerGpio",  esp32p4_milib_leerGpio},
```

La clave es `paquete/Clase` con `/`, no con `.`.

### 3.4 Añadir al build

Edita `device/jvm/Makerules`:

```make
C_FILES += jvm_io_target.c native_hw.c native_io.c native_fat.c native_sys.c \
           native_lvgl.c native_net.c native_milib.c jvmglue.c
```

Recompila el kernel:

```sh
cd /home/daniel/xinu-s3-amp
make kernel S3_CONFIG=platforms/esp32s3/xinu.conf
```

### 3.5 App Java

Crea `apps/java/MiApp/MiApp.java`:

```java
import esp32p4.MiLib;

public class MiApp {
    public static void main(String[] args) {
        int r = MiLib.suma(2, 3);
        System.out.println("2 + 3 = " + r);
        System.out.println(MiLib.saludo("Xinu"));
        System.out.println("GPIO0 = " + MiLib.leerGpio(0));
    }
}
```

Crea `apps/java/MiApp/Makefile`:

```make
APP      = MiApp
JAVAC   ?= javac
BUILD   ?= build
RELEASE  = 8
LIB_DIR  = ../../../jvm/lib
LIB_SRC  = $(LIB_DIR)/esp32p4/MiLib.java

all: $(BUILD)/$(APP).class

$(BUILD)/$(APP).class: $(APP).java $(LIB_SRC)
	@mkdir -p $(BUILD)
	@$(JAVAC) --release $(RELEASE) -Xlint:-options -d $(BUILD) $(LIB_SRC)
	@$(JAVAC) --release $(RELEASE) -Xlint:-options -cp $(BUILD) -d $(BUILD) $(APP).java

clean:
	@$(RM) -r $(BUILD)
```

Compila y copia a la SD:

```sh
cd /home/daniel/xinu-s3-amp
./jvm/build-examples.sh --to /ruta/a/la/sd
```

La SD debe quedar así:

```text
/java/MiApp.class
/java/esp32p4/MiLib.class
/java/esp32p4/HW.class
...
```

Ejecuta:

```text
xsh$ java MiApp
```

---

## 4. Cómo añadir una librería a `java_lib`

Hay dos caminos:

### 4.1 Native nueva

1. Crea `jvm/lib/esp32p4/MiLib.java`.
2. Crea `device/jvm/native_milib.c`.
3. Declara el `extern` en `jvm/java_lib.c`.
4. Añade la entrada a `method_table[]`.
5. Añade el `.c` a `device/jvm/Makerules`.
6. Recompila el kernel.
7. Compila la clase y cópiala a `/java/esp32p4/`.

### 4.2 Solo Java (sin native)

Si la librería no necesita hardware, puedes escribirla solo en Java y
compilarla como cualquier clase. La JVM ejecutará sus métodos normalmente.
No hace falta tocar `java_lib.c`.

---

## 5. Tipos y conversiones

| Java | C | Nota |
|---|---|---|
| `int` | `int` | 32 bits |
| `long` | `long long` | Usa `popLong`/`pushLong` |
| `float` | `float` | Usa `popFloat`/`pushFloat` |
| `double` | `double` | Usa `popDouble`/`pushDouble` |
| `boolean` | `int` | 0/1 |
| `String` | `char *` | `jvm_pop_text` / `heap_new_string` |
| `int[]` | ref de heap | `heap_array_load/store` |
| `byte[]` | ref de heap | `heap_array_load/store` |
| objeto | ref de heap | `heap_object_get/put` |

No hay `Object`, `Class`, `Throwable` ni `synchronized` completo. Si necesitas
un objeto, usa una clase Java normal con campos; la JVM la representa como un
slot con `nfields`.

---

## 6. Memoria y GC

No hay GC. Los objetos que creas con `new`, `String`, arrays o
`StringBuilder` ocupan slots del heap hasta que termina la tarea. La JVM tiene
`JVM_HEAP_SLOTS = 2048`.

Consecuencias para un native:

- no devuelvas punteros a memoria local del native;
- si devuelves un String, usa `heap_new_string()` (copia);
- no liberes a mano un objeto del heap: lo hará `jvm_state_free()` al final;
- evita crear objetos en bucles largos;
- no guardes punteros del heap en variables globales: la instancia es por
  tarea.

---

## 7. Depuración

### 7.1 El native no se llama

Comprueba:

- el nombre de clase en la tabla usa `/` (`esp32p4/MiLib`);
- el nombre del método coincide exactamente (`suma`, no `Suma`);
- el `.class` está en `/java/esp32p4/MiLib.class`;
- el `.c` está en `device/jvm/Makerules`;
- recompilaste el kernel después de registrar el native.

### 7.2 El native devuelve basura

Suele ser orden de `pop`:

```java
MiLib.suma(2, 3)
```

La pila es `[2][3]`; el primer `popInt` devuelve `3`, el segundo `2`. Si haces
`a + b`, da igual; si restas, no.

### 7.3 La JVM muere

Mira la consola:

```text
[jvm] heap: slots agotados (2048)
[jvm] heap: OOM creando ...
[jvm] heap: referencia invalida ...
```

No hay recuperación: `jvm_exit()` mata la tarea. Reduce asignaciones o aumenta
`JVM_HEAP_SLOTS` (y el tamaño del estado en PSRAM).

### 7.4 `java: no se pudo abrir`

La FAT no está montada o la clase no está en `/java`. Prueba:

```text
fat mount
ls /java
java MiApp
```

---

## 8. Natives existentes

| Clase | Métodos | Fichero |
|---|---|---|
| `esp32p4.HW` | `delay`, `millis`, `gpioWrite`, `gpioRead`, `sound` | `device/jvm/native_hw.c` |
| `esp32p4.RGB` | `set`, `off`, `get` | `device/jvm/native_hw.c` |
| `esp32p4.IO` | `print`, `println`, `readChar`, `strLen`, `strAt`, `printBuf` | `device/jvm/native_io.c` |
| `esp32p4.FAT` | `open`, `close`, `read`, `write`, `seek`, `tell`, `eof`, `readLine`, `writeString`, `readAll`, `writeAll`, `appendString`, `exists`, `isDirectory`, `length`, `delete`, `mkdir`, `rmdir`, `list` | `device/jvm/native_fat.c` |
| `esp32p4.Sys` | `run`, `runHere`, `kill`, `sleep`, `yield`, `tid` | `device/jvm/native_sys.c` |
| `esp32p4.LVGL` | `init`, `screen`, `clear`, `label`, `button`, `rect`, `setText`, `setTextFmt`, `setPos`, `setSize`, `setColor`, `tick`, `pollEvent`, `eventWidget`, `del`, `img`, `getWidth`, `getHeight`, `screenWidth`, `screenHeight` | `device/jvm/native_lvgl.c` |
| `esp32p4.Net` | `ip`, `isConnected` | `device/jvm/native_net.c` |
| `java.lang` | `Math`, `Integer`, `String`, `Boolean`, `Float`, `Character` | `jvm/java_std.c` |

`esp32p4.PMIC` no existe en el S3. `Thread.sleep(long)` se mapea a
`esp32p4.Sys.sleep(int)`.

---

## 9. Makefile de app

Plantilla mínima:

```make
APP      = MiApp
JAVAC   ?= javac
BUILD   ?= build
RELEASE  = 8
LIB_DIR  = ../../../jvm/lib
LIB_SRC  = $(LIB_DIR)/esp32p4/MiLib.java

all: $(BUILD)/$(APP).class

$(BUILD)/$(APP).class: $(APP).java $(LIB_SRC)
	@mkdir -p $(BUILD)
	@$(JAVAC) --release $(RELEASE) -Xlint:-options -d $(BUILD) $(LIB_SRC)
	@$(JAVAC) --release $(RELEASE) -Xlint:-options -cp $(BUILD) -d $(BUILD) $(APP).java

clean:
	@$(RM) -r $(BUILD)
```

Para varias librerías, añádelas a `LIB_SRC`.

---

## 10. Checklist

- [ ] `.java` de la librería en `jvm/lib/esp32p4/`
- [ ] `.c` del native en `device/jvm/`
- [ ] `extern` en `jvm/java_lib.c`
- [ ] entrada en `method_table[]`
- [ ] `.c` en `device/jvm/Makerules`
- [ ] `make kernel`
- [ ] `.class` compilada y copiada a `/java/esp32p4/`
- [ ] app compilada y copiada a `/java/`
- [ ] `fat mount`, `ls /java`, `java MiApp`

---

## 11. Referencias

- `jvm/simple_jvm.h`: tipos, `StackFrame`, heap, límites.
- `jvm/java_lib.c`: tabla de natives.
- `jvm/java_std.c`: natives de `java.lang`.
- `device/jvm/native_*.c`: ejemplos reales.
- `device/jvm/Makerules`: lista de fuentes del target.
- `jvm/build-examples.sh`: compilación de librerías, tests y apps.
- `docs/MANUAL-AMP.md`: arquitectura del kernel y de la JVM.
