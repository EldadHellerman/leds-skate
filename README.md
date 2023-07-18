# ⛸️ leds-skate ⛸️
This is an old project of mine, back from 2018.

It is wifi controlled addressable LEDs, using esp8266, as well as an android app to control them.
The esp8266 also runs a low level interperter, so custom animation scripts can be written and executed.
The LEDs were mounted at the bottom of a pair of ice skates, each leg with it's own controller and battery.

I know an interpreter is way overcomplicated for the task at hand but it was done mainly for fun.

Originally there was an android app to control them as well, but it's so old I can't get it to work in android studio.
Anyway (and for that reason), I thing a browser based GUI will be best (which will be hosted on the esp8266 as well).

To control the addressable LED's (ws2812b and ws2812c 20*20) I'm using
[Charles Lohr WS2812 i2s library]([https://github.com/vuejs/vue](https://github.com/cnlohr/esp8266ws2812i2s)),
that does it with the very poorly documented DMA of the esp8266.

# Build
I'm used the esp-open-sdk toolchain. will upate this once I found out the best way to build this in 2023.
