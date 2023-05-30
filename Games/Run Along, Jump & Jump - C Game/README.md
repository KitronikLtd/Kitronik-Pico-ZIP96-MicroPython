# Run Along, Jump & Jump - C Game
This folder contains all the files you need to get started programming in C for the Kitronik Pico ZIP96 Retro Gamer.

## Try the game
To try the game simply download one of the UF2 files in this folder and use the BOOTSEL button to load them onto your Pico.

The `rajaj-single.uf2` file contains a single player version of Run Along, Jump & Jump.

.The `rajaj-client.uf2` and `rajaj-server.uf2` files contain a wireless two player version of Run Along, Jump & Jump. This version of the game requires two Kitronik ZIP96 Retro Gamers and two Raspberry Pi Pico W's. One of the Pico W's needs to have the `rajaj-server.uf2` file loaded onto it. The other Pico W needs to have the `rajaj-client.uf2` file loaded onto it.

## ZIP96Pico
The `ZIP96Pico.c` and `ZIP96Pico.h` files provide the library functions that we can use to control the ZIP96 Retro Gamer.

This includes everything from the buttons, vibration motor and buzzer, to the screen and a Colour struct to help with storing RGB values for the ZIPLEDs.

Examples for how to use this library can be found in both the single player version in the `main-single.c` file, and the two player version of the game.

The ZIP96Pico C library also requires the `ws2812.pio` file, to run the PIO for control of the ZIPLEDs.

## PicoWNetworking
On top of the ZIP96Pico library, there is also a PicoWNetworking library available in the `PicoWNetworking.c` and `PicoWNetworking.h` files.

This provides an simpler interface for using the networking chip on the Pico.

Examples for how to use this library for both server and client functionality can be found in the `main-server.c` and `main-client.c` files.

The PicoWNetworking C library also requires the `lwipopts.h` file, to set common settings used for Pico W Networking.
