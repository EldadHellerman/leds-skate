# ⛸️ leds-skate ⛸️
This is an old project of mine, back from 2018.

It is wifi controlled addressable LEDs, using esp8266, as well as an android app to control them.
The esp8266 also runs a low level interperter, so custom animation scripts can be written and executed.
The LEDs were mounted at the bottom of a pair of ice skates, each leg with it's own controller and battery.

I know an interpreter is way overcomplicated for the task at hand but it was done mainly for fun.

Originally there was an android app to control them as well, but it's so old I can't get it to work in android studio.
Anyway (and for that reason), I thing a browser based GUI will be best (which will be hosted on the esp8266 as well).

To control the addressable LED's (ws2812b and ws2812c 20*20) I'm using
[Charles Lohr WS2812 i2s library](https://github.com/cnlohr/esp8266ws2812i2s),
that does it with the very poorly documented DMA of the esp8266.

# Build
I'm building this with WSL (through VS code) and uploading to the chip from windows.

To set up the environment:
1. Clone [ESP8266_NONOS_SDK](https://github.com/espressif/ESP8266_NONOS_SDK) (works with release V3.0.5). This is the newer version of the SDK, with some changes over the older one.
2. Clone [esptool](https://github.com/espressif/esptool). This is used to upload the code to the chip.
3. Install xtensa-lx106-elf-gcc on WSL (as well as Make and maybe other dependencies).
4. Run make to build the binaries.
5. Use the upload script on windows to upload to the chip.
