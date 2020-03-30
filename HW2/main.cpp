#include "mbed.h"

BusOut display(D6, D7, D9, D10, D11, D5, D4, D8);
char table[4] = {0x06, 0x3F, 0xBF,0x00};

DigitalIn  Switch(SW3);
DigitalOut redLED(LED1);
DigitalOut greenLED(LED2);

Serial pc( USBTX, USBRX );
AnalogOut Aout(DAC0_OUT);
AnalogIn Ain(A0);
float ADCdata[1000];

int sample = 1000;
int i;

int main(){
  for (i = 0; i < sample; i++){
    Aout = Ain;
    ADCdata[i] = Ain;
    wait(1./sample);
  }
  for (i = 0; i < sample; i++){
    pc.printf("%1.3f\r\n", ADCdata[i]);
    wait(0.1);
  }
  while(1){
    for(float j=0; j<2; j+=0.05 ){
      Aout = 0.5 + 0.5*sin(j*3.14159);
      wait(0.0001);
    }
    if(Switch){
        greenLED = 0;
        redLED = 1;
    }
    else{
      redLED = 0;
      greenLED = 1;
      for (int i = 0; i<4; i = i+1){
            display = table[i];
            if (i!=3){
              wait(1);  
            }
      }
      
    }
  }
  
  
}
