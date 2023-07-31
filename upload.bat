@REM esptool --chip esp8266 --port COM7 --baud 115200 write_flash --flash_size 1MB --flash_freq 40m --flash_mode dout --verify ^
@REM     0xFB000 ..\..\espressif\ESP8266_NONOS_SDK\bin\blank.bin ^
@REM     0xFE000 ..\..\espressif\ESP8266_NONOS_SDK\bin\blank.bin

@REM 0xFC000 ..\..\espressif\ESP8266_NONOS_SDK\bin\esp_init_data_default_v08.bin ^
esptool --chip esp8266 --port COM7 --baud 115200 write_flash --flash_size 1MB --flash_freq 40m --flash_mode dout --verify ^
    0xFC000 ..\..\espressif\ESP8266_NONOS_SDK_V2.2\bin\esp_init_data_default_v08.bin ^
    0x00000 build/main.elf-0x00000.bin ^
    0x10000 build/main.elf-0x10000.bin 
echo done.
pause