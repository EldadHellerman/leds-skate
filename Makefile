CC = xtensa-lx106-elf-gcc
AR = xtensa-lx106-elf-ar
LD = xtensa-lx106-elf-ld
OBJ_COPY = xtensa-lx106-elf-objcopy
OBJ_DUMP = xtensa-lx106-elf-objdump
# CC = "/mnt/d/Hobbies/programing/esp8266/espressif/xtensa-lx106-elf/bin/xtensa-lx106-elf-gcc"

GCC_INCLUDE_DIR = "/mnt/d/Hobbies/programing/esp8266/espressif/xtensa-lx106-elf/xtensa-lx106-elf/include/"
FILENAME = main
INCLUDE_DIR = include
BUILD_DIR = build
SRC_DIR = src
BIN_DIR = image
# SDK_DIR = /mnt/d/Hobbies/programing/esp8266/espressif/ESP8266_NONOS_SDK_V2.2
SDK_DIR = /mnt/d/Hobbies/programing/esp8266/espressif/ESP8266_NONOS_SDK
SDK_INCLUDE_DIR = $(SDK_DIR)/include/
SDK_LIB_DIR = $(SDK_DIR)/lib
SDK_LINKER_SCRIPTS_DIR = $(SDK_DIR)/ld
LINKER_SCRIPT = linker_scripts/my_linker_script.ld

# BAUDRATE = 115200
# PORT = COM7

_OBJ = $(FILENAME).o server_main.o server_leds.o command_parser.o command_executer.o ws2812_i2s.o
OBJ = $(patsubst %,$(BUILD_DIR)/%,$(_OBJ))
CFLAGS = -c -I$(INCLUDE_DIR) -I$(SDK_INCLUDE_DIR) -I$(GCC_INCLUDE_DIR) -mlongcalls -DICACHE_FLASH -Wall -std=c99 #-Werror

LD_FLAGS = -L$(SDK_LIB_DIR) --start-group -lc -lgcc -lhal -lphy -lpp -lnet80211 -llwip -lwpa -lcrypto -lmain -ljson -lupgrade -lssl -lpwm -lsmartconfig --end-group
LD_FLAGS += -nostdlib -nodefaultlibs --no-check-sections --gc-sections -static
LD_FLAGS += -T$(LINKER_SCRIPT) -Map $(BUILD_DIR)/$(FILENAME).map
LD_FLAGS += -u call_user_start

.PHONY: build clean upload fresh

build: $(BUILD_DIR)/$(FILENAME)-0x00000.bin

$(BUILD_DIR)/$(FILENAME)-0x00000.bin: $(BUILD_DIR)/$(FILENAME).elf
	@echo "done! $^"
	esptool.py --chip esp8266 elf2image --use_segments --flash_size 1MB --flash_freq 40m --flash_mode dout $^
	esptool.py --chip esp8266 elf2image --flash_size 1MB --flash_freq 40m --flash_mode dout $^
	$(OBJ_DUMP) -t $^ > $(BUILD_DIR)/dissasembly-sections.txt
	$(OBJ_DUMP) -h $^ > $(BUILD_DIR)/dissasembly-headers.txt
	$(OBJ_DUMP) -d $^ > $(BUILD_DIR)/dissasembly.txt

#extracting app_partition.o from libmain.a since from some reason ld doesnt find system_partition_table_regist()
$(BUILD_DIR)/app_partition.o: $(SDK_DIR)/lib/libmain.a
	mkdir -p $(BUILD_DIR)/temp/
	cp $< $(BUILD_DIR)/temp/libmain.a
	cd $(BUILD_DIR)/temp/; $(AR) x libmain.a
	cd ../..
	cp $(BUILD_DIR)/temp/app_partition.o $(BUILD_DIR)/app_partition.o
	rm -f -r $(BUILD_DIR)/temp
	
$(BUILD_DIR)/$(FILENAME).elf: $(OBJ) $(BUILD_DIR)/app_partition.o
	@echo linking
#	$(AR) x $(SDK_LIB_DIR)/libmain.a
	$(LD) $(LD_FLAGS) -o $@ $^

$(BUILD_DIR)/$(FILENAME).o: $(SRC_DIR)/$(FILENAME).c $(INCLUDE_DIR)/server_main.h $(INCLUDE_DIR)/server_leds.h $(INCLUDE_DIR)/command_parser.h $(INCLUDE_DIR)/ws2812_i2s.h
	@echo compiling $@
	$(CC) $(CFLAGS) -o $@ $<
$(BUILD_DIR)/server_main.o: $(SRC_DIR)/server_main.c $(INCLUDE_DIR)/command_executer.h
	@echo compiling $@
	$(CC) $(CFLAGS) -o $@ $<
$(BUILD_DIR)/server_leds.o: $(SRC_DIR)/server_leds.c $(INCLUDE_DIR)/command_parser.h $(INCLUDE_DIR)/command_executer.h
	@echo compiling $@
	$(CC) $(CFLAGS) -o $@ $<
$(BUILD_DIR)/command_parser.o: $(SRC_DIR)/command_parser.c
	@echo compiling $@
	$(CC) $(CFLAGS) -o $@ $<
$(BUILD_DIR)/command_executer.o: $(SRC_DIR)/command_executer.c $(INCLUDE_DIR)/command_parser.h $(INCLUDE_DIR)/ws2812_i2s.h
	@echo compiling $@
	$(CC) $(CFLAGS) -o $@ $<
$(BUILD_DIR)/ws2812_i2s.o: $(SRC_DIR)/ws2812_i2s.c $(INCLUDE_DIR)/slc_register.h $(INCLUDE_DIR)/pin_mux_register.h
	@echo compiling $@
	$(CC) $(CFLAGS) -o $@ $<

upload: $(BUILD_DIR)/$(FILENAME).elf-0x00000.bin $(BUILD_DIR)/$(FILENAME).elf-0x10000.bin
#   sudo -S chmod 666 /dev/tty8
#	./tty_access.sh
#	esptool.py --port /dev/ttyS8 --baud $(BAUDRATE) write_flash 0 $(BUILD_DIR)/$(FILENAME).elf-0x00000.bin 0x10000 $(BUILD_DIR)/$(FILENAME).elf-0x10000.bin

clean:
	rm -f -r $(BUILD_DIR)/*
	clear

fresh: clean build