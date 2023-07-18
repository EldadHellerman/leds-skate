CC = $(gccpath)xtensa-lx106-elf-gcc
#codepath = /mnt/c/Users/Eldad/Desktop/folders/hobbys/Programing/ESP8266/esp-open-sdk/code/
#winpath = C:\Users\Eldad\Desktop\folders\hobbys\Programing\ESP8266\esp-open-sdk\code\
#dir = /mnt/c/Users/Eldad/Desktop/folders/hobbys/Programing/ESP8266/esp-open-sdk/ESP8266_NONOS_SDK-2.1.0-18-g61248df/include/
#-B /mnt/c/Users/Eldad/Desktop/folders/hobbys/Programing/ESP8266/esp-open-sdk/ESP8266_NONOS_SDK-2.1.0-18-g61248df/include/

FILENAME = main
BAUDRATE = 115200
OBJ = $(FILENAME).o server_main.o server_leds.o command_parser.o command_executer.o ws2812_i2s.o

CFLAGS = -I. -DICACHE_FLASH -mlongcalls -Wall -std=c99 #-Werror
LDLIBS = -nostdlib -Wl,--start-group -lmain -lnet80211 -lwpa -llwip -lpp -lphy -lc -lcrypto -Wl,--end-group -lgcc
#may need -lcrypto when using IR 
LDFLAGS = -Teagle.app.v6.ld -Teagle.rom.addr.v6.ld
.PHONY: all clean flash

all $(FILENAME)-0x00000.bin: $(FILENAME)
	esptool.py elf2image $^

$(FILENAME): $(OBJ)

$(FILENAME).o: $(FILENAME).c server_main.h server_leds.h command_parser.h ws2812_i2s.h
server_main.o: server_main.c command_executer.h
server_leds.o: server_leds.c command_parser.h command_executer.h
command_parser.o: command_parser.c
command_executer.o: command_executer.c command_parser.h ws2812_i2s.h
ws2812_i2s.o: ws2812_i2s.c slc_register.h pin_mux_register.h

# this was in eclipse build settings (run a bash script in wsl to enable serial port access):
# <buildCommand>wsl.exe</buildCommand>
# <buildArguments>echo "ehkl" | sudo -S chmod 666 /dev/ttyS10; make</buildArguments>
# <buildTarget>flash</buildTarget>

# and in general, make was called via WSL. maybe do that in vscode as well if there trouble.

flash: $(FILENAME)-0x00000.bin
	esptool.py --port /dev/ttyS10 --baud $(BAUDRATE) write_flash 0 $(FILENAME)-0x00000.bin 0x10000 $(FILENAME)-0x10000.bin

clean:
	rm -f $(FILENAME) $(FILENAME)-0x00000.bin $(FILENAME)-0x10000.bin $(OBJ)