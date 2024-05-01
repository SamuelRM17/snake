#include "ripes_system.h"
#include <unistd.h> 

#define SW0 (0x01)
#define SW1 (0x02)
#define SW2 (0x04)
#define SW3 (0x08)

volatile unsigned int * led_base = LED_MATRIX_0_BASE;
volatile unsigned int * d_pad_up = D_PAD_0_UP;
volatile unsigned int * d_pad_do = D_PAD_0_DOWN;
volatile unsigned int * d_pad_le = D_PAD_0_LEFT;
volatile unsigned int * d_pad_ri = D_PAD_0_RIGHT;

volatile unsigned int * switch_base = SWITCHES_0_BASE;

int randomNumber = 0;

int generateRandomNumber() {
    randomNumber = (randomNumber * 1103515245 + 12345) & 0x7FFFFFFF;
    return randomNumber;
}

int isOccupied(int x, int y, int snakeLength, int (*snake)[2]) {
    for (int i = 0; i < snakeLength; i++) {
        if (snake[i][0] == x && snake[i][1] == y) {
            return 1; //the coordinates are occupied by the snake
        }
    }
    return 0; //the coordinates are not occupied by the snake
}

void printRectangle(int x, int y, int w, int h, int color) {
    led_base = LED_MATRIX_0_BASE;

    for (int i = 1; i < x; i++) {
        led_base += 1;
    }
    for (int i = 1; i < y; i++) {
        led_base += LED_MATRIX_0_WIDTH;
    }
    //A la derecha
    for (int i = 1; i < w; i++) {
        led_base = led_base + 1;
        (*led_base) = color;
    }
    //Abajo
    for (int i = 1; i < h; i++) {
        led_base += LED_MATRIX_0_WIDTH;
        (*led_base) = color;
    }
    //A la izquierda
    for (int i = 1; i < w; i++) {
        led_base -= 1;
        (*led_base) = color;
    }
    //Arriba
    for (int i = 1; i < h; i++) {
        led_base -= LED_MATRIX_0_WIDTH;
        (*led_base) = color;
    }
}
void printRectangleFull(int x, int y, int w, int n, int color[50]){
     
     
     for(int i = 0; i<x; i++){
          led_base += 1;
     }
     for(int i = 0; i<y; i++){
          led_base += LED_MATRIX_0_WIDTH;
     }
     volatile unsigned int * start = led_base;
     
     for(int i = 0; i<w; i++){
          led_base = start + i;
          volatile unsigned int * led_base_save = led_base;
          for(int j = 0; j<n; j++){
               led_base += LED_MATRIX_0_WIDTH;
               *led_base = color;
          }
          
     }
     }

void deleteTail(int x, int y) {
    printRectangle(x, y, 2, 2, 0x000000); //prints a black rectangle to delete
}

void updateSnake(int x, int y, int snakeLength, int (*snake)[2], int eating) {
    printRectangle(x, y, 2, 2, 0xFF0000);
    int newSnake[50][2];
    newSnake[0][0] = x;
    newSnake[0][1] = y;
    if (snakeLength > 1) {
        for (int i = 0; i < snakeLength; i++) {
            newSnake[i + 1][0] = snake[i][0];
            newSnake[i + 1][1] = snake[i][1];
        }
    }
    if (eating == 0) {
        deleteTail(snake[snakeLength - 1][0], snake[snakeLength - 1][1]);
    }
    else {
        newSnake[snakeLength][0] = snake[snakeLength - 1][0];
        newSnake[snakeLength][1] = snake[snakeLength - 1][1];
    }

    for (int i = 0; i <= snakeLength; i++) {
        snake[i][0] = newSnake[i][0];
        snake[i][1] = newSnake[i][1];
    }
}



void startGame() {
    unsigned int mask = 0;
    unsigned int x = 3;
    unsigned int y = 3;

    printRectangle(0, 0, 35, 25, 0x33FFF3); // Border of the game

    int snake[50][2]; // Stores the values where the snake is
    int snakeLength = 1; // Stores the length of the snake

    int state = 0; // Determines where the snake is programmed to turn

    int appleX = rand() % 15;
    int appleY = rand() % 15;
    appleX = appleX * 2 + 1;
    appleY = appleY * 2 + 1;

    appleX = 5;
    appleY = 5; // Hardcoded for testing eating
    printRectangle(appleX, appleY, 2, 2, 0x00FF00); // First apple

    int first = 0;
    int alive = 1;

    volatile unsigned int *original_led_base = led_base;
    printRectangle(x, y, 2, 2, 0xFF0000); // First rectangle
    led_base = original_led_base;

    while (1) {
        while (alive == 1) {
            unsigned int moved = 0;

            if (*d_pad_up == 1 && state != 2) {
                state = 1;
                moved = 1;
                y -= 2;
            } else if (*d_pad_do == 1 && state != 1) {
                state = 2;
                moved = 1;
                y += 2;
            } else if (*d_pad_le == 1 && state != 4) {
                state = 3;
                moved = 1;
                x -= 2;
            } else if (*d_pad_ri == 1 && state != 3) {
                state = 4;
                moved = 1;
                x += 2;
            } else {
                // Continue moving in the current direction if no keypad input
                if (state == 1) {
                    moved = 1;
                    y -= 2;
                } else if (state == 2) {
                    moved = 1;
                    y += 2;
                } else if (state == 3) {
                    moved = 1;
                    x -= 2;
                } else if (state == 4) {
                    moved = 1;
                    x += 2;
                }
            }

            if (moved) {
                // Check for borders
                if (x <= 1 || x >= 34 || y <= 1 || y >= 24) {
                    alive = 0;
                    printRectangle(0, 0, 35, 25, 0x33FFF3);
                }
                if (appleX == x && appleY == y) {
                    updateSnake(x, y, snakeLength, snake, 1);
                    snakeLength++;
                    // Occupied
                    appleX = (generateRandomNumber() % (15 - 3)) * 2 + 1;
                    appleY = (generateRandomNumber() % (15 - 3)) * 2 + 1;
                    printRectangle(appleX, appleY, 2, 2, 0x00FF00); // Print apple
                } else {
                    // Sleep
                    for (int i = 0; i < 10000; i++) {
                    }
                    updateSnake(x, y, snakeLength, snake, 0);
                    printRectangle(0, 0, 35, 25, 0x33FFF3);
                }
            }
        }
        if (alive == 0){
            break;
        }
    }
}



void main() {
    while(1) {
        
        if (*switch_base & SW0) {
            startGame();
        }
    }
}