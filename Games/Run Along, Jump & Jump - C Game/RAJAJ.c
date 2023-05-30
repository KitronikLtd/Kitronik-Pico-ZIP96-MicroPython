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

#include "pico/stdlib.h"

#include "RAJAJ.h"
#include "ZIP96Pico.h"

/*
Player functions for player movement functionality.
*/
Player playerInit(uint8_t x, uint8_t y) {
    Player player = {x, y, 0, 0, false, false};
    return player;
}

// Moves Player position up one and reduces jump.
void playerUp(Player *player, char (*layout)[8][50]) {
    // Top of layout, can't move.
    if (player->y == 0) {
        --player->jump;
        return;
    }

    // Move up when jump is even.
    if (player->jump % 2 == 0) {
        // Move up when Sky is above.
        if ((*layout)[player->y - 1][player->x] == 'S') {
            --player->y;
            vibrationVibrate();
            sleep_ms(50);
            vibrationStop();
        }
    }

    // Reduce jump left.
    --player->jump;
}

// Moves Player position down one and reduces jump variable.
void playerDown(Player *player, char (*layout)[8][50], uint8_t yLength, Walker *walkers, uint8_t walkersLength) {
    // Bottom of layout, can't move.
    if (player->y == yLength - 1) {
        return;
    }

    // Loop through each walker in walkers.
    for (uint8_t i = 0; i < walkersLength; i++) {
        // Squash walker when falling on top of it.
        if (playerCollision(player, walkers[i].x, walkers[i].y - 1)) {
            // Only if it's not already dead.
            if (!walkers[i].dead) {
                walkers[i].dead = true;
                // Don't want to move down anymore.
                player->jump = 2;
                return;
            }
        }
    }
    
    // Move down when Sky is below.
    if ((*layout)[player->y + 1][player->x] == 'S') {
        // Move down when falling is set (allows player to float).
        if (player->falling) {
            ++player->y;
        }

        // Not finished falling.
        player->falling = true;
        return;
    }
    
    // Vibrate when just finished falling.
    if (player->falling) {
        vibrationVibrate();
        sleep_ms(50);
        vibrationStop();
    }
    
    // Finished falling.
    player->jump = -1;
    player->falling = false;
}

// Moves Player position left one.
void playerLeft(Player *player, char (*layout)[8][50]) {
    // Start of layout, can't move.
    if (player->x == 0) {
        return;
    }

    // Move left when Sky is before.
    if ((*layout)[player->y][player->x - 1] == 'S') {
        --player->x;
    }
}

// Moves Player position right one.
void playerRight(Player *player, char (*layout)[8][50], uint8_t xLength) {
    // End of layout, can't move.
    if (player->x == xLength - 1) {
        return;
    }
    
    // Move right when Sky or House is after
    if ((*layout)[player->y][player->x + 1] == 'S' || (*layout)[player->y][player->x + 1] == 'H') {
        ++player->x;
    }
}

// Update how many coins have been collected.
void playerCheckCoins(Player *player, Coin *coins, uint8_t coinsLength) {
    for (uint8_t i = 0; i < coinsLength; i++) {
        if (playerCollision(player, coins[i].x, coins[i].y)) {
            if (!coins[i].collected) {
                coins[i].collected = true;
                player->coins++;
            }
        }
    }
}

// Check if a given position is equal to the Player's.
bool playerCollision(Player *player, uint8_t x, uint8_t y) {
    return player->x == x && player->y == y;
}

/*
Setup the Coin structs for each level.
*/
Coin coins1[7] = {{0, 0, false}, {0, 1, false}, {0, 2, false}, {0, 3, false}, {9, 2, false}, {10, 2, false}, {11, 1, false}};
Coin coins2[6] = {{0, 3, false}, {3, 5, false}, {4, 4, false}, {15, 6, false}, {18, 2, false}, {18, 3, false}};
Coin coins3[6] = {{5, 5, false}, {5, 6, false}, {13, 3, false}, {14, 3, false}, {22, 0, false}, {29, 3, false}};
Coin coins4[8] = {{7, 2, false}, {7, 3, false}, {7, 4, false}, {14, 3, false}, {14, 4, false}, {16, 2, false}, {16, 3, false}, {16, 4, false}};
Coin coins5[9] = {{6, 4, false}, {7, 4, false}, {8, 4, false}, {9, 4, false}, {6, 1, false}, {8, 1, false}, {10, 1, false}, {12, 1, false}, {14, 1, false}};
Coin coins6[7] = {{8, 4, false}, {9, 4, false}, {10, 4, false}, {13, 0, false}, {14, 0, false}, {17, 4, false}, {18, 4, false}};
Coin coins7[8] = {{7, 4, false}, {8, 4, false}, {9, 4, false}, {10, 4, false}, {8, 0, false}, {9, 0, false}, {13, 0, false}, {14, 0, false}};
Coin coins8[12] = {{5, 3, false}, {6, 3, false}, {8, 0, false}, {11, 3, false}, {12, 3, false}, {17, 0, false}, {22, 0, false}, {23, 0, false}, {24, 0, false}, {25, 0, false}, {23, 3, false}, {24, 3, false}};

/*
Setup the Walker structs for each level.
Walker functions for walker enemy movement functionality.
*/
Walker walkers1[2] = {{3, 6, 1, 1, 0, false}, {11, 5, 1, 3, 0, false}};
Walker walkers2[0] = {};
Walker walkers3[2] = {{11, 6, 1, 2, 0, false}, {20, 6, 1, 3, 0, false}};
Walker walkers4[3] = {{7, 5, 1, 2, 0, false}, {11, 5, 1, 2, 0, false}, {16, 5, 1, 2, 0, false}};
Walker walkers5[3] = {{6, 5, 1, 2, 0, false}, {8, 5, 1, 2, 0, false}, {10, 5, 1, 2, 0, false}};
Walker walkers6[4] = {{6, 5, 1, 2, 0, false}, {10, 5, 1, 2, 0, false}, {17, 5, 1, 2, 0, false}, {25, 5, 1, 2, 0, false}};
Walker walkers7[5] = {{6, 5, 1, 1, 0, false}, {14, 5, 1, 1, 0, false}, {22, 5, 1, 1, 0, false}, {10, 2, 1, 2, 0, false}, {17, 2, 1, 2, 0, false}};
Walker walkers8[4] = {{6, 5, 1, 1, 0, false}, {8, 5, 1, 1, 0, false}, {17, 5, 1, 1, 0, false}, {27, 5, 1, 1, 0, false}};

// Move Walker left and right along the ground.
void walkerMove(Walker *walker, char (*layout)[8][50], uint8_t xLength) {
    // Wait for the delay to reach zero before moving.
    if (walker->moveDelay > 0) {
        walker->moveDelay -= 1;
        return;
    }
    
    // Reset delay.
    walker->moveDelay = walker->delay;
    // Calculate new position with current move direction.
    int8_t newX = walker->x + walker->moveX;
    
    if (newX < 0) {
        // Start of layout.
        walker->x = 0;
    } else if (newX > xLength - 1) {
        // End of layout.
        walker->x = xLength - 1;
    } else if ((*layout)[walker->y][newX] == 'S') {
        // Move in current direction when Sky is after.
        walker->x += walker->moveX;
        return;
    }

    // Invert move direction.
    walker->moveX = -walker->moveX;
}

/*
Setup the Floater structs for each level.
Floater functions for floater enemy movement functionality.
*/
Floater floaters1[0] = {};
Floater floaters2[5] = {{5, 6, 1, 1, 0}, {9, 5, 1, 2, 0}, {11, 5, 1, 3, 0}, {14, 6, 1, 3, 0}, {17, 6, 1, 3, 0}};
Floater floaters3[2] = {{8, 6, 1, 1, 0}, {23, 6, 1, 4, 0}};
Floater floaters4[4] = {{21, 3, 1, 1, 0}, {22, 2, 1, 1, 0}, {23, 1, 1, 1, 0}, {24, 0, 1, 1, 0}};
Floater floaters5[4] = {{7, 0, 1, 1, 0}, {9, 0, 1, 1, 0}, {11, 0, 1, 1, 0}, {13, 0, 1, 1, 0}};
Floater floaters6[3] = {{9, 0, 1, 1, 0}, {21, 0, 1, 1, 0}, {22, 5, 1, 1, 0}};
Floater floaters7[2] = {{6, 0, 1, 1, 0}, {16, 0, 1, 1, 0}};
Floater floaters8[5] = {{3, 0, 1, 1, 0}, {8, 0, 1, 1, 0}, {17, 2, 1, 1, 0}, {20, 0, 1, 1, 0}, {27, 0, 1, 1, 0}};

// Move Walker up and down.
void floaterMove(Floater *floater, char (*layout)[8][50], uint8_t yLength) {
    // Wait for the delay to reach zero before moving.
    if (floater->moveDelay > 0) {
        floater->moveDelay -= 1;
        return;
    }
    
    // Reset delay.
    floater->moveDelay = floater->delay;
    // Calculate new position with current move direction
    int8_t newY = floater->y + floater->moveY;

    if (newY < 0) {
        // Top of layout.
        floater->y = 0;
    } else if (newY > yLength - 1) {
        // Bottom of layout.
        floater->y = yLength - 1;
    } else if ((*layout)[newY][floater->x] == 'S') {
        // Move in current direction when Sky is after.
        floater->y += floater->moveY;
        return;
    }

    // Invert move direction.
    floater->moveY = -floater->moveY;
}

/*
Setup the level layout arrays for each level.
*/
char level1[8][50] = {
    {'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'R', 'R', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S'},
    {'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'B', 'B', 'S', 'S', 'S', 'R', 'R', 'R', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S'},
    {'S', 'B', 'B', 'B', 'B', 'B', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'R', 'R', 'R', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S'},
    {'S', 'S', 'S', 'S', 'S', 'S', 'B', 'B', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'H', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S'},
    {'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'H', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S'},
    {'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'G', 'G', 'S', 'S', 'S', 'S', 'S', 'H', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S'},
    {'S', 'S', 'G', 'S', 'S', 'S', 'G', 'G', 'G', 'D', 'D', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G'},
    {'G', 'G', 'D', 'G', 'G', 'G', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D'}};
char level2[8][50] = {
    {'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'B', 'B', 'B', 'B', 'B', 'S', 'S', 'S', 'S', 'S', 'B', 'B', 'S', 'S', 'S', 'R', 'R', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S'},
    {'B', 'B', 'B', 'B', 'S', 'S', 'S', 'S', 'B', 'S', 'B', 'S', 'B', 'S', 'S', 'B', 'B', 'S', 'B', 'B', 'S', 'S', 'R', 'R', 'R', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S'},
    {'S', 'S', 'S', 'B', 'B', 'B', 'B', 'S', 'B', 'S', 'B', 'S', 'B', 'S', 'S', 'S', 'B', 'S', 'S', 'B', 'S', 'S', 'R', 'R', 'R', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S'},
    {'S', 'S', 'S', 'B', 'B', 'S', 'B', 'S', 'B', 'S', 'S', 'S', 'B', 'B', 'S', 'S', 'B', 'S', 'S', 'B', 'S', 'S', 'S', 'S', 'H', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S'},
    {'S', 'S', 'S', 'B', 'S', 'S', 'B', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'B', 'S', 'B', 'B', 'S', 'S', 'S', 'S', 'H', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S'},
    {'S', 'S', 'B', 'S', 'S', 'S', 'B', 'S', 'S', 'S', 'G', 'S', 'G', 'S', 'S', 'B', 'B', 'S', 'B', 'B', 'S', 'S', 'S', 'S', 'H', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S'},
    {'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'G', 'G', 'D', 'G', 'D', 'S', 'S', 'S', 'B', 'S', 'S', 'S', 'S', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G'},
    {'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'D', 'D', 'D', 'D', 'D', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D'}};
char level3[8][50] = {
    {'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'R', 'R', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S'},
    {'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'G', 'G', 'G', 'G', 'S', 'S', 'S', 'S', 'S', 'R', 'R', 'R', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S'},
    {'S', 'S', 'S', 'G', 'G', 'G', 'G', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'B', 'S', 'S', 'S', 'G', 'D', 'D', 'D', 'D', 'G', 'S', 'S', 'S', 'S', 'R', 'R', 'R', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S'},
    {'S', 'S', 'S', 'D', 'D', 'D', 'D', 'G', 'S', 'S', 'S', 'S', 'B', 'S', 'S', 'B', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'D', 'D', 'S', 'S', 'S', 'S', 'S', 'S', 'H', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S'},
    {'S', 'S', 'G', 'D', 'D', 'D', 'D', 'D', 'S', 'S', 'S', 'S', 'S', 'B', 'B', 'S', 'S', 'S', 'G', 'S', 'S', 'S', 'S', 'S', 'B', 'B', 'B', 'B', 'S', 'S', 'D', 'D', 'G', 'S', 'S', 'S', 'S', 'S', 'H', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S'},
    {'S', 'S', 'D', 'D', 'D', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'D', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'D', 'D', 'D', 'S', 'S', 'S', 'S', 'S', 'H', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S'},
    {'G', 'G', 'D', 'D', 'D', 'S', 'S', 'S', 'S', 'G', 'G', 'S', 'S', 'S', 'S', 'S', 'S', 'G', 'D', 'G', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'G', 'D', 'D', 'D', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G'},
    {'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'G', 'G', 'G', 'G', 'G', 'G', 'D', 'D', 'D', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D'}};
char level4[8][50] = {
    {'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'R', 'R', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S'},
    {'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'G', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'R', 'R', 'R', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S'},
    {'S', 'S', 'S', 'S', 'S', 'B', 'B', 'S', 'G', 'S', 'S', 'S', 'S', 'S', 'S', 'B', 'S', 'D', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'R', 'R', 'R', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S'},
    {'S', 'S', 'S', 'S', 'B', 'B', 'B', 'S', 'D', 'G', 'S', 'S', 'S', 'B', 'S', 'B', 'S', 'D', 'G', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'H', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S'},
    {'S', 'S', 'S', 'S', 'B', 'B', 'B', 'S', 'S', 'D', 'G', 'S', 'S', 'B', 'S', 'B', 'S', 'D', 'D', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'H', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S'},
    {'S', 'S', 'G', 'S', 'S', 'S', 'S', 'S', 'G', 'D', 'D', 'S', 'S', 'S', 'S', 'S', 'S', 'D', 'D', 'S', 'S', 'G', 'G', 'G', 'G', 'S', 'S', 'S', 'H', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S'},
    {'G', 'G', 'D', 'G', 'G', 'G', 'G', 'G', 'D', 'D', 'D', 'G', 'G', 'G', 'G', 'G', 'G', 'D', 'D', 'G', 'G', 'D', 'D', 'D', 'D', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G'},
    {'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D'}};
char level5[8][50] = {
    {'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'R', 'R', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S'},
    {'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'R', 'R', 'R', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S'},
    {'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'R', 'R', 'R', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S'},
    {'S', 'S', 'S', 'S', 'S', 'G', 'B', 'B', 'B', 'B', 'B', 'B', 'B', 'B', 'B', 'B', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'H', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S'},
    {'S', 'S', 'S', 'S', 'G', 'D', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'G', 'S', 'S', 'S', 'S', 'S', 'H', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S'},
    {'S', 'S', 'S', 'G', 'D', 'D', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'D', 'G', 'S', 'S', 'S', 'S', 'H', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S'},
    {'G', 'G', 'G', 'D', 'D', 'D', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'D', 'D', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G'},
    {'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D'}};
char level6[8][50] = {
    {'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'R', 'R', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S'},
    {'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'B', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'R', 'R', 'R', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S'},
    {'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'B', 'B', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'R', 'R', 'R', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S'},
    {'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'B', 'B', 'B', 'S', 'S', 'S', 'S', 'S', 'G', 'B', 'B', 'B', 'B', 'S', 'S', 'S', 'S', 'S', 'H', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S'},
    {'S', 'S', 'S', 'S', 'G', 'G', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'G', 'D', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'H', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S'},
    {'S', 'S', 'S', 'G', 'D', 'D', 'S', 'S', 'S', 'G', 'S', 'S', 'S', 'S', 'G', 'D', 'D', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'H', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S'},
    {'G', 'G', 'G', 'D', 'D', 'D', 'G', 'G', 'G', 'D', 'G', 'G', 'G', 'G', 'D', 'D', 'D', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G'},
    {'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D'}};
char level7[8][50] = {
    {'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'B', 'S', 'S', 'S', 'S', 'S', 'B', 'S', 'S', 'S', 'R', 'R', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S'},
    {'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'B', 'B', 'B', 'B', 'S', 'S', 'B', 'S', 'S', 'R', 'R', 'R', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S'},
    {'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'B', 'B', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'B', 'S', 'S', 'R', 'R', 'R', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S'},
    {'S', 'S', 'S', 'S', 'S', 'B', 'B', 'B', 'B', 'B', 'B', 'B', 'B', 'B', 'B', 'B', 'B', 'B', 'B', 'S', 'S', 'S', 'S', 'H', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S'},
    {'S', 'S', 'S', 'S', 'B', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'H', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S'},
    {'S', 'S', 'G', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'H', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S'},
    {'G', 'G', 'D', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G'},
    {'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D'}};
char level8[8][50] = {
    {'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'R', 'R', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S'},
    {'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'B', 'B', 'B', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'R', 'R', 'R', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S'},
    {'S', 'S', 'S', 'S', 'S', 'B', 'B', 'S', 'S', 'S', 'B', 'B', 'B', 'B', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'B', 'B', 'B', 'B', 'S', 'S', 'R', 'R', 'R', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S'},
    {'S', 'S', 'S', 'S', 'B', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'H', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S'},
    {'S', 'S', 'S', 'B', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'H', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S'},
    {'S', 'S', 'S', 'S', 'S', 'G', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'H', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S', 'S'},
    {'G', 'G', 'G', 'G', 'G', 'D', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G'},
    {'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D'}};
