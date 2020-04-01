#include "mbed.h"

Serial pc( USBTX, USBRX );
AnalogOut Aout(DAC0_OUT);
AnalogIn Ain(A0);
DigitalIn  Switch(SW3);
DigitalOut redLED(LED1);
DigitalOut greenLED(LED2);

BusOut display(D6, D7, D9, D10, D11, D5, D4, D8);

char table[11] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F,0xBF};

int sample = 1001;
int i;

float ADCdata[1001];

int main(){
  int count = 0;
 redLED = 0;
  greenLED = 1;
  for (i = 1; i < sample; i++){
    Aout = Ain;
    ADCdata[i] = Ain;
    wait(1./sample);
    // wait(0.02);
    if(ADCdata[i-1] <= 0.5){
        if(ADCdata[i] > 0.5){
            count++ ;
        }
    }
  }

  for (i = 0; i < sample; i++){
    pc.printf("%1.3f\r\n", ADCdata[i]);
    wait(0.1);
    // pc.printf("%d\r\n",count);
    // wait(0.3);
  }
  
  count = count - 6 ;
  
    float j;

  while(1){
    if( Switch == 0 ){
        redLED = 0;
        greenLED = 1;
        display = table[(count / 100) % 10];
        wait(1.0);
        display = table[(count / 10) % 10];
        wait(1.0);
        display = table[count / 10];
        wait(1.0);
    }
    if( Switch == 1 ){
        display  = 0x00;
        redLED = 1;
        greenLED = 0;
        for( j=0; j<2; j+=0.05 ){
            Aout = 0.5 + 0.5*sin(j*3.14159);
            wait(1./count/40);
        }
    }
  }
}


// #include "mbed.h"

// BusOut display(D6, D7, D9, D10, D11, D5, D4, D8);
// char table[4] = {0x06, 0x3F, 0xBF,0x00};

// DigitalIn  Switch(SW3);
// DigitalOut redLED(LED1);
// DigitalOut greenLED(LED2);

// Serial pc( USBTX, USBRX );
// AnalogOut Aout(DAC0_OUT);
// AnalogIn Ain(A0);
// float ADCdata[1000];

// int sample = 1000;
// int i;

// int main(){
//   for (i = 0; i < sample; i++){
//     Aout = Ain;
//     ADCdata[i] = Ain;
//     wait(1./sample);
//   }
//   for (i = 0; i < sample; i++){
//     pc.printf("%1.3f\r\n", ADCdata[i]);
//     wait(0.1);
//   }
//   while(1){
//     for(float j=0; j<2; j+=0.05 ){
//       Aout = 0.5 + 0.5*sin(j*3.14159);
//       wait(0.0001);
//     }
//     if(Switch){
//         greenLED = 0;
//         redLED = 1;
//     }
//     else{
//       redLED = 0;
//       greenLED = 1;
//       for (int i = 0; i<4; i = i+1){
//             display = table[i];
//             if (i!=3){
//               wait(1);  
//             }
//       }
      
//     }
//   }
  
  
// }