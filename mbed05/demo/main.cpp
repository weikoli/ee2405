#include "mbed.h"

Ticker time-up;
DigitalOut led3(LED3);

void blink(){
    led3 = !led3;
}

int main(){
    time_up.attach(&blink,0.5);
    while(1);
}