@REM esptool --chip esp8266 --port COM7 --baud 115200 write_flash --flash_size 1MB --flash_freq 40m --flash_mode dout --verify ^
    @REM 0xFB000 ..\..\espressif\ESP8266_NONOS_SDK\bin\blank.bin ^
    @REM 0xFD000 ..\..\espressif\ESP8266_NONOS_SDK\bin\blank.bin ^
    @REM 0xFE000 ..\..\espressif\ESP8266_NONOS_SDK\bin\blank.bin ^
    @REM 0xFF000 ..\..\espressif\ESP8266_NONOS_SDK\bin\blank.bin

esptool --chip esp8266 --port COM7 --baud 460800 write_flash --flash_size 1MB --flash_freq 40m --flash_mode dout --verify ^
    0xFC000 ..\..\espressif\ESP8266_NONOS_SDK\bin\esp_init_data_default_v08.bin ^
    0x0000 ..\..\espressif\ESP8266_NONOS_SDK\bin\boot_V1.7.bin ^
    0x01000 build_ota/1/main.elf-0x00000.bin ^
    0x11000 build_ota/1/main.elf-0x11000.bin ^
    0x81000 build_ota/2/main.elf-0x00000.bin ^
    0x91000 build_ota/2/main.elf-0x91000.bin
@REM 0x10000 build_ota/1/main.elf-0x00000.bin ^
@REM 0x19000 build_ota/1/main.elf-0x19000.bin ^
@REM 0x0000 build/main.elf-0x00000.bin ^
    
echo done.
pause