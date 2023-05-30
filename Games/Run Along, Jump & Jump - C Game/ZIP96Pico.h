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

#ifndef ZIP96_PICO_H_
#define ZIP96_PICO_H_

typedef struct Colour {
    uint8_t r, g, b;
} Colour;

#define UP_PIN 14
#define DOWN_PIN 12
#define LEFT_PIN 13
#define RIGHT_PIN 15
#define A_PIN 1
#define B_PIN 2

void buttonInit(uint8_t pin);
bool buttonPressed(uint8_t pin);

#define VIBRATION_PIN 4

void vibrationInit();
void vibrationVibrate();
void vibrationStop();

#define BUZZER_PIN 5

void buzzerInit();
void buzzerPlayTone(uint16_t frequency);
void buzzerPlayToneLength(uint16_t frequency, uint16_t length);
void buzzerStopTone();

#define LED_PIN 7
#define NUM_LEDS 96
#define SCREEN_WIDTH 12
#define SCREEN_HEIGHT 8

void screenInit();
void screenShow();
void screenSetLED(uint8_t i, Colour colour);
void screenSetLEDMatrix(uint8_t x, uint8_t y, Colour colour);
void screenFill(Colour colour) ;
void screenSetBrightness(uint8_t value);

void initKitronikZIP96();

#endif // ZIP96_PICO_H_
