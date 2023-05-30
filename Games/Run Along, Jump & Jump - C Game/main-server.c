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

#include <stdio.h>
#include <stdlib.h>

#include "pico/cyw43_arch.h"
#include "pico/multicore.h"
#include "pico/stdlib.h"
#include "pico/util/queue.h"

#include "RAJAJ.h"
#include "ZIP96Pico.h"
#include "PicoWNetworking.h"

//
#include <string.h>
#include <time.h>
#include <errno.h>

#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "lwip/udp.h"
#include "cyw43_config.h"
//

Player player;
Player other;
Coin *coins;
uint8_t coinsLength = 0;
Walker *walkers;
uint8_t walkersLength = 0;
Floater *floaters;
uint8_t floatersLength = 0;

uint8_t xOffset = 0;
char (*layout)[8][50];
uint8_t xLength, yLength;

uint8_t screenBrightness = 10;

uint8_t level = 1;
bool levelOver = true;
char levelWinner = 'P';
uint8_t delayCount = 0;

queue_t playerQueue;
queue_t otherQueue;

void networking();
Colour getColour(char character);
void updateScreen();
void reset(uint8_t level);

int main() {
    stdio_init_all();

    // Setup the gamer and screen.
    initKitronikZIP96();
    screenFill(getColour('S'));
    screenShow();

    // Setup the queues to send messages between the game and networking threads.
    queue_init(&playerQueue, sizeof(Config), QUEUE_SIZE);
    queue_init(&otherQueue, sizeof(Config), QUEUE_SIZE);
    multicore_launch_core1(networking);

    // Setup the two players.
    player = playerInit(0, 6);
    other = playerInit(1, 6);
    
    // Loop until all levels are complete.
    while (level < 9) {
        // Delay to allow other device to catch up.
        sleep_ms(2000);
        
        // Loop until level is setup.
        while (levelOver) {
            // When A button is pressed, reset the game.
            if (buttonPressed(A_PIN)) {
                levelOver = false;
            }

            sleep_ms(150);
        }

        // Reset the game.
        queue_free(&otherQueue);
        reset(level);
        xOffset = 0;
        delayCount = 0;

        // Loop until level is over.
        while (!levelOver) {
            // Increase brightness when Up pressed.
            if (buttonPressed(UP_PIN)) {
                if (screenBrightness < 30) {
                    screenBrightness += 5;
                    screenSetBrightness(screenBrightness);
                }
            }
            
            // Decrease brightness when Down pressed.
            if (buttonPressed(DOWN_PIN)) {
                if (screenBrightness != 5) {
                    screenBrightness -= 5;
                    screenSetBrightness(screenBrightness);
                }
            }
            
            // Set player to jumping when A pressed.
            if (buttonPressed(A_PIN)) {
                //  Only when they aren't jumping or falling.
                if (player.jump < 0 && !player.falling) {
                    player.jump = 4;
                }
            }
            
            // Move player left when Left pressed.
            if (buttonPressed(LEFT_PIN)) {
                playerLeft(&player, layout);
            }
            
            // Move player right when Right pressed.
            if (buttonPressed(RIGHT_PIN)) {
                playerRight(&player, layout, xOffset + SCREEN_WIDTH);
            }
            
            if (player.jump > 0) {
                // Move player up when jumping.
                playerUp(&player, layout);
            } else {
                // Move player down when not jumping (gravity).
                playerDown(&player, layout, yLength, walkers, walkersLength);
            }

            // Update number of coins collected.
            playerCheckCoins(&player, coins, coinsLength);
            uint8_t coinCount = 0;
            for (uint8_t i = 0; i < coinsLength; i++) {
                if (coins[i].collected) {
                    coinCount++;
                }
            }

            // End the game when player or other reaches the House,
            // Only when they have collected all the coins between them.
            if ((*layout)[player.y][player.x] == 'H' && coinCount == coinsLength) {
                levelOver = true;
                levelWinner = 'P';
            }

            // Loop through each walker in walkers.
            for (uint8_t i = 0; i < walkersLength; i++) {
                // Ignore walker when they're dead.
                if (walkers[i].dead) {
                    continue;
                }

                // Move walker.
                walkerMove(&walkers[i], layout, xLength);

                // End the game when walker collides with player.
                if (playerCollision(&player, walkers[i].x, walkers[i].y)) {
                    levelOver = true;
                    levelWinner = 'W';
                    player.dead = true;
                }
            }

            // Loop through each floater in floaters.
            for (uint8_t i = 0; i < floatersLength; i++) {
                // Move floater.
                floaterMove(&floaters[i], layout, yLength);

                // End the game when floater collides with player.
                if (playerCollision(&player, floaters[i].x, floaters[i].y)) {
                    levelOver = true;
                    levelWinner = 'F';
                    player.dead = true;
                }
            }

            // Update the screen
            updateScreen();

            // Add the current level layout configuration to the playerQueue.
            if (!queue_is_full(&playerQueue)) {
                Config config = {player.x, player.y, player.coins};
                for (uint8_t i = 0; i < coinsLength; i++) {
                    config.collected[i] = coins[i].collected ? 1 : 0;
                }
                for (uint8_t i = 0; i < walkersLength; i++) {
                    config.dead[i] = walkers[i].dead ? 1 : 0;
                }
                queue_add_blocking(&playerQueue, &config);
            }

            // Remove the other players level layout configuration from the otherQueue.
            if (!queue_is_empty(&otherQueue)) {
                Config config;
                queue_remove_blocking(&otherQueue, &config);
                if (config.coins == GAME_STOP) {
                    levelOver = true;
                    // level = config.x;
                    levelWinner = config.y;
                } else {
                    if (delayCount > 1) {
                        other.x = config.x;
                        other.y = config.y;
                        other.coins = config.coins;
                        for (uint8_t i = 0; i < coinsLength; i++) {
                            if (config.collected[i] == 1) {
                                coins[i].collected = true;
                            }
                        }
                        for (uint8_t i = 0; i < walkersLength; i++) {
                            if (config.dead[i] == 1) {
                                walkers[i].dead = true;
                            }
                        }
                    } else {
                        delayCount++;
                    }
                }
            }
            
            sleep_ms(150);
        }

        // Move onto next level if the winner was a player.
        if (levelWinner == 'P') {
            player.coins = 0;
            other.coins = 0;
            level++;
        }

        vibrationVibrate();
        sleep_ms(500);
        vibrationStop();
        screenFill(getColour(levelWinner));
        screenShow();

        // Empty the playerQueue.
        while (!queue_is_empty(&playerQueue)) {
            Config config;
            queue_remove_blocking(&playerQueue, &config);
        }
        // Fill the playerQueue with GAME_STOP messages to send to the other player.
        while (!queue_is_full(&playerQueue)) {
            Config config = {level, levelWinner, GAME_STOP};
            queue_add_blocking(&playerQueue, &config);
        }
    }

    return 0;
}

void networking() {
    sleep_ms(150);
    screenSetLED(0, getColour('B'));
    screenShow();

    // Start the WAP for client to connect to.
    Server *state = serverInit();
    if (!state) {
        return; // No server state received.
    }

    // Start the TCP server for client to connect to.
    if (!serverOpen(state)) {
        serverResult(state, -1);
    }

    screenSetLED(1, getColour('C'));
    screenShow();

    // Waiting for connection to be established.
    while (!state->connected) {
        static int led_on = true;
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_on);
        led_on = !led_on;
        sleep_ms(1000);

        if (led_on) {
            screenSetLED(2, getColour('C'));
            screenShow();
        } else {
            screenSetLED(2, getColour('B'));
            screenShow();
        }
    }

    screenSetLED(3, getColour('G'));
    screenShow();

    Config config;

    // Loop until no longer connected.
    while (state->connected) {
        // Remove the players current level layout configuration from the playerQueue.
        if (!queue_is_empty(&playerQueue)) {
            queue_remove_blocking(&playerQueue, &config);
            // Process the config and send it to the client.
            if (config.coins == GAME_STOP) {
                uint8_t data[3];
                data[0] = config.x;
                data[1] = config.y;
                data[2] = config.coins;
                serverSend(state, data, 3);
            } else {
                uint8_t data[23];
                data[0] = config.x;
                data[1] = config.y;
                data[2] = config.coins;
                for (uint8_t i = 0; i < coinsLength; i++) {
                    data[3 + i] = config.collected[i];
                }
                for (uint8_t i = 0; i < walkersLength; i++) {
                    data[3 + coinsLength + i] = config.dead[i];
                }
                serverSend(state, data, 3 + coinsLength + walkersLength);
            }
        }

        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, true);
        sleep_ms(100);

        // Received data from the client.
        if (state->received) {
            state->received = false;
            // Process the received config and add it to the otherQueue.
            if (state->length == 3 + coinsLength + walkersLength) {
                if (!queue_is_full(&otherQueue)) {
                    Config config = {state->data[0], state->data[1], state->data[2]};
                    for (uint8_t i = 0; i < coinsLength; i++) {
                        if (3 + i >= state->length) {
                            break;
                        }
                        config.collected[i] = state->data[3 + i];
                    }
                    for (uint8_t i = 0; i < walkersLength; i++) {
                        if (3 + coinsLength + i >= state->length) {
                            break;
                        }
                        config.dead[i] = state->data[3 + coinsLength + i];
                    }
                    queue_add_blocking(&otherQueue, &config);
                }
            } else if (state->length == 3) {
                if (!queue_is_full(&otherQueue)) {
                    Config config = {state->data[0], state->data[1], GAME_STOP};
                    queue_add_blocking(&otherQueue, &config);
                }
            }
            state->length = 0;
        }
        
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, false);
        sleep_ms(100);
    }

    // Close the TCP server and WAP.
    serverClose(state);
    free(state);
}

// Return screen colour for a given character.
Colour getColour(char character) {
    // Sky
    Colour colour = {0, 0, 0};

    if (character == 'G') {
        // Grass
        colour.r = 0;
        colour.g = 255;
        colour.b = 0;
    } else if (character == 'D') {
        // Dirt
        colour.r = 139;
        colour.g = 69;
        colour.b = 19;
    } else if (character == 'B') {
        // Block
        colour.r = 255;
        colour.g = 0;
        colour.b = 0;
    } else if (character == 'P') {
        // Player
        colour.r = 0;
        colour.g = 0;
        colour.b = 255;
    } else if (character == 'C') {
        // Coin
        colour.r = 223;
        colour.g = 209;
        colour.b = 17;
    } else if (character == 'W') {
        // Walker
        colour.r = 255;
        colour.g = 20;
        colour.b = 147;
    } else if (character == 'R') {
        // Roof
        colour.r = 0;
        colour.g = 0;
        colour.b = 255;
    } else if (character == 'H') {
        // House
        colour.r = 255;
        colour.g = 255;
        colour.b = 255;
    } else if (character == 'F') {
        // Floater
        colour.r = 138;
        colour.g = 60;
        colour.b = 188;
    } else if (character == 'O') {
        // Other
        colour.r = 255;
        colour.g = 255;
        colour.b = 255;
    }

    return colour;
}

// Update the screen with new positions
void updateScreen() {
    // Used to detect when to move the screen left or right on the layout.
    uint8_t offsetWidth = SCREEN_WIDTH / 2;
    // Used to calculate the difference and middle between player and other.
    uint8_t playerDiff = 0;
    uint8_t playerMiddle = 0;

    if (player.x > other.x) {
        // When player is further right than other.
        playerDiff = player.x - other.x;
        playerMiddle = other.x + (playerDiff / 2);
    } else {
        // When other is further right than player.
        playerDiff = other.x - player.x;
        playerMiddle = player.x + (playerDiff / 2);
    }

    if (playerMiddle - xOffset >= offsetWidth) {
        // When middle on the right half of the screen.
        if (playerMiddle + offsetWidth < xLength) {
            // Move the screen right when there is space at the end of the layout.
            xOffset++;
        }
    } else if (playerMiddle - xOffset < offsetWidth - 1) {
        // When middle on the left half of the screen.
        if (playerMiddle - offsetWidth + 1 >= 0) {
            // Move the screen left when there is space at the start of the layout.
            xOffset--;
        }
    }
    
    Colour colour;

    // Draw the current level layout to the screen.
    for (uint8_t x = 0; x < SCREEN_WIDTH; x++) {
        for (uint8_t y = 0; y < SCREEN_HEIGHT; y++) {
            screenSetLEDMatrix(x, y, getColour((*layout)[y][x + xOffset]));
        }
    }
    
    // Draw player to the screen.
    screenSetLEDMatrix(player.x - xOffset, player.y, getColour('P'));
    // Draw other to the screen.
    if (other.x - xOffset >= 0 && other.x - xOffset < SCREEN_WIDTH) {
        screenSetLEDMatrix(other.x - xOffset, other.y, getColour('O'));
    }

    // Draw coins to the screen.
    for (uint8_t i = 0; i < coinsLength; i++) {
        // When coin hasn't been collected.
        if (!coins[i].collected) {
            // When coin is in the screen's view of the layout.
            if (coins[i].x - xOffset >= 0 && coins[i].x - xOffset < SCREEN_WIDTH) {
                screenSetLEDMatrix(coins[i].x - xOffset, coins[i].y, getColour('C'));
            }
        }
    }
    
    // Draw walkers to the screen.
    for (uint8_t i = 0; i < walkersLength; i++) {
        // When walker hasn't been jumped on.
        if (!walkers[i].dead) {
            // When walker is in the screen's view of the layout.
            if (walkers[i].x - xOffset >= 0 && walkers[i].x - xOffset < SCREEN_WIDTH) {
                screenSetLEDMatrix(walkers[i].x - xOffset, walkers[i].y, getColour('W'));
            }
        }
    }

    // Draw floaters to the screen.
    for (uint8_t i = 0; i < floatersLength; i++) {
        // When floater is in the screen's view of the layout.
        if (floaters[i].x - xOffset >= 0 && floaters[i].x - xOffset < SCREEN_WIDTH) {
            screenSetLEDMatrix(floaters[i].x - xOffset, floaters[i].y, getColour('F'));
        }
    }
    
    // Update the screen.
    screenShow();
}

// Reset the game variables to a given level.
void reset(uint8_t level) {
    if (level == 1) {
        layout = &level1;
        xLength = X_LENGTH_1;
        yLength = Y_LENGTH_1;
        player.x = 0;
        player.y = 6;
        player.dead = false;
        other.x = 1;
        other.y = 6;
        other.dead = false;
        coins = coins1;
        coinsLength = COINS_1_LENGTH;
        walkers = walkers1;
        walkersLength = WALKERS_1_LENGTH;
        floaters = floaters1;
        floatersLength = FLOATERS_1_LENGTH;
    } else if (level == 2) {
        layout = &level2;
        xLength = X_LENGTH_2;
        yLength = Y_LENGTH_2;
        player.x = 0;
        player.y = 0;
        player.dead = false;
        other.x = 1;
        other.y = 0;
        other.dead = false;
        coins = coins2;
        coinsLength = COINS_2_LENGTH;
        walkers = walkers2;
        walkersLength = WALKERS_2_LENGTH;
        floaters = floaters2;
        floatersLength = FLOATERS_2_LENGTH;
    } else if (level == 3) {
        layout = &level3;
        xLength = X_LENGTH_3;
        yLength = Y_LENGTH_3;
        player.x = 0;
        player.y = 5;
        player.dead = false;
        other.x = 1;
        other.y = 5;
        other.dead = false;
        coins = coins3;
        coinsLength = COINS_3_LENGTH;
        walkers = walkers3;
        walkersLength = WALKERS_3_LENGTH;
        floaters = floaters3;
        floatersLength = FLOATERS_3_LENGTH;
    } else if (level == 4) {
        layout = &level4;
        xLength = X_LENGTH_4;
        yLength = Y_LENGTH_4;
        player.x = 0;
        player.y = 5;
        player.dead = false;
        other.x = 1;
        other.y = 5;
        other.dead = false;
        coins = coins4;
        coinsLength = COINS_4_LENGTH;
        walkers = walkers4;
        walkersLength = WALKERS_4_LENGTH;
        floaters = floaters4;
        floatersLength = FLOATERS_4_LENGTH;
    } else if (level == 5) {
        layout = &level5;
        xLength = X_LENGTH_5;
        yLength = Y_LENGTH_5;
        player.x = 0;
        player.y = 5;
        player.dead = false;
        other.x = 1;
        other.y = 5;
        other.dead = false;
        coins = coins5;
        coinsLength = COINS_5_LENGTH;
        walkers = walkers5;
        walkersLength = WALKERS_5_LENGTH;
        floaters = floaters5;
        floatersLength = FLOATERS_5_LENGTH;
    } else if (level == 6) {
        layout = &level6;
        xLength = X_LENGTH_6;
        yLength = Y_LENGTH_6;
        player.x = 0;
        player.y = 5;
        player.dead = false;
        other.x = 1;
        other.y = 5;
        other.dead = false;
        coins = coins6;
        coinsLength = COINS_6_LENGTH;
        walkers = walkers6;
        walkersLength = WALKERS_6_LENGTH;
        floaters = floaters6;
        floatersLength = FLOATERS_6_LENGTH;
    } else if (level == 7) {
        layout = &level7;
        xLength = X_LENGTH_7;
        yLength = Y_LENGTH_7;
        player.x = 0;
        player.y = 5;
        player.dead = false;
        other.x = 1;
        other.y = 5;
        other.dead = false;
        coins = coins7;
        coinsLength = COINS_7_LENGTH;
        walkers = walkers7;
        walkersLength = WALKERS_7_LENGTH;
        floaters = floaters7;
        floatersLength = FLOATERS_7_LENGTH;
    } else if (level == 8) {
        layout = &level8;
        xLength = X_LENGTH_8;
        yLength = Y_LENGTH_8;
        player.x = 0;
        player.y = 5;
        player.dead = false;
        other.x = 1;
        other.y = 5;
        other.dead = false;
        coins = coins8;
        coinsLength = COINS_8_LENGTH;
        walkers = walkers8;
        walkersLength = WALKERS_8_LENGTH;
        floaters = floaters8;
        floatersLength = FLOATERS_8_LENGTH;
    }
}
