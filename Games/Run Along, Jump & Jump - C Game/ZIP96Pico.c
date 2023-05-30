/*
MIT License

Copyright (c) 2022 Kitronik Ltd 

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "hardware/pwm.h"

#include "pico/stdlib.h"

#include "ws2812.pio.h"

#include "ZIP96Pico.h"

/*
*
Library for the Pico 96 LED retro gamer.
*
*/

/*
The button functions enable the use of the user input buttons on the board.
*/
void buttonInit(uint8_t pin) {
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_IN);
    gpio_pull_down(pin);
}

// Check whether the given button is currently being pressed.
bool buttonPressed(uint8_t pin) {
    return gpio_get(pin);
}

/*
The vibration functions enable the use of the vibration motor on the board.
*/
void vibrationInit() {
    gpio_init(VIBRATION_PIN);
    gpio_set_dir(VIBRATION_PIN, GPIO_OUT);
    vibrationStop();
}

// Turn on the vibration motor.
void vibrationVibrate() {
    gpio_put(VIBRATION_PIN, true);
}

// Turn off the vibration motor.
void vibrationStop() {
    gpio_put(VIBRATION_PIN, false);
}

/*
The buzzer functions enable control of the piezo buzzer on the board.
*/
uint buzzerSlice, buzzerChannel;

// Setup the piezo buzzer at the specified pin.
void buzzerInit() {
    gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM);
    buzzerSlice = pwm_gpio_to_slice_num(BUZZER_PIN);
    buzzerChannel = pwm_gpio_to_channel(BUZZER_PIN);
}

// Play a continous tone at a specified frequency.
void buzzerPlayTone(uint16_t frequency) {
    pwm_set_enabled(buzzerSlice, true);
    uint32_t clock = 125000000;
    uint32_t divider16 = clock / frequency / 4096 + (clock % (frequency * 4096) != 0);
    if (divider16 / 16 == 0)
        divider16 = 16;
    uint32_t wrap = clock * 16 / divider16 / frequency - 1;
    pwm_set_clkdiv_int_frac(buzzerSlice, divider16 / 16, divider16 & 0xF);
    pwm_set_wrap(buzzerSlice, wrap);
    pwm_set_chan_level(buzzerSlice, buzzerChannel, wrap * 50 / 100);
}

// Play a tone at a speciied frequency for a specified length of time in ms.
void buzzerPlayToneLength(uint16_t frequency, uint16_t length) {
    buzzerPlayTone(frequency);
    sleep_ms(length);
    buzzerStopTone();
}

// Stop the buzzer producing a tone.
void buzzerStopTone() {
    pwm_set_enabled(buzzerSlice, false);
}

/*
The screen functions enable control of the ZIP LEDs on the board.
*/
uint8_t redLEDs[NUM_LEDS], greenLEDs[NUM_LEDS], blueLEDs[NUM_LEDS];
float brightness = 0.1;
PIO pio;
uint sm;

// We drive the ZIP LEDs using a PIO program which we start here.
void screenInit() {
    pio = pio0;
    sm = pio_claim_unused_sm(pio, false);
    if (sm == -1) {
        pio = pio1;
        sm = pio_claim_unused_sm(pio, false);
    }
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, LED_PIN, 800000, false);
}

// Show pushes the current setup of the LEDs to the physical LEDs - it makes them visible.
void screenShow() {
    uint32_t LED = 0;
    for (uint8_t i = 0; i < NUM_LEDS; i++) {
        LED = ((uint32_t) (redLEDs[i] * brightness) << 8) |
            ((uint32_t) (greenLEDs[i] * brightness) << 16) |
            (uint32_t) (blueLEDs[i] * brightness);
        pio_sm_put_blocking(pio, sm, LED << 8u);
    }
}

// Sets the colour of an individual LED. Use show to make change visible.
void screenSetLED(uint8_t i, Colour colour) {
    redLEDs[i] = colour.r;
    greenLEDs[i] = colour.g;
    blueLEDs[i] = colour.b;
}

// Sets the colour of an individual LED using a matrix. Use show to make change visible.
void screenSetLEDMatrix(uint8_t x, uint8_t y, Colour colour) {
    uint8_t i = x + (y * SCREEN_WIDTH);
    screenSetLED(i, colour);
}

// Sets the colour of all LEDs to be the same.
void screenFill(Colour colour) {
    for (uint8_t i = 0; i < NUM_LEDS; i++)
        screenSetLED(i, colour);
}

// Takes 0-100 as a brightness value, brighness is applies in the show function.
void screenSetBrightness(uint8_t value) {
    if (value < 0) {
        value = 0;
    } else if (value > 100) {
        value = 100;
    }

    brightness = value / 100.f;
}

// Initialise all of the ZIP96 Retro Gamer board components.
void initKitronikZIP96() {
    buttonInit(UP_PIN);
    buttonInit(DOWN_PIN);
    buttonInit(LEFT_PIN);
    buttonInit(RIGHT_PIN);
    buttonInit(A_PIN);
    buttonInit(B_PIN);

    vibrationInit();
    buzzerInit();
    screenInit();
}
