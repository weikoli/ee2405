#include "mbed.h"
#include "uLCD_4DGL.h"

uLCD_4DGL uLCD(D1, D0, D2); // serial tx, serial rx, reset pin;

int main()
{
    // basic printf demo = 16 by 18 characters on screen
    uLCD.printf("\n106000107\n"); //Default Green on black text
    
    // uLCD.text_width(4); //4X size text
    // uLCD.text_height(4);
    uLCD.color(RED);

    uLCD.line(0, 0, 60, 0, 0xFF0000);
    uLCD.line(0, 0, 0, 60, 0xFF0000);
    uLCD.line(60, 60, 0, 60, 0xFF0000);
    // uLCD.line(0, 60, 60, 60, 0xFF0000);
}