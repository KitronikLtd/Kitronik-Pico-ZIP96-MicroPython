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

#ifndef RAJAJ_H_
#define RAJAJ_H_

typedef struct Player {
    uint8_t x, y, coins;
    int8_t jump;
    bool falling, dead;
} Player;

typedef struct Coin {
    uint8_t x, y;
    bool collected;
} Coin;

typedef struct Walker {
    uint8_t x, y, moveX, delay, moveDelay;
    bool dead;
} Walker;

typedef struct Floater {
    uint8_t x, y, moveY, delay, moveDelay;
} Floater;

typedef struct Config {
    uint8_t x, y, coins;
    uint8_t collected[12], dead[5];
} Config;

#define GAME_STOP 128
#define QUEUE_SIZE 1

Player playerInit(uint8_t x, uint8_t y);
void playerUp(Player *player, char (*layout)[8][50]);
void playerDown(Player *player, char (*layout)[8][50], uint8_t yLength, Walker *walkers, uint8_t walkersLength);
void playerLeft(Player *player, char (*layout)[8][50]);
void playerRight(Player *player, char (*layout)[8][50], uint8_t xLength);
void playerCheckCoins(Player *player, Coin *coins, uint8_t coinsLength);
bool playerCollision(Player *player, uint8_t x, uint8_t y);

#define COINS_1_LENGTH 7
#define COINS_2_LENGTH 6
#define COINS_3_LENGTH 6
#define COINS_4_LENGTH 8
#define COINS_5_LENGTH 9
#define COINS_6_LENGTH 7
#define COINS_7_LENGTH 8
#define COINS_8_LENGTH 12

extern Coin coins1[7];
extern Coin coins2[6];
extern Coin coins3[6];
extern Coin coins4[8];
extern Coin coins5[9];
extern Coin coins6[7];
extern Coin coins7[8];
extern Coin coins8[12];

#define WALKERS_1_LENGTH 2
#define WALKERS_2_LENGTH 0
#define WALKERS_3_LENGTH 2
#define WALKERS_4_LENGTH 3
#define WALKERS_5_LENGTH 3
#define WALKERS_6_LENGTH 4
#define WALKERS_7_LENGTH 5
#define WALKERS_8_LENGTH 4

extern Walker walkers1[2];
extern Walker walkers2[0];
extern Walker walkers3[2];
extern Walker walkers4[3];
extern Walker walkers5[3];
extern Walker walkers6[4];
extern Walker walkers7[5];
extern Walker walkers8[4];

void walkerMove(Walker *walker, char (*layout)[8][50], uint8_t xLength);

#define FLOATERS_1_LENGTH 0
#define FLOATERS_2_LENGTH 5
#define FLOATERS_3_LENGTH 2
#define FLOATERS_4_LENGTH 4
#define FLOATERS_5_LENGTH 4
#define FLOATERS_6_LENGTH 3
#define FLOATERS_7_LENGTH 2
#define FLOATERS_8_LENGTH 5

extern Floater floaters1[0];
extern Floater floaters2[5];
extern Floater floaters3[2];
extern Floater floaters4[4];
extern Floater floaters5[4];
extern Floater floaters6[3];
extern Floater floaters7[2];
extern Floater floaters8[5];

void floaterMove(Floater *floater, char (*layout)[8][50], uint8_t yLength);

#define X_LENGTH_1 17
#define Y_LENGTH_1 8
#define X_LENGTH_2 25
#define Y_LENGTH_2 8
#define X_LENGTH_3 39
#define Y_LENGTH_3 8
#define X_LENGTH_4 29
#define Y_LENGTH_4 8
#define X_LENGTH_5 25
#define Y_LENGTH_5 8
#define X_LENGTH_6 27
#define Y_LENGTH_6 8
#define X_LENGTH_7 24
#define Y_LENGTH_7 8
#define X_LENGTH_8 31
#define Y_LENGTH_8 8

extern char level1[8][50];
extern char level2[8][50];
extern char level3[8][50];
extern char level4[8][50];
extern char level5[8][50];
extern char level6[8][50];
extern char level7[8][50];
extern char level8[8][50];

#endif // RAJAJ_H_
