# CC = xtensa-lx106-elf-gcc
CC = "/mnt/d/Hobbies/programing/esp8266/espressif/xtensa-lx106-elf/bin/xtensa-lx106-elf-gcc"
FILENAME = main
BAUDRATE = 115200
INCLUDE_DIR = include
BUILD_DIR = build
SRC_DIR = src
BIN_DIR = image
SDK_DIR = "/mnt/d/Hobbies/programing/esp8266/espressif/ESP8266_NONOS_SDK/include/"
GCC_INCLUDE_DIR = "/mnt/d/Hobbies/programing/esp8266/espressif/xtensa-lx106-elf/xtensa-lx106-elf/include/"
LIB_DIR = "/mnt/d/Hobbies/programing/esp8266/espressif/ESP8266_NONOS_SDK/lib"
LIBKER_SCRIPTS_DIR = "/mnt/d/Hobbies/programing/esp8266/espressif/ESP8266_NONOS_SDK/ld"
_OBJ = $(FILENAME).o server_main.o server_leds.o command_parser.o command_executer.o ws2812_i2s.o
OBJ = $(patsubst %,$(BUILD_DIR)/%,$(_OBJ))

CFLAGS = -c -I$(INCLUDE_DIR)/ -I$(SDK_DIR) -I$(GCC_INCLUDE_DIR) -B$(LIBKER_SCRIPTS_DIR) -DICACHE_FLASH -mlongcalls -Wall -std=c99 #-Werror
CFLAGS_LINKING = -I$(INCLUDE_DIR)/ -I$(SDK_DIR) -I$(GCC_INCLUDE_DIR) -B$(LIBKER_SCRIPTS_DIR) -DICACHE_FLASH -mlongcalls -Wall -std=c99 #-Werror
LDLIBS = -Wl,--start-group -L$(LIB_DIR) -nostdlib -nodefaultlibs -lmain -lnet80211 -lwpa -llwip -lpp -lphy -lc -lcrypto -Wl,--end-group -lgcc
#may need -lcrypto when using IR 
LDFLAGS = -Tmy_linker_script.ld -Teagle.rom.addr.v6.ld
.PHONY: build clean upload

build $(BUILD_DIR)/$(FILENAME)-0x00000.bin: $(BUILD_DIR)/$(FILENAME).elf
	@echo "done! $^"
	esptool.py elf2image $^

$(BUILD_DIR)/$(FILENAME).elf: $(OBJ)
	$(CC) $(CFLAGS_LINKING) $(LDLIBS) $(LDFLAGS) -o $@ $^

$(BUILD_DIR)/$(FILENAME).o: $(SRC_DIR)/$(FILENAME).c $(INCLUDE_DIR)/server_main.h $(INCLUDE_DIR)/server_leds.h $(INCLUDE_DIR)/command_parser.h $(INCLUDE_DIR)/ws2812_i2s.h
	echo "$@, $^"
	$(CC) $(CFLAGS) $(LDLIBS) $(LDFLAGS) -o $@ $<
$(BUILD_DIR)/server_main.o: $(SRC_DIR)/server_main.c $(INCLUDE_DIR)/command_executer.h
	$(CC) $(CFLAGS) $(LDLIBS) $(LDFLAGS) -o $@ $<
$(BUILD_DIR)/server_leds.o: $(SRC_DIR)/server_leds.c $(INCLUDE_DIR)/command_parser.h $(INCLUDE_DIR)/command_executer.h
	$(CC) $(CFLAGS) $(LDLIBS) $(LDFLAGS) -o $@ $<
$(BUILD_DIR)/command_parser.o: $(SRC_DIR)/command_parser.c
	$(CC) $(CFLAGS) $(LDLIBS) $(LDFLAGS) -o $@ $<
$(BUILD_DIR)/command_executer.o: $(SRC_DIR)/command_executer.c $(INCLUDE_DIR)/command_parser.h $(INCLUDE_DIR)/ws2812_i2s.h
	$(CC) $(CFLAGS) $(LDLIBS) $(LDFLAGS) -o $@ $<
$(BUILD_DIR)/ws2812_i2s.o: $(SRC_DIR)/ws2812_i2s.c $(INCLUDE_DIR)/slc_register.h $(INCLUDE_DIR)/pin_mux_register.h
	$(CC) $(CFLAGS) $(LDLIBS) $(LDFLAGS) -o $@ $<

# and in general, make was called via WSL. maybe do that in vscode as well if there trouble.

upload: $(BUILD_DIR)/$(FILENAME).elf-0x00000.bin $(BUILD_DIR)/$(FILENAME).elf-0x10000.bin
# sudo -S chmod 666 /dev/tty8
	./tty_access.sh
	esptool.py --port /dev/ttyS8 --baud $(BAUDRATE) write_flash 0 $(BUILD_DIR)/$(FILENAME).elf-0x00000.bin 0x10000 $(BUILD_DIR)/$(FILENAME).elf-0x10000.bin

clean:
	rm -f $(BUILD_DIR)/*