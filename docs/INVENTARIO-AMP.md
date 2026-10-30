# Inventario del árbol Xinu AMP

Generado por `tools/gen_inventory.py`. No editar a mano: se regenera con
`make inventory`.

El inventario excluye `.git`, artefactos de build (`*.o`, `*.d`, `*.elf`,
`*.img`, `*.class`) y directorios temporales. Incluye todo el código fuente,
cabeceras, scripts, configuración y documentación.

| Fichero | Tipo | Rol |
|---|---|---|
| `.gitignore` | `sin extensión` | Fichero de configuración o soporte |
| `AUTHORS` | `sin extensión` | Fichero de configuración o soporte |
| `LICENSE` | `sin extensión` | Fichero de configuración o soporte |
| `Makefile` | `sin extensión` | Wrapper raíz: kernel, loader, web, docs, java, flash |
| `PENDIENTES.md` | `md` | Documentación |
| `README.md` | `md` | Documentación |
| `README_AMP.md` | `md` | Estado y contrato del port AMP |
| `README_BUILD.md` | `md` | Guía de compilación y configuración |
| `README_ESP32S3.md` | `md` | Hardware, periféricos y resultados del port |
| `env.sh` | `sh` | Carga el toolchain xtensa-esp32s3-elf en PATH |
| `apps/Makerules` | `sin extensión` | App o comando auxiliar |
| `apps/date.c` | `c` | App o comando auxiliar |
| `apps/rdate.c` | `c` | App o comando auxiliar |
| `apps/java/ColorShow/ColorShow.java` | `java` | App o comando auxiliar |
| `apps/java/ColorShow/Makefile` | `sin extensión` | App o comando auxiliar |
| `apps/java/DelayDemo/DelayDemo.java` | `java` | App o comando auxiliar |
| `apps/java/DelayDemo/Makefile` | `sin extensión` | App o comando auxiliar |
| `apps/java/ImgDemo/ImgDemo.java` | `java` | App o comando auxiliar |
| `apps/java/ImgDemo/Makefile` | `sin extensión` | App o comando auxiliar |
| `apps/java/LedBlink/LedBlink.java` | `java` | App o comando auxiliar |
| `apps/java/LedBlink/Makefile` | `sin extensión` | App o comando auxiliar |
| `apps/java/LvglDemo/LvglDemo.java` | `java` | App o comando auxiliar |
| `apps/java/LvglDemo/Makefile` | `sin extensión` | App o comando auxiliar |
| `apps/java/Menu/Makefile` | `sin extensión` | App o comando auxiliar |
| `apps/java/Menu/Menu.java` | `java` | App o comando auxiliar |
| `apps/java/RgbDemo/Makefile` | `sin extensión` | App o comando auxiliar |
| `apps/java/RgbDemo/RgbDemo.java` | `java` | App o comando auxiliar |
| `apps/java/WordQuest/Makefile` | `sin extensión` | App o comando auxiliar |
| `apps/java/WordQuest/WordQuest.java` | `java` | App o comando auxiliar |
| `apps/java/WordQuest/save.dat` | `dat` | App o comando auxiliar |
| `compile/Doxyfile` | `sin extensión` | Build, configuración o linker |
| `compile/Doxymain.c` | `c` | Build, configuración o linker |
| `compile/Makefile` | `sin extensión` | Build real del kernel y targets s3-* |
| `compile/mkvers.sh` | `sh` | Build, configuración o linker |
| `compile/version` | `sin extensión` | Build, configuración o linker |
| `compile/vn` | `sin extensión` | Build, configuración o linker |
| `compile/arch/README` | `sin extensión` | Build, configuración o linker |
| `compile/arch/xtensa/platformVars` | `sin extensión` | Build, configuración o linker |
| `compile/config/Makefile` | `sin extensión` | Build, configuración o linker |
| `compile/config/config` | `sin extensión` | Build, configuración o linker |
| `compile/config/config.c` | `c` | Build, configuración o linker |
| `compile/config/parse.c` | `c` | Build, configuración o linker |
| `compile/config/parse.y` | `y` | Build, configuración o linker |
| `compile/config/scan.l` | `l` | Build, configuración o linker |
| `compile/config/y.tab.h` | `h` | Build, configuración o linker |
| `compile/data/Makerules` | `sin extensión` | Build, configuración o linker |
| `compile/platforms/esp32s3/ld-amp.script` | `script` | Copia de referencia del linker AMP |
| `compile/platforms/esp32s3/platformVars` | `sin extensión` | Flags, componentes y drivers |
| `compile/platforms/esp32s3/xinu-test.conf` | `conf` | Configuración de prueba sin periféricos |
| `compile/platforms/esp32s3/xinu.conf` | `conf` | Configuración completa de la placa |
| `compile/scripts/make-arm-cross-compiler` | `sin extensión` | Build, configuración o linker |
| `device/Doxygroup.c` | `c` | Driver/dispositivo: Doxygroup.c |
| `device/button/Makerules` | `sin extensión` | Driver/dispositivo: button |
| `device/button/buttonClose.c` | `c` | Driver/dispositivo: button |
| `device/button/buttonControl.c` | `c` | Driver/dispositivo: button |
| `device/button/buttonGetc.c` | `c` | Driver/dispositivo: button |
| `device/button/buttonInit.c` | `c` | Driver/dispositivo: button |
| `device/button/buttonInterrupt.c` | `c` | Driver/dispositivo: button |
| `device/button/buttonOpen.c` | `c` | Driver/dispositivo: button |
| `device/button/buttonPressed.c` | `c` | Driver/dispositivo: button |
| `device/button/buttonPutc.c` | `c` | Driver/dispositivo: button |
| `device/button/buttonRead.c` | `c` | Driver/dispositivo: button |
| `device/button/buttonWrite.c` | `c` | Driver/dispositivo: button |
| `device/jvm/Makerules` | `sin extensión` | Fuentes del componente JVM del target |
| `device/jvm/jvm_io_target.c` | `c` | Seam de plataforma: FAT, PSRAM y kill |
| `device/jvm/jvmglue.c` | `c` | jvmState, jvmTask y jvm_run_path |
| `device/jvm/native_fat.c` | `c` | Natives esp32p4.FAT |
| `device/jvm/native_hw.c` | `c` | Natives esp32p4.HW y esp32p4.RGB |
| `device/jvm/native_io.c` | `c` | Natives esp32p4.IO |
| `device/jvm/native_lvgl.c` | `c` | Natives esp32p4.LVGL |
| `device/jvm/native_net.c` | `c` | Natives esp32p4.Net |
| `device/jvm/native_sys.c` | `c` | Natives esp32p4.Sys |
| `device/memtty/Makerules` | `sin extensión` | Driver/dispositivo: memtty |
| `device/memtty/memtty.c` | `c` | Dispositivo de captura del terminal web |
| `device/nam/Makerules` | `sin extensión` | Driver/dispositivo: nam |
| `device/nam/mount.c` | `c` | Driver/dispositivo: nam |
| `device/nam/naminit.c` | `c` | Driver/dispositivo: nam |
| `device/nam/nammap.c` | `c` | Driver/dispositivo: nam |
| `device/nam/namopen.c` | `c` | Driver/dispositivo: nam |
| `device/raw/Doxygroup.c` | `c` | Driver/dispositivo: raw |
| `device/raw/Makerules` | `sin extensión` | Driver/dispositivo: raw |
| `device/raw/rawClose.c` | `c` | Driver/dispositivo: raw |
| `device/raw/rawControl.c` | `c` | Driver/dispositivo: raw |
| `device/raw/rawDemux.c` | `c` | Driver/dispositivo: raw |
| `device/raw/rawInit.c` | `c` | Driver/dispositivo: raw |
| `device/raw/rawOpen.c` | `c` | Driver/dispositivo: raw |
| `device/raw/rawRead.c` | `c` | Driver/dispositivo: raw |
| `device/raw/rawRecv.c` | `c` | Driver/dispositivo: raw |
| `device/raw/rawSend.c` | `c` | Driver/dispositivo: raw |
| `device/raw/rawWrite.c` | `c` | Driver/dispositivo: raw |
| `device/rgb/Makerules` | `sin extensión` | Driver/dispositivo: rgb |
| `device/rgb/rgbClose.c` | `c` | Driver/dispositivo: rgb |
| `device/rgb/rgbControl.c` | `c` | Driver/dispositivo: rgb |
| `device/rgb/rgbInit.c` | `c` | Driver/dispositivo: rgb |
| `device/rgb/rgbOpen.c` | `c` | Driver/dispositivo: rgb |
| `device/rgb/rgbRead.c` | `c` | Driver/dispositivo: rgb |
| `device/rgb/rgbSet.c` | `c` | Driver/dispositivo: rgb |
| `device/rgb/rgbWrite.c` | `c` | Driver/dispositivo: rgb |
| `device/sd/Makerules` | `sin extensión` | Driver/dispositivo: sd |
| `device/sd/sdCard.c` | `c` | Driver/dispositivo: sd |
| `device/sd/sdClose.c` | `c` | Driver/dispositivo: sd |
| `device/sd/sdControl.c` | `c` | Driver/dispositivo: sd |
| `device/sd/sdGetc.c` | `c` | Driver/dispositivo: sd |
| `device/sd/sdHost.c` | `c` | Driver/dispositivo: sd |
| `device/sd/sdInit.c` | `c` | Driver/dispositivo: sd |
| `device/sd/sdOpen.c` | `c` | Driver/dispositivo: sd |
| `device/sd/sdPutc.c` | `c` | Driver/dispositivo: sd |
| `device/sd/sdRead.c` | `c` | Driver/dispositivo: sd |
| `device/sd/sdWrite.c` | `c` | Driver/dispositivo: sd |
| `device/sd/sdhost.h` | `h` | Driver/dispositivo: sd |
| `device/tcp/Doxygroup.c` | `c` | Driver/dispositivo: tcp |
| `device/tcp/Makerules` | `sin extensión` | Driver/dispositivo: tcp |
| `device/tcp/tcpAlloc.c` | `c` | Driver/dispositivo: tcp |
| `device/tcp/tcpAllocPassive.c` | `c` | Driver/dispositivo: tcp |
| `device/tcp/tcpChksum.c` | `c` | Driver/dispositivo: tcp |
| `device/tcp/tcpClose.c` | `c` | Driver/dispositivo: tcp |
| `device/tcp/tcpControl.c` | `c` | Driver/dispositivo: tcp |
| `device/tcp/tcpDemux.c` | `c` | Driver/dispositivo: tcp |
| `device/tcp/tcpFree.c` | `c` | Driver/dispositivo: tcp |
| `device/tcp/tcpGetc.c` | `c` | Driver/dispositivo: tcp |
| `device/tcp/tcpInit.c` | `c` | Driver/dispositivo: tcp |
| `device/tcp/tcpOpen.c` | `c` | Driver/dispositivo: tcp |
| `device/tcp/tcpOpenActive.c` | `c` | Driver/dispositivo: tcp |
| `device/tcp/tcpPutc.c` | `c` | Driver/dispositivo: tcp |
| `device/tcp/tcpRead.c` | `c` | Driver/dispositivo: tcp |
| `device/tcp/tcpRecv.c` | `c` | Driver/dispositivo: tcp |
| `device/tcp/tcpRecvAck.c` | `c` | Driver/dispositivo: tcp |
| `device/tcp/tcpRecvData.c` | `c` | Driver/dispositivo: tcp |
| `device/tcp/tcpRecvListen.c` | `c` | Driver/dispositivo: tcp |
| `device/tcp/tcpRecvOpts.c` | `c` | Driver/dispositivo: tcp |
| `device/tcp/tcpRecvOther.c` | `c` | Driver/dispositivo: tcp |
| `device/tcp/tcpRecvRtt.c` | `c` | Driver/dispositivo: tcp |
| `device/tcp/tcpRecvSynsent.c` | `c` | Driver/dispositivo: tcp |
| `device/tcp/tcpRecvValid.c` | `c` | Driver/dispositivo: tcp |
| `device/tcp/tcpSend.c` | `c` | Driver/dispositivo: tcp |
| `device/tcp/tcpSendAck.c` | `c` | Driver/dispositivo: tcp |
| `device/tcp/tcpSendData.c` | `c` | Driver/dispositivo: tcp |
| `device/tcp/tcpSendPersist.c` | `c` | Driver/dispositivo: tcp |
| `device/tcp/tcpSendRst.c` | `c` | Driver/dispositivo: tcp |
| `device/tcp/tcpSendRxt.c` | `c` | Driver/dispositivo: tcp |
| `device/tcp/tcpSendSyn.c` | `c` | Driver/dispositivo: tcp |
| `device/tcp/tcpSendWindow.c` | `c` | Driver/dispositivo: tcp |
| `device/tcp/tcpSeqdiff.c` | `c` | Driver/dispositivo: tcp |
| `device/tcp/tcpSetup.c` | `c` | Driver/dispositivo: tcp |
| `device/tcp/tcpStat.c` | `c` | Driver/dispositivo: tcp |
| `device/tcp/tcpTimer.c` | `c` | Driver/dispositivo: tcp |
| `device/tcp/tcpTimerPurge.c` | `c` | Driver/dispositivo: tcp |
| `device/tcp/tcpTimerRemain.c` | `c` | Driver/dispositivo: tcp |
| `device/tcp/tcpTimerSched.c` | `c` | Driver/dispositivo: tcp |
| `device/tcp/tcpTimerTrigger.c` | `c` | Driver/dispositivo: tcp |
| `device/tcp/tcpWrite.c` | `c` | Driver/dispositivo: tcp |
| `device/telnet/Doxygroup.c` | `c` | Driver/dispositivo: telnet |
| `device/telnet/Makerules` | `sin extensión` | Driver/dispositivo: telnet |
| `device/telnet/telnetAlloc.c` | `c` | Driver/dispositivo: telnet |
| `device/telnet/telnetClose.c` | `c` | Driver/dispositivo: telnet |
| `device/telnet/telnetControl.c` | `c` | Driver/dispositivo: telnet |
| `device/telnet/telnetFlush.c` | `c` | Driver/dispositivo: telnet |
| `device/telnet/telnetGetc.c` | `c` | Driver/dispositivo: telnet |
| `device/telnet/telnetInit.c` | `c` | Driver/dispositivo: telnet |
| `device/telnet/telnetOpen.c` | `c` | Driver/dispositivo: telnet |
| `device/telnet/telnetPutc.c` | `c` | Driver/dispositivo: telnet |
| `device/telnet/telnetRead.c` | `c` | Driver/dispositivo: telnet |
| `device/telnet/telnetServer.c` | `c` | Driver/dispositivo: telnet |
| `device/telnet/telnetWrite.c` | `c` | Driver/dispositivo: telnet |
| `device/tft/Makerules` | `sin extensión` | Driver/dispositivo: tft |
| `device/tft/tftClose.c` | `c` | Driver/dispositivo: tft |
| `device/tft/tftControl.c` | `c` | Driver/dispositivo: tft |
| `device/tft/tftGetc.c` | `c` | Driver/dispositivo: tft |
| `device/tft/tftHw.c` | `c` | Driver/dispositivo: tft |
| `device/tft/tftInit.c` | `c` | Driver/dispositivo: tft |
| `device/tft/tftOpen.c` | `c` | Driver/dispositivo: tft |
| `device/tft/tftPutc.c` | `c` | Driver/dispositivo: tft |
| `device/tft/tftRead.c` | `c` | Driver/dispositivo: tft |
| `device/tft/tftWrite.c` | `c` | Driver/dispositivo: tft |
| `device/tty/Doxygroup.c` | `c` | Driver/dispositivo: tty |
| `device/tty/Makerules` | `sin extensión` | Driver/dispositivo: tty |
| `device/tty/ttyClose.c` | `c` | Driver/dispositivo: tty |
| `device/tty/ttyControl.c` | `c` | Driver/dispositivo: tty |
| `device/tty/ttyGetc.c` | `c` | Driver/dispositivo: tty |
| `device/tty/ttyInit.c` | `c` | Driver/dispositivo: tty |
| `device/tty/ttyOpen.c` | `c` | Driver/dispositivo: tty |
| `device/tty/ttyPutc.c` | `c` | Driver/dispositivo: tty |
| `device/tty/ttyRead.c` | `c` | Driver/dispositivo: tty |
| `device/tty/ttyWrite.c` | `c` | Driver/dispositivo: tty |
| `device/uart/Doxygroup.c` | `c` | Driver/dispositivo: uart |
| `device/uart/README` | `sin extensión` | Driver/dispositivo: uart |
| `device/uart/kprintf.c` | `c` | Driver/dispositivo: uart |
| `device/uart/kvprintf.c` | `c` | Driver/dispositivo: uart |
| `device/uart/uartControl.c` | `c` | Driver/dispositivo: uart |
| `device/uart/uartGetc.c` | `c` | Driver/dispositivo: uart |
| `device/uart/uartInit.c` | `c` | Driver/dispositivo: uart |
| `device/uart/uartPutc.c` | `c` | Driver/dispositivo: uart |
| `device/uart/uartRead.c` | `c` | Driver/dispositivo: uart |
| `device/uart/uartStat.c` | `c` | Driver/dispositivo: uart |
| `device/uart/uartWrite.c` | `c` | Driver/dispositivo: uart |
| `device/uart-esp32s3/Makerules` | `sin extensión` | Driver/dispositivo: uart-esp32s3 |
| `device/uart-esp32s3/esp32uart.h` | `h` | Driver/dispositivo: uart-esp32s3 |
| `device/uart-esp32s3/kgetc.c` | `c` | Driver/dispositivo: uart-esp32s3 |
| `device/uart-esp32s3/kputc.c` | `c` | Driver/dispositivo: uart-esp32s3 |
| `device/uart-esp32s3/uartHwInit.c` | `c` | Driver/dispositivo: uart-esp32s3 |
| `device/uart-esp32s3/uartHwPutc.c` | `c` | Driver/dispositivo: uart-esp32s3 |
| `device/uart-esp32s3/uartHwStat.c` | `c` | Driver/dispositivo: uart-esp32s3 |
| `device/uart-esp32s3/uartInterrupt.c` | `c` | Driver/dispositivo: uart-esp32s3 |
| `device/udp/Doxygroup.c` | `c` | Driver/dispositivo: udp |
| `device/udp/Makerules` | `sin extensión` | Driver/dispositivo: udp |
| `device/udp/udpAlloc.c` | `c` | Driver/dispositivo: udp |
| `device/udp/udpChksum.c` | `c` | Driver/dispositivo: udp |
| `device/udp/udpClose.c` | `c` | Driver/dispositivo: udp |
| `device/udp/udpControl.c` | `c` | Driver/dispositivo: udp |
| `device/udp/udpDemux.c` | `c` | Driver/dispositivo: udp |
| `device/udp/udpFreebuf.c` | `c` | Driver/dispositivo: udp |
| `device/udp/udpGetbuf.c` | `c` | Driver/dispositivo: udp |
| `device/udp/udpInit.c` | `c` | Driver/dispositivo: udp |
| `device/udp/udpOpen.c` | `c` | Driver/dispositivo: udp |
| `device/udp/udpRead.c` | `c` | Driver/dispositivo: udp |
| `device/udp/udpRecv.c` | `c` | Driver/dispositivo: udp |
| `device/udp/udpSend.c` | `c` | Driver/dispositivo: udp |
| `device/udp/udpWrite.c` | `c` | Driver/dispositivo: udp |
| `device/w5500/Makerules` | `sin extensión` | Driver/dispositivo: w5500 |
| `device/w5500/w5500Close.c` | `c` | Driver/dispositivo: w5500 |
| `device/w5500/w5500Command.c` | `c` | Driver/dispositivo: w5500 |
| `device/w5500/w5500Control.c` | `c` | Driver/dispositivo: w5500 |
| `device/w5500/w5500Getc.c` | `c` | Driver/dispositivo: w5500 |
| `device/w5500/w5500Init.c` | `c` | Driver/dispositivo: w5500 |
| `device/w5500/w5500Interrupt.c` | `c` | Driver/dispositivo: w5500 |
| `device/w5500/w5500Open.c` | `c` | Driver/dispositivo: w5500 |
| `device/w5500/w5500Putc.c` | `c` | Driver/dispositivo: w5500 |
| `device/w5500/w5500Read.c` | `c` | Driver/dispositivo: w5500 |
| `device/w5500/w5500Spi.c` | `c` | Driver/dispositivo: w5500 |
| `device/w5500/w5500Write.c` | `c` | Driver/dispositivo: w5500 |
| `docs/Getting-Started.rst` | `rst` | Documentación |
| `docs/INVENTARIO-AMP.md` | `md` | Este inventario generado |
| `docs/Introduction.rst` | `rst` | Documentación |
| `docs/MANUAL-AMP.md` | `md` | Manual del kernel AMP |
| `docs/MANUAL-KERNEL.md` | `md` | Manual de C, concurrencia y caso real |
| `docs/MANUAL-LOADER.md` | `md` | Manual del Xinu loader |
| `docs/MANUAL-NATIVES.md` | `md` | Tutorial de librerías Java y natives |
| `docs/Makefile` | `sin extensión` | Documentación |
| `docs/README` | `sin extensión` | Documentación |
| `docs/conf.py` | `py` | Documentación |
| `docs/index.rst` | `rst` | Documentación |
| `docs/make.bat` | `bat` | Documentación |
| `docs/xinusource.py` | `py` | Documentación |
| `docs/arm/ARM-Interrupt-Handling.rst` | `rst` | Documentación |
| `docs/arm/ARM-Preemptive-Multitasking.rst` | `rst` | Documentación |
| `docs/arm/ARM-qemu.rst` | `rst` | Documentación |
| `docs/arm/index.rst` | `rst` | Documentación |
| `docs/arm/rpi/BCM2835-Interrupt-Controller.rst` | `rst` | Documentación |
| `docs/arm/rpi/BCM2835-Memory-Barriers.rst` | `rst` | Documentación |
| `docs/arm/rpi/BCM2835-System-Timer.rst` | `rst` | Documentación |
| `docs/arm/rpi/BCM2835.rst` | `rst` | Documentación |
| `docs/arm/rpi/Raspberry-Pi-Interrupt-Handling.rst` | `rst` | Documentación |
| `docs/arm/rpi/Raspberry-Pi.rst` | `rst` | Documentación |
| `docs/arm/rpi/RaspberryPi.jpg` | `jpg` | Documentación |
| `docs/arm/rpi/SMSC-LAN9512.rst` | `rst` | Documentación |
| `docs/arm/rpi/Synopsys-USB-Controller.rst` | `rst` | Documentación |
| `docs/arm/rpi/XinuPi.rst` | `rst` | Documentación |
| `docs/arm/rpi/index.rst` | `rst` | Documentación |
| `docs/development/Build-System.rst` | `rst` | Documentación |
| `docs/development/Documentation.rst` | `rst` | Documentación |
| `docs/development/Git-Repository.rst` | `rst` | Documentación |
| `docs/development/Kernel-Normal-Form.rst` | `rst` | Documentación |
| `docs/development/Porting.rst` | `rst` | Documentación |
| `docs/development/Systems-Laboratory.rst` | `rst` | Documentación |
| `docs/development/Trace.rst` | `rst` | Documentación |
| `docs/development/XINU-Summer2011.png` | `png` | Documentación |
| `docs/development/index.rst` | `rst` | Documentación |
| `docs/features/Mailboxes.rst` | `rst` | Documentación |
| `docs/features/Memory-Management.rst` | `rst` | Documentación |
| `docs/features/Message-Passing.rst` | `rst` | Documentación |
| `docs/features/Preemptive-Multitasking.rst` | `rst` | Documentación |
| `docs/features/Shell.rst` | `rst` | Documentación |
| `docs/features/Standard-Library.rst` | `rst` | Documentación |
| `docs/features/TTY-Driver.rst` | `rst` | Documentación |
| `docs/features/TtyDriver.png` | `png` | Documentación |
| `docs/features/USB-Keyboard-Driver.rst` | `rst` | Documentación |
| `docs/features/USB.rst` | `rst` | Documentación |
| `docs/features/index.rst` | `rst` | Documentación |
| `docs/features/networking/ARP.rst` | `rst` | Documentación |
| `docs/features/networking/DHCP.rst` | `rst` | Documentación |
| `docs/features/networking/ICMP.rst` | `rst` | Documentación |
| `docs/features/networking/Networking-Stack.rst` | `rst` | Documentación |
| `docs/features/networking/Routing.rst` | `rst` | Documentación |
| `docs/features/networking/TCP.rst` | `rst` | Documentación |
| `docs/features/networking/TFTP.rst` | `rst` | Documentación |
| `docs/features/networking/UDP.rst` | `rst` | Documentación |
| `docs/features/networking/XINUNetStack-Print.jpeg` | `jpeg` | Documentación |
| `docs/features/networking/XINUNetStack-Screen.jpeg` | `jpeg` | Documentación |
| `docs/features/networking/XINUNetStack.pdf` | `pdf` | Documentación |
| `docs/features/networking/index.rst` | `rst` | Documentación |
| `docs/mips/Common-Firmware-Environment.rst` | `rst` | Documentación |
| `docs/mips/EJTAG-ID-Codes-and-Implementation-Registers.rst` | `rst` | Documentación |
| `docs/mips/EJTAG.rst` | `rst` | Documentación |
| `docs/mips/Exception-and-Interrupt-Handling.rst` | `rst` | Documentación |
| `docs/mips/Flash-Driver.rst` | `rst` | Documentación |
| `docs/mips/Flash-Memory.rst` | `rst` | Documentación |
| `docs/mips/General-Purpose-Input-Output.rst` | `rst` | Documentación |
| `docs/mips/HOWTO-Backup-Router.rst` | `rst` | Documentación |
| `docs/mips/HOWTO-Connect-to-Modified-Router.rst` | `rst` | Documentación |
| `docs/mips/HOWTO-Install-OpenWRT.rst` | `rst` | Documentación |
| `docs/mips/HOWTO-Modify-the-ASUS-Hardware.rst` | `rst` | Documentación |
| `docs/mips/HOWTO-Modify-the-Linksys-Hardware.rst` | `rst` | Documentación |
| `docs/mips/HOWTO-Recover-Router.rst` | `rst` | Documentación |
| `docs/mips/Memory.rst` | `rst` | Documentación |
| `docs/mips/Mips-console.rst` | `rst` | Documentación |
| `docs/mips/Mipsel-qemu.rst` | `rst` | Documentación |
| `docs/mips/Processor.rst` | `rst` | Documentación |
| `docs/mips/Serial-Adapter-Diagrams.rst` | `rst` | Documentación |
| `docs/mips/Startup.rst` | `rst` | Documentación |
| `docs/mips/Switch-Driver.rst` | `rst` | Documentación |
| `docs/mips/TRX-Header.rst` | `rst` | Documentación |
| `docs/mips/UART-Driver.rst` | `rst` | Documentación |
| `docs/mips/WRT54GL.rst` | `rst` | Documentación |
| `docs/mips/index.rst` | `rst` | Documentación |
| `docs/mips/images/Attach_Back.jpg` | `jpg` | Documentación |
| `docs/mips/images/Attach_Front.jpg` | `jpg` | Documentación |
| `docs/mips/images/Baytech.png` | `png` | Documentación |
| `docs/mips/images/BaytechWiring.png` | `png` | Documentación |
| `docs/mips/images/Complete.jpg` | `jpg` | Documentación |
| `docs/mips/images/Complete_transceiver_board.jpg` | `jpg` | Documentación |
| `docs/mips/images/Completed_transceiver.jpg` | `jpg` | Documentación |
| `docs/mips/images/DB9M.png` | `png` | Documentación |
| `docs/mips/images/DB9_serial_port_wiring.jpg` | `jpg` | Documentación |
| `docs/mips/images/Drilled_front.jpg` | `jpg` | Documentación |
| `docs/mips/images/Front.jpg` | `jpg` | Documentación |
| `docs/mips/images/Front_sticker.jpg` | `jpg` | Documentación |
| `docs/mips/images/Impcode.png` | `png` | Documentación |
| `docs/mips/images/JTAGunbuffered.png` | `png` | Documentación |
| `docs/mips/images/NullModem.png` | `png` | Documentación |
| `docs/mips/images/Opening-linksys.jpg` | `jpg` | Documentación |
| `docs/mips/images/Serial.jpg` | `jpg` | Documentación |
| `docs/mips/images/Serial_Port_Connection.jpg` | `jpg` | Documentación |
| `docs/mips/images/Serial_ports_diagram.jpg` | `jpg` | Documentación |
| `docs/mips/images/Serial_ports_done.jpg` | `jpg` | Documentación |
| `docs/mips/images/Serial_ports_done1.jpg` | `jpg` | Documentación |
| `docs/mips/images/Te_jtag_cable.png` | `png` | Documentación |
| `docs/mips/images/Tranceiver_attached.jpg` | `jpg` | Documentación |
| `docs/mips/images/Transceiver_board.jpg` | `jpg` | Documentación |
| `docs/mips/images/Transceiver_schematic.jpg` | `jpg` | Documentación |
| `docs/mips/images/Transceiver_schematic_wl-330gE.jpg` | `jpg` | Documentación |
| `docs/mips/images/UartAsyncDriver.png` | `png` | Documentación |
| `docs/mips/images/Wiggler.png` | `png` | Documentación |
| `docs/mips/images/Wrt54gl-layout.jpg` | `jpg` | Documentación |
| `docs/mips/images/Xinu-Wiggler.png` | `png` | Documentación |
| `docs/projects/Curses.rst` | `rst` | Documentación |
| `docs/projects/WinX.rst` | `rst` | Documentación |
| `docs/projects/WinXinu-Installation.rst` | `rst` | Documentación |
| `docs/projects/WinXinu.rst` | `rst` | Documentación |
| `docs/projects/XinuPhone.rst` | `rst` | Documentación |
| `docs/projects/Xinuphone-schematic-page1.png` | `png` | Documentación |
| `docs/projects/Xinuphone-schematic-page2.png` | `png` | Documentación |
| `docs/projects/Xinuphone-system.jpg` | `jpg` | Documentación |
| `docs/projects/Xipx.rst` | `rst` | Documentación |
| `docs/projects/index.rst` | `rst` | Documentación |
| `docs/riscv/RISCV-qemu.rst` | `rst` | Documentación |
| `docs/riscv/index.rst` | `rst` | Documentación |
| `docs/teaching/Adding-Monitors-To-Xinu.rst` | `rst` | Documentación |
| `docs/teaching/Compiler-Construction-With-Embedded-Xinu.rst` | `rst` | Documentación |
| `docs/teaching/HOWTO-Build-Backend-Pool.rst` | `rst` | Documentación |
| `docs/teaching/HOWTO-Deploy-Xinu.rst` | `rst` | Documentación |
| `docs/teaching/Laboratory.rst` | `rst` | Documentación |
| `docs/teaching/Networking-With-Xinu.rst` | `rst` | Documentación |
| `docs/teaching/Student-Built-Xinu.rst` | `rst` | Documentación |
| `docs/teaching/Student-Extended-Xinu.rst` | `rst` | Documentación |
| `docs/teaching/Xinu-Helper-Class.rst` | `rst` | Documentación |
| `docs/teaching/index.rst` | `rst` | Documentación |
| `docs/teaching/assignments/ARP.rst` | `rst` | Documentación |
| `docs/teaching/assignments/Asynchronous-Device-Driver.rst` | `rst` | Documentación |
| `docs/teaching/assignments/Basic-Networking---Ping.rst` | `rst` | Documentación |
| `docs/teaching/assignments/C-Basics.rst` | `rst` | Documentación |
| `docs/teaching/assignments/C-Structs-and-Pointers.rst` | `rst` | Documentación |
| `docs/teaching/assignments/Context-Switch-and-Non-Preemptive-Scheduling.rst` | `rst` | Documentación |
| `docs/teaching/assignments/DHCP-Implementation.rst` | `rst` | Documentación |
| `docs/teaching/assignments/Delta-Queues.rst` | `rst` | Documentación |
| `docs/teaching/assignments/Heap-Memory.rst` | `rst` | Documentación |
| `docs/teaching/assignments/IP-ICMP-ARP-Applications.rst` | `rst` | Documentación |
| `docs/teaching/assignments/IP-ICMP.rst` | `rst` | Documentación |
| `docs/teaching/assignments/Instruction-Selection.rst` | `rst` | Documentación |
| `docs/teaching/assignments/LL-SC.rst` | `rst` | Documentación |
| `docs/teaching/assignments/Networking-Applications.rst` | `rst` | Documentación |
| `docs/teaching/assignments/Networking-Standards.rst` | `rst` | Documentación |
| `docs/teaching/assignments/Packet-Demultiplexing.rst` | `rst` | Documentación |
| `docs/teaching/assignments/Parser.rst` | `rst` | Documentación |
| `docs/teaching/assignments/Preemption-and-Synchronization.rst` | `rst` | Documentación |
| `docs/teaching/assignments/Priority-Scheduling-and-Preemption.rst` | `rst` | Documentación |
| `docs/teaching/assignments/Priority-Scheduling-and-Process-Termination.rst` | `rst` | Documentación |
| `docs/teaching/assignments/Scanner.rst` | `rst` | Documentación |
| `docs/teaching/assignments/Semantic-Analysis.rst` | `rst` | Documentación |
| `docs/teaching/assignments/Synchronization-and-Interprocess-Communication.rst` | `rst` | Documentación |
| `docs/teaching/assignments/Synchronous-Serial-Driver.rst` | `rst` | Documentación |
| `docs/teaching/assignments/TCP-Implementation.rst` | `rst` | Documentación |
| `docs/teaching/assignments/Translation.rst` | `rst` | Documentación |
| `docs/teaching/assignments/UDP-Implementation.rst` | `rst` | Documentación |
| `docs/teaching/assignments/Ultra-Tiny-File-System.rst` | `rst` | Documentación |
| `docs/teaching/assignments/index.rst` | `rst` | Documentación |
| `fs/Makerules` | `sin extensión` | Sistema de archivos FAT |
| `fs/Src/disk.c` | `c` | Sistema de archivos FAT |
| `fs/Src/fatPort.c` | `c` | Sistema de archivos FAT |
| `fs/Src/fat_access.c` | `c` | Acceso de bajo nivel a FAT |
| `fs/Src/fat_cache.c` | `c` | Sistema de archivos FAT |
| `fs/Src/fat_filelib.c` | `c` | API de ficheros FAT |
| `fs/Src/fat_format.c` | `c` | Sistema de archivos FAT |
| `fs/Src/fat_misc.c` | `c` | Sistema de archivos FAT |
| `fs/Src/fat_string.c` | `c` | Sistema de archivos FAT |
| `fs/Src/fat_table.c` | `c` | Sistema de archivos FAT |
| `fs/Src/fat_write.c` | `c` | Sistema de archivos FAT |
| `include/arp.h` | `h` | Cabecera pública del kernel |
| `include/assert.h` | `h` | Cabecera pública del kernel |
| `include/backplane.h` | `h` | Cabecera pública del kernel |
| `include/board.h` | `h` | Cabecera pública del kernel |
| `include/bootinfo.h` | `h` | Contrato de datos loader ↔ Xinu (v7) |
| `include/bufpool.h` | `h` | Cabecera pública del kernel |
| `include/button.h` | `h` | Cabecera pública del kernel |
| `include/clock.h` | `h` | Cabecera pública del kernel |
| `include/compiler.h` | `h` | Cabecera pública del kernel |
| `include/conf.h` | `h` | Cabecera pública del kernel |
| `include/ctype.h` | `h` | Cabecera pública del kernel |
| `include/date.h` | `h` | Cabecera pública del kernel |
| `include/debug.h` | `h` | Cabecera pública del kernel |
| `include/device.h` | `h` | devtab[], struct devsw y tipos de dispositivo |
| `include/dhcpc.h` | `h` | Cabecera pública del kernel |
| `include/disk.h` | `h` | Cabecera pública del kernel |
| `include/endianness.h` | `h` | Cabecera pública del kernel |
| `include/ether.h` | `h` | Cabecera pública del kernel |
| `include/ethernet.h` | `h` | Cabecera pública del kernel |
| `include/fat_access.h` | `h` | Cabecera pública del kernel |
| `include/fat_cache.h` | `h` | Cabecera pública del kernel |
| `include/fat_defs.h` | `h` | Cabecera pública del kernel |
| `include/fat_filelib.h` | `h` | Cabecera pública del kernel |
| `include/fat_format.h` | `h` | Cabecera pública del kernel |
| `include/fat_list.h` | `h` | Cabecera pública del kernel |
| `include/fat_misc.h` | `h` | Cabecera pública del kernel |
| `include/fat_opts.h` | `h` | Cabecera pública del kernel |
| `include/fat_port.h` | `h` | Cabecera pública del kernel |
| `include/fat_string.h` | `h` | Cabecera pública del kernel |
| `include/fat_table.h` | `h` | Cabecera pública del kernel |
| `include/fat_types.h` | `h` | Cabecera pública del kernel |
| `include/fat_write.h` | `h` | Cabecera pública del kernel |
| `include/flash.h` | `h` | Cabecera pública del kernel |
| `include/gpio.h` | `h` | Cabecera pública del kernel |
| `include/icmp.h` | `h` | Cabecera pública del kernel |
| `include/ipv4.h` | `h` | Cabecera pública del kernel |
| `include/kernel.h` | `h` | API del kernel y constantes globales |
| `include/limits.h` | `h` | Cabecera pública del kernel |
| `include/mailbox.h` | `h` | Cabecera pública del kernel |
| `include/memory.h` | `h` | Cabecera pública del kernel |
| `include/memtty.h` | `h` | Cabecera pública del kernel |
| `include/monitor.h` | `h` | Cabecera pública del kernel |
| `include/nam.h` | `h` | Cabecera pública del kernel |
| `include/name.h` | `h` | Cabecera pública del kernel |
| `include/network.h` | `h` | Cabecera pública del kernel |
| `include/null.h` | `h` | Cabecera pública del kernel |
| `include/platform.h` | `h` | Cabecera pública del kernel |
| `include/psram.h` | `h` | Cabecera pública del kernel |
| `include/queue.h` | `h` | Cabecera pública del kernel |
| `include/raw.h` | `h` | Cabecera pública del kernel |
| `include/rgb.h` | `h` | Cabecera pública del kernel |
| `include/route.h` | `h` | Cabecera pública del kernel |
| `include/sbconfig.h` | `h` | Cabecera pública del kernel |
| `include/sd.h` | `h` | Cabecera pública del kernel |
| `include/semaphore.h` | `h` | Cabecera pública del kernel |
| `include/shell.h` | `h` | Prototipos de comandos y constantes del shell |
| `include/st7735.h` | `h` | Cabecera pública del kernel |
| `include/stack.h` | `h` | Cabecera pública del kernel |
| `include/stdarg.h` | `h` | Cabecera pública del kernel |
| `include/stddef.h` | `h` | Cabecera pública del kernel |
| `include/stdint.h` | `h` | Cabecera pública del kernel |
| `include/stdio.h` | `h` | Cabecera pública del kernel |
| `include/stdlib.h` | `h` | Cabecera pública del kernel |
| `include/string.h` | `h` | Cabecera pública del kernel |
| `include/syscall.h` | `h` | Cabecera pública del kernel |
| `include/tcp.h` | `h` | Cabecera pública del kernel |
| `include/telnet.h` | `h` | Cabecera pública del kernel |
| `include/tft.h` | `h` | Cabecera pública del kernel |
| `include/tftp.h` | `h` | Cabecera pública del kernel |
| `include/thread.h` | `h` | Hilos, prioridades y tamaños de pila |
| `include/tty.h` | `h` | Cabecera pública del kernel |
| `include/uart.h` | `h` | Cabecera pública del kernel |
| `include/udp.h` | `h` | Cabecera pública del kernel |
| `include/usb_std_defs.h` | `h` | Cabecera pública del kernel |
| `include/usb_util.h` | `h` | Cabecera pública del kernel |
| `include/version.h` | `h` | Cabecera pública del kernel |
| `include/vlan.h` | `h` | Cabecera pública del kernel |
| `include/w5500.h` | `h` | Cabecera pública del kernel |
| `include/webterm.h` | `h` | Cabecera pública del kernel |
| `include/xinu_image.h` | `h` | Formato empaquetado opcional XIMG |
| `include/xsh_fs.h` | `h` | Cabecera pública del kernel |
| `include/xsh_http.h` | `h` | Cabecera pública del kernel |
| `include/xtensa.h` | `h` | Cabecera pública del kernel |
| `jvm/LICENSE` | `sin extensión` | Simple JVM: intérprete o soporte |
| `jvm/build-examples.sh` | `sh` | Compila librerías, tests y apps Java |
| `jvm/bytecodes.c` | `c` | Intérprete de bytecodes |
| `jvm/class_parser.c` | `c` | Parser de la cabecera .class |
| `jvm/constant_pool_parser.c` | `c` | Parser del constant pool |
| `jvm/field_pool_parser.c` | `c` | Simple JVM: intérprete o soporte |
| `jvm/free_pool.c` | `c` | Simple JVM: intérprete o soporte |
| `jvm/heap.c` | `c` | Objetos, arrays, String y StringBuilder |
| `jvm/interface_pool_parser.c` | `c` | Simple JVM: intérprete o soporte |
| `jvm/java_lib.c` | `c` | Tabla de natives y registro de librerías |
| `jvm/java_lib.h` | `h` | Simple JVM: intérprete o soporte |
| `jvm/java_std.c` | `c` | Natives de java.lang |
| `jvm/jvm_io.h` | `h` | Simple JVM: intérprete o soporte |
| `jvm/jvm_map.h` | `h` | Simple JVM: intérprete o soporte |
| `jvm/jvm_run.c` | `c` | Carga, <clinit>, main y liberación del estado |
| `jvm/method_pool_parser.c` | `c` | Simple JVM: intérprete o soporte |
| `jvm/simple_jvm.h` | `h` | Estado de la VM, StackFrame, heap y límites |
| `jvm/stack.c` | `c` | Pila de operandos y frames |
| `jvm/lib/esp32p4/FAT.java` | `java` | Simple JVM: intérprete o soporte |
| `jvm/lib/esp32p4/HW.java` | `java` | Simple JVM: intérprete o soporte |
| `jvm/lib/esp32p4/IO.java` | `java` | Simple JVM: intérprete o soporte |
| `jvm/lib/esp32p4/LVGL.java` | `java` | Simple JVM: intérprete o soporte |
| `jvm/lib/esp32p4/Net.java` | `java` | Simple JVM: intérprete o soporte |
| `jvm/lib/esp32p4/RGB.java` | `java` | Simple JVM: intérprete o soporte |
| `jvm/lib/esp32p4/Sys.java` | `java` | Simple JVM: intérprete o soporte |
| `jvm/tests/Args.java` | `java` | Simple JVM: intérprete o soporte |
| `jvm/tests/Arr.java` | `java` | Simple JVM: intérprete o soporte |
| `jvm/tests/Fat.java` | `java` | Simple JVM: intérprete o soporte |
| `jvm/tests/Fields1.java` | `java` | Simple JVM: intérprete o soporte |
| `jvm/tests/FloatTest.java` | `java` | Simple JVM: intérprete o soporte |
| `jvm/tests/IfElse.java` | `java` | Simple JVM: intérprete o soporte |
| `jvm/tests/Long1.java` | `java` | Simple JVM: intérprete o soporte |
| `jvm/tests/Loop.java` | `java` | Simple JVM: intérprete o soporte |
| `jvm/tests/MethodReturn.java` | `java` | Simple JVM: intérprete o soporte |
| `jvm/tests/NewByte.java` | `java` | Simple JVM: intérprete o soporte |
| `jvm/tests/NewString.java` | `java` | Simple JVM: intérprete o soporte |
| `jvm/tests/Recursion.java` | `java` | Simple JVM: intérprete o soporte |
| `jvm/tests/StaticVoidA.java` | `java` | Simple JVM: intérprete o soporte |
| `jvm/tests/Stdlib.java` | `java` | Simple JVM: intérprete o soporte |
| `jvm/tests/StrBuild.java` | `java` | Simple JVM: intérprete o soporte |
| `jvm/tests/Switch1.java` | `java` | Simple JVM: intérprete o soporte |
| `jvm/tests/SysTest.java` | `java` | Simple JVM: intérprete o soporte |
| `jvm/tests/Tetris.java` | `java` | Simple JVM: intérprete o soporte |
| `jvm/tests/Workspace.java` | `java` | Simple JVM: intérprete o soporte |
| `lib/Makerules` | `sin extensión` | libxc: libc mínima |
| `lib/libxc/Doxygroup.c` | `c` | libxc: libc mínima |
| `lib/libxc/Makefile` | `sin extensión` | libxc: libc mínima |
| `lib/libxc/abs.c` | `c` | libxc: libc mínima |
| `lib/libxc/atoi.c` | `c` | libxc: libc mínima |
| `lib/libxc/atol.c` | `c` | libxc: libc mínima |
| `lib/libxc/bzero.c` | `c` | libxc: libc mínima |
| `lib/libxc/calloc.c` | `c` | libxc: libc mínima |
| `lib/libxc/ctype_.c` | `c` | libxc: libc mínima |
| `lib/libxc/doprnt.c` | `c` | libxc: libc mínima |
| `lib/libxc/doscan.c` | `c` | libxc: libc mínima |
| `lib/libxc/fgetc.c` | `c` | libxc: libc mínima |
| `lib/libxc/fgets.c` | `c` | libxc: libc mínima |
| `lib/libxc/fprintf.c` | `c` | libxc: libc mínima |
| `lib/libxc/fputc.c` | `c` | libxc: libc mínima |
| `lib/libxc/fputs.c` | `c` | libxc: libc mínima |
| `lib/libxc/free.c` | `c` | libxc: libc mínima |
| `lib/libxc/fscanf.c` | `c` | libxc: libc mínima |
| `lib/libxc/labs.c` | `c` | libxc: libc mínima |
| `lib/libxc/malloc.c` | `c` | libxc: libc mínima |
| `lib/libxc/memchr.c` | `c` | libxc: libc mínima |
| `lib/libxc/memcmp.c` | `c` | libxc: libc mínima |
| `lib/libxc/memcpy.c` | `c` | libxc: libc mínima |
| `lib/libxc/memmove.c` | `c` | libxc: libc mínima |
| `lib/libxc/memset.c` | `c` | libxc: libc mínima |
| `lib/libxc/printf.c` | `c` | libxc: libc mínima |
| `lib/libxc/qsort.c` | `c` | libxc: libc mínima |
| `lib/libxc/rand.c` | `c` | libxc: libc mínima |
| `lib/libxc/realloc.c` | `c` | libxc: libc mínima |
| `lib/libxc/sprintf.c` | `c` | libxc: libc mínima |
| `lib/libxc/sscanf.c` | `c` | libxc: libc mínima |
| `lib/libxc/strchr.c` | `c` | libxc: libc mínima |
| `lib/libxc/strcmp.c` | `c` | libxc: libc mínima |
| `lib/libxc/strcpy.c` | `c` | libxc: libc mínima |
| `lib/libxc/strlcpy.c` | `c` | libxc: libc mínima |
| `lib/libxc/strlen.c` | `c` | libxc: libc mínima |
| `lib/libxc/strncat.c` | `c` | libxc: libc mínima |
| `lib/libxc/strncmp.c` | `c` | libxc: libc mínima |
| `lib/libxc/strncpy.c` | `c` | libxc: libc mínima |
| `lib/libxc/strnlen.c` | `c` | libxc: libc mínima |
| `lib/libxc/strrchr.c` | `c` | libxc: libc mínima |
| `lib/libxc/strstr.c` | `c` | libxc: libc mínima |
| `loader/arch/README` | `sin extensión` | Arranque, linker o plataforma del payload |
| `loader/platforms/esp32s3/Makerules` | `sin extensión` | Arranque, linker o plataforma del payload |
| `loader/platforms/esp32s3/start.S` | `S` | Entrada _start del payload AMP |
| `mailbox/Doxygroup.c` | `c` | Mailbox del kernel |
| `mailbox/Makerules` | `sin extensión` | Mailbox del kernel |
| `mailbox/mailboxAlloc.c` | `c` | Mailbox del kernel |
| `mailbox/mailboxCount.c` | `c` | Mailbox del kernel |
| `mailbox/mailboxFree.c` | `c` | Mailbox del kernel |
| `mailbox/mailboxInit.c` | `c` | Mailbox del kernel |
| `mailbox/mailboxReceive.c` | `c` | Mailbox del kernel |
| `mailbox/mailboxSend.c` | `c` | Mailbox del kernel |
| `network/Doxygroup.c` | `c` | Pila de red: Doxygroup.c |
| `network/Makerules` | `sin extensión` | Pila de red: Makerules |
| `network/arp/Doxygroup.c` | `c` | Pila de red: arp |
| `network/arp/Makerules` | `sin extensión` | Pila de red: arp |
| `network/arp/arpAlloc.c` | `c` | Pila de red: arp |
| `network/arp/arpDaemon.c` | `c` | Pila de red: arp |
| `network/arp/arpFree.c` | `c` | Pila de red: arp |
| `network/arp/arpGetEntry.c` | `c` | Pila de red: arp |
| `network/arp/arpInit.c` | `c` | Pila de red: arp |
| `network/arp/arpLearn.c` | `c` | Pila de red: arp |
| `network/arp/arpLookup.c` | `c` | Pila de red: arp |
| `network/arp/arpNotify.c` | `c` | Pila de red: arp |
| `network/arp/arpRecv.c` | `c` | Pila de red: arp |
| `network/arp/arpSendReply.c` | `c` | Pila de red: arp |
| `network/arp/arpSendRqst.c` | `c` | Pila de red: arp |
| `network/dhcpc/Doxygroup.c` | `c` | Pila de red: dhcpc |
| `network/dhcpc/Makerules` | `sin extensión` | Pila de red: dhcpc |
| `network/dhcpc/dhcp.h` | `h` | Pila de red: dhcpc |
| `network/dhcpc/dhcpClient.c` | `c` | Pila de red: dhcpc |
| `network/dhcpc/dhcpRecvReply.c` | `c` | Pila de red: dhcpc |
| `network/dhcpc/dhcpSendRequest.c` | `c` | Pila de red: dhcpc |
| `network/icmp/Doxygroup.c` | `c` | Pila de red: icmp |
| `network/icmp/Makerules` | `sin extensión` | Pila de red: icmp |
| `network/icmp/icmpDaemon.c` | `c` | Pila de red: icmp |
| `network/icmp/icmpDestUnreach.c` | `c` | Pila de red: icmp |
| `network/icmp/icmpEchoReply.c` | `c` | Pila de red: icmp |
| `network/icmp/icmpEchoRequest.c` | `c` | Pila de red: icmp |
| `network/icmp/icmpInit.c` | `c` | Pila de red: icmp |
| `network/icmp/icmpRecv.c` | `c` | Pila de red: icmp |
| `network/icmp/icmpRedirect.c` | `c` | Pila de red: icmp |
| `network/icmp/icmpSend.c` | `c` | Pila de red: icmp |
| `network/icmp/icmpTimeExceeded.c` | `c` | Pila de red: icmp |
| `network/ipv4/Doxygroup.c` | `c` | Pila de red: ipv4 |
| `network/ipv4/Makerules` | `sin extensión` | Pila de red: ipv4 |
| `network/ipv4/dot2ipv4.c` | `c` | Pila de red: ipv4 |
| `network/ipv4/ipv4Recv.c` | `c` | Pila de red: ipv4 |
| `network/ipv4/ipv4RecvDemux.c` | `c` | Pila de red: ipv4 |
| `network/ipv4/ipv4RecvValid.c` | `c` | Pila de red: ipv4 |
| `network/ipv4/ipv4Send.c` | `c` | Pila de red: ipv4 |
| `network/ipv4/ipv4SendFrag.c` | `c` | Pila de red: ipv4 |
| `network/net/Makerules` | `sin extensión` | Pila de red: net |
| `network/net/netChksum.c` | `c` | Pila de red: net |
| `network/net/netDown.c` | `c` | Pila de red: net |
| `network/net/netFreebuf.c` | `c` | Pila de red: net |
| `network/net/netGetbuf.c` | `c` | Pila de red: net |
| `network/net/netInit.c` | `c` | Inicialización de la pila de red |
| `network/net/netLookup.c` | `c` | Pila de red: net |
| `network/net/netRecv.c` | `c` | Pila de red: net |
| `network/net/netSend.c` | `c` | Pila de red: net |
| `network/net/netUp.c` | `c` | Activación de interfaz de red |
| `network/netaddr/Makerules` | `sin extensión` | Pila de red: netaddr |
| `network/netaddr/netaddrequal.c` | `c` | Pila de red: netaddr |
| `network/netaddr/netaddrhost.c` | `c` | Pila de red: netaddr |
| `network/netaddr/netaddrmask.c` | `c` | Pila de red: netaddr |
| `network/netaddr/netaddrsprintf.c` | `c` | Pila de red: netaddr |
| `network/route/Doxygroup.c` | `c` | Pila de red: route |
| `network/route/Makerules` | `sin extensión` | Pila de red: route |
| `network/route/rtAdd.c` | `c` | Pila de red: route |
| `network/route/rtAlloc.c` | `c` | Pila de red: route |
| `network/route/rtClear.c` | `c` | Pila de red: route |
| `network/route/rtDaemon.c` | `c` | Pila de red: route |
| `network/route/rtDefault.c` | `c` | Pila de red: route |
| `network/route/rtInit.c` | `c` | Pila de red: route |
| `network/route/rtLookup.c` | `c` | Pila de red: route |
| `network/route/rtRecv.c` | `c` | Pila de red: route |
| `network/route/rtRemove.c` | `c` | Pila de red: route |
| `network/route/rtSend.c` | `c` | Pila de red: route |
| `network/tftp/Doxygroup.c` | `c` | Pila de red: tftp |
| `network/tftp/Makerules` | `sin extensión` | Pila de red: tftp |
| `network/tftp/tftpGet.c` | `c` | Pila de red: tftp |
| `network/tftp/tftpGetIntoBuffer.c` | `c` | Pila de red: tftp |
| `network/tftp/tftpRecvPackets.c` | `c` | Pila de red: tftp |
| `network/tftp/tftpSendACK.c` | `c` | Pila de red: tftp |
| `network/tftp/tftpSendRRQ.c` | `c` | Pila de red: tftp |
| `shell/Doxygroup.c` | `c` | Shell, HTTP o terminal web |
| `shell/Makerules` | `sin extensión` | Shell, HTTP o terminal web |
| `shell/getopt.c` | `c` | Shell, HTTP o terminal web |
| `shell/httpapi.c` | `c` | API JSON de archivos y ejecución remota |
| `shell/lexan.c` | `c` | Shell, HTTP o terminal web |
| `shell/shell.c` | `c` | Intérprete y commandtab[] |
| `shell/webterm.c` | `c` | Terminal web, MEMTTY0 y worker webshell |
| `shell/xsh_arp.c` | `c` | Comando shell: arp |
| `shell/xsh_button.c` | `c` | Comando shell: button |
| `shell/xsh_cat.c` | `c` | Comando shell: cat |
| `shell/xsh_cd.c` | `c` | Comando shell: cd |
| `shell/xsh_clear.c` | `c` | Comando shell: clear |
| `shell/xsh_config.c` | `c` | Comando shell: config |
| `shell/xsh_cp.c` | `c` | Comando shell: cp |
| `shell/xsh_curl.c` | `c` | Comando shell: curl |
| `shell/xsh_date.c` | `c` | Comando shell: date |
| `shell/xsh_df.c` | `c` | Comando shell: df |
| `shell/xsh_dns.c` | `c` | Comando shell: dns |
| `shell/xsh_du.c` | `c` | Comando shell: du |
| `shell/xsh_dumpfile.c` | `c` | Comando shell: dumpfile |
| `shell/xsh_echo.c` | `c` | Comando shell: echo |
| `shell/xsh_exit.c` | `c` | Comando shell: exit |
| `shell/xsh_fat.c` | `c` | Comando shell: fat |
| `shell/xsh_free.c` | `c` | Comando shell: free |
| `shell/xsh_fsutil.c` | `c` | Comando shell: fsutil |
| `shell/xsh_gpio.c` | `c` | Comando shell: gpio |
| `shell/xsh_head.c` | `c` | Comando shell: head |
| `shell/xsh_help.c` | `c` | Comando shell: help |
| `shell/xsh_hexdump.c` | `c` | Comando shell: hexdump |
| `shell/xsh_httpd.c` | `c` | Servidor HTTP y ficheros estáticos |
| `shell/xsh_ifconfig.c` | `c` | Comando shell: ifconfig |
| `shell/xsh_ip.c` | `c` | Comando shell: ip |
| `shell/xsh_java.c` | `c` | Comando shell: java |
| `shell/xsh_kill.c` | `c` | Comando shell: kill |
| `shell/xsh_ls.c` | `c` | Comando shell: ls |
| `shell/xsh_memdump.c` | `c` | Comando shell: memdump |
| `shell/xsh_memstat.c` | `c` | Comando shell: memstat |
| `shell/xsh_mkdir.c` | `c` | Comando shell: mkdir |
| `shell/xsh_mon.c` | `c` | Comando shell: mon |
| `shell/xsh_mv.c` | `c` | Comando shell: mv |
| `shell/xsh_nc.c` | `c` | Comando shell: nc |
| `shell/xsh_netdown.c` | `c` | Comando shell: netdown |
| `shell/xsh_netstat.c` | `c` | Comando shell: netstat |
| `shell/xsh_netup.c` | `c` | Arranque de red, DHCP y espera de link |
| `shell/xsh_ping.c` | `c` | Comando shell: ping |
| `shell/xsh_ps.c` | `c` | ps: hilos, estado, prioridad y CPU% |
| `shell/xsh_psram.c` | `c` | Comando shell: psram |
| `shell/xsh_pwd.c` | `c` | Comando shell: pwd |
| `shell/xsh_rdate.c` | `c` | Comando shell: rdate |
| `shell/xsh_reset.c` | `c` | Comando shell: reset |
| `shell/xsh_rm.c` | `c` | Comando shell: rm |
| `shell/xsh_rmdir.c` | `c` | Comando shell: rmdir |
| `shell/xsh_route.c` | `c` | Comando shell: route |
| `shell/xsh_sd.c` | `c` | Comando shell: sd |
| `shell/xsh_sem.c` | `c` | Comando shell: sem |
| `shell/xsh_sleep.c` | `c` | Comando shell: sleep |
| `shell/xsh_snoop.c` | `c` | Comando shell: snoop |
| `shell/xsh_stack.c` | `c` | Comando shell: stack |
| `shell/xsh_stat.c` | `c` | Comando shell: stat |
| `shell/xsh_tail.c` | `c` | Comando shell: tail |
| `shell/xsh_tcpstat.c` | `c` | Comando shell: tcpstat |
| `shell/xsh_telnet.c` | `c` | Comando shell: telnet |
| `shell/xsh_telnetd.c` | `c` | Comando shell: telnetd |
| `shell/xsh_telnetserver.c` | `c` | Comando shell: telnetserver |
| `shell/xsh_tft.c` | `c` | Comando shell: tft |
| `shell/xsh_udpserver.c` | `c` | Servidor UDP de comandos y firmware |
| `shell/xsh_udpstat.c` | `c` | Comando shell: udpstat |
| `shell/xsh_w5500.c` | `c` | Comando shell: w5500 |
| `shell/xsh_wget.c` | `c` | Comando shell: wget |
| `shell/xsh_xmodem.c` | `c` | Comando shell: xmodem |
| `system/Doxygroup.c` | `c` | Kernel: núcleo, syscalls o scheduler |
| `system/Makerules` | `sin extensión` | Kernel: núcleo, syscalls o scheduler |
| `system/bfpalloc.c` | `c` | Kernel: núcleo, syscalls o scheduler |
| `system/bfpfree.c` | `c` | Kernel: núcleo, syscalls o scheduler |
| `system/buffree.c` | `c` | Kernel: núcleo, syscalls o scheduler |
| `system/bufget.c` | `c` | Kernel: núcleo, syscalls o scheduler |
| `system/chprio.c` | `c` | Kernel: núcleo, syscalls o scheduler |
| `system/clkhandler.c` | `c` | Kernel: núcleo, syscalls o scheduler |
| `system/clkinit.c` | `c` | Kernel: núcleo, syscalls o scheduler |
| `system/close.c` | `c` | Kernel: núcleo, syscalls o scheduler |
| `system/conf.c` | `c` | Kernel: núcleo, syscalls o scheduler |
| `system/control.c` | `c` | Kernel: núcleo, syscalls o scheduler |
| `system/create.c` | `c` | Kernel: núcleo, syscalls o scheduler |
| `system/debug.c` | `c` | Kernel: núcleo, syscalls o scheduler |
| `system/getc.c` | `c` | Kernel: núcleo, syscalls o scheduler |
| `system/getdev.c` | `c` | Kernel: núcleo, syscalls o scheduler |
| `system/getitem.c` | `c` | Kernel: núcleo, syscalls o scheduler |
| `system/getprio.c` | `c` | Kernel: núcleo, syscalls o scheduler |
| `system/gettid.c` | `c` | Kernel: núcleo, syscalls o scheduler |
| `system/heap.c` | `c` | Kernel: núcleo, syscalls o scheduler |
| `system/initialize.c` | `c` | nulluser(), sysinit() y tablas del kernel |
| `system/insert.c` | `c` | Kernel: núcleo, syscalls o scheduler |
| `system/insertd.c` | `c` | Kernel: núcleo, syscalls o scheduler |
| `system/ioerr.c` | `c` | Kernel: núcleo, syscalls o scheduler |
| `system/ionull.c` | `c` | Kernel: núcleo, syscalls o scheduler |
| `system/kill.c` | `c` | Kernel: núcleo, syscalls o scheduler |
| `system/lock.c` | `c` | Kernel: núcleo, syscalls o scheduler |
| `system/main.c` | `c` | Arranque de consola, TFT, red, servidores y shell |
| `system/mdelay.c` | `c` | Kernel: núcleo, syscalls o scheduler |
| `system/memfree.c` | `c` | Kernel: núcleo, syscalls o scheduler |
| `system/memget.c` | `c` | Kernel: núcleo, syscalls o scheduler |
| `system/moncount.c` | `c` | Kernel: núcleo, syscalls o scheduler |
| `system/moncreate.c` | `c` | Kernel: núcleo, syscalls o scheduler |
| `system/monfree.c` | `c` | Kernel: núcleo, syscalls o scheduler |
| `system/open.c` | `c` | Kernel: núcleo, syscalls o scheduler |
| `system/putc.c` | `c` | Kernel: núcleo, syscalls o scheduler |
| `system/queinit.c` | `c` | Kernel: núcleo, syscalls o scheduler |
| `system/queue.c` | `c` | Kernel: núcleo, syscalls o scheduler |
| `system/read.c` | `c` | Kernel: núcleo, syscalls o scheduler |
| `system/ready.c` | `c` | Kernel: núcleo, syscalls o scheduler |
| `system/receive.c` | `c` | Kernel: núcleo, syscalls o scheduler |
| `system/recvclr.c` | `c` | Kernel: núcleo, syscalls o scheduler |
| `system/recvtime.c` | `c` | Kernel: núcleo, syscalls o scheduler |
| `system/resched.c` | `c` | Kernel: núcleo, syscalls o scheduler |
| `system/resume.c` | `c` | Kernel: núcleo, syscalls o scheduler |
| `system/seek.c` | `c` | Kernel: núcleo, syscalls o scheduler |
| `system/semcount.c` | `c` | Kernel: núcleo, syscalls o scheduler |
| `system/semcreate.c` | `c` | Kernel: núcleo, syscalls o scheduler |
| `system/semfree.c` | `c` | Kernel: núcleo, syscalls o scheduler |
| `system/send.c` | `c` | Kernel: núcleo, syscalls o scheduler |
| `system/signal.c` | `c` | Kernel: núcleo, syscalls o scheduler |
| `system/signaln.c` | `c` | Kernel: núcleo, syscalls o scheduler |
| `system/sleep.c` | `c` | Kernel: núcleo, syscalls o scheduler |
| `system/softfloat.c` | `c` | Kernel: núcleo, syscalls o scheduler |
| `system/stack.c` | `c` | Kernel: núcleo, syscalls o scheduler |
| `system/stkget.c` | `c` | Kernel: núcleo, syscalls o scheduler |
| `system/suspend.c` | `c` | Kernel: núcleo, syscalls o scheduler |
| `system/udelay.c` | `c` | Kernel: núcleo, syscalls o scheduler |
| `system/unlock.c` | `c` | Kernel: núcleo, syscalls o scheduler |
| `system/unsleep.c` | `c` | Kernel: núcleo, syscalls o scheduler |
| `system/userret.c` | `c` | Kernel: núcleo, syscalls o scheduler |
| `system/wait.c` | `c` | Kernel: núcleo, syscalls o scheduler |
| `system/wakeup.c` | `c` | Kernel: núcleo, syscalls o scheduler |
| `system/write.c` | `c` | Kernel: núcleo, syscalls o scheduler |
| `system/xdone.c` | `c` | Kernel: núcleo, syscalls o scheduler |
| `system/yield.c` | `c` | Kernel: núcleo, syscalls o scheduler |
| `system/arch/README` | `sin extensión` | Kernel: arquitectura Xtensa |
| `system/arch/xtensa/ctxsw.S` | `S` | Cambio de contexto ABI CALL0 |
| `system/arch/xtensa/espclk.c` | `c` | Kernel: arquitectura Xtensa |
| `system/arch/xtensa/exception.c` | `c` | Kernel: arquitectura Xtensa |
| `system/arch/xtensa/halt.S` | `S` | Kernel: arquitectura Xtensa |
| `system/arch/xtensa/interrupt.h` | `h` | Kernel: arquitectura Xtensa |
| `system/arch/xtensa/intutils.S` | `S` | Kernel: arquitectura Xtensa |
| `system/arch/xtensa/panic.c` | `c` | Kernel: arquitectura Xtensa |
| `system/arch/xtensa/pause.S` | `S` | Kernel: arquitectura Xtensa |
| `system/arch/xtensa/setupStack.c` | `c` | Kernel: arquitectura Xtensa |
| `system/arch/xtensa/vectors.S` | `S` | Kernel: arquitectura Xtensa |
| `system/platforms/esp32s3/Makerules` | `sin extensión` | Kernel: plataforma ESP32-S3 |
| `system/platforms/esp32s3/bswapsi2.S` | `S` | Kernel: plataforma ESP32-S3 |
| `system/platforms/esp32s3/ctxsw.S` | `S` | Kernel: plataforma ESP32-S3 |
| `system/platforms/esp32s3/espclk.c` | `c` | Reloj CCOUNT desde bootinfo |
| `system/platforms/esp32s3/exception.c` | `c` | Kernel: plataforma ESP32-S3 |
| `system/platforms/esp32s3/halt.S` | `S` | Kernel: plataforma ESP32-S3 |
| `system/platforms/esp32s3/intc.c` | `c` | Interrupciones Xtensa del payload |
| `system/platforms/esp32s3/interrupt.h` | `h` | Kernel: plataforma ESP32-S3 |
| `system/platforms/esp32s3/intutils.S` | `S` | Kernel: plataforma ESP32-S3 |
| `system/platforms/esp32s3/panic.c` | `c` | Kernel: plataforma ESP32-S3 |
| `system/platforms/esp32s3/pause.S` | `S` | Kernel: plataforma ESP32-S3 |
| `system/platforms/esp32s3/platforminit.c` | `c` | Valida bootinfo y publica PSRAM |
| `system/platforms/esp32s3/setupStack.c` | `c` | Kernel: plataforma ESP32-S3 |
| `system/platforms/esp32s3/vectors.S` | `S` | Kernel: plataforma ESP32-S3 |
| `tools/gen_inventory.py` | `py` | Generador de este inventario |
| `tools/mkxinuimg.py` | `py` | Empaquetador opcional XIMG (no activo) |
| `web/README.md` | `md` | Endpoints, terminal, gestor y estructura web |
| `web/amp.html` | `html` | Interfaz web o build web |
| `web/build_web.py` | `py` | Genera y valida las páginas HTML |
| `web/docs.html` | `html` | Documentación técnica heredada |
| `web/index.html` | `html` | Dashboard, terminal web y gestor de archivos |
| `web/inventory.html` | `html` | Interfaz web o build web |
| `web/loader.html` | `html` | Interfaz web o build web |
| `web/manual.html` | `html` | Interfaz web o build web |
| `web/md2html.py` | `py` | Conversor Markdown → HTML |
| `web/natives.html` | `html` | Interfaz web o build web |

---

Total: **833 ficheros**.

Los módulos y los ficheros clave se explican en
[MANUAL-AMP.md](MANUAL-AMP.md), [MANUAL-LOADER.md](MANUAL-LOADER.md) y
[MANUAL-NATIVES.md](MANUAL-NATIVES.md).
