#include "mbed.h"
#include "bbcar.h"

Ticker servo_ticker;
Ticker encoder_ticker;
DigitalOut red_led(LED1);
DigitalOut blue_led(LED2);
PwmOut pin8(D8), pin9(D9);
Serial pc(USBTX, USBRX);
DigitalInOut ping(D10);
DigitalInOut ping2(D7);
DigitalIn pin3(D3);
Serial xbee(D12, D11);
Serial uart(D1, D0);
Timer t;
Timer t2;
Timer object;

int a = 0;
float val;
float now;
float val2;
float now2 = 0;

EventQueue queue(32 * EVENTS_EVENT_SIZE);
Thread thread;

void xbee_rx_interrupt(void);
void xbee_rx(void);
void reply_messange(char *xbee_reply, char *messange);
void check_addr(char *xbee_reply, char *messenger);

BBCar car(pin8, pin9, servo_ticker);


void GetXBee()
{
    char c[3];
    int t = 0;
    
    // output count
    if (a == 0){
        t = sprintf(c, "%02d", 00);
        xbee.printf("%s", c);
    }else if(a == 1){
        t = sprintf(c, "%02d", 11);
        xbee.printf("%s", c);
    }else if(a == 2){
        t = sprintf(c, "%02d", 22);
        xbee.printf("%s", c);
    }else if (a == 3){
        t = sprintf(c, "%02d", 33);
        xbee.printf("%s", c);
    }else{
        t = sprintf(c, "%02.1f", now2);
        xbee.printf("%s", c);
    }
    
    // pc.printf("%d\r\n",ACCcount);   
}
void reply_messange(char *xbee_reply, char *messange){
  xbee_reply[0] = xbee.getc();
  xbee_reply[1] = xbee.getc();
  xbee_reply[2] = xbee.getc();
  if(xbee_reply[1] == 'O' && xbee_reply[2] == 'K'){
    pc.printf("%s\r\n", messange);
    xbee_reply[0] = '\0';
    xbee_reply[1] = '\0';
    xbee_reply[2] = '\0';
  }
}

void check_addr(char *xbee_reply, char *messenger){
  xbee_reply[0] = xbee.getc();
  xbee_reply[1] = xbee.getc();
  xbee_reply[2] = xbee.getc();
  xbee_reply[3] = xbee.getc();
  pc.printf("%s = %c%c%c\r\n", messenger, xbee_reply[1], xbee_reply[2], xbee_reply[3]);
  xbee_reply[0] = '\0';
  xbee_reply[1] = '\0';
  xbee_reply[2] = '\0';
  xbee_reply[3] = '\0';
}

void xbee_rx(void)
{
   static int i = 0;
  static char buf[100] = {0};
  while(xbee.readable()){
    char c = xbee.getc();
    if(c!='\r' && c!='\n'){
      buf[i] = c;
      i++;
      buf[i] = '\0';
    }else{
      i = 0;
      pc.printf("Get: %s\r\n", buf);
      xbee.printf("%s", buf);
    }
  }
  wait(0.1);
  xbee.attach(xbee_rx_interrupt, Serial::RxIrq); // reattach interrupt
}

void xbee_rx_interrupt(void)
{
  xbee.attach(NULL, Serial::RxIrq); // detach interrupt
  queue.call(&xbee_rx);
}

void ClassifyImage()
{
    uart.baud(9600);
    char s[21];
    sprintf(s,"image_classification");
    uart.puts(s);
    pc.printf("QQ\r\n");
}

int main() 
{
    red_led = 0;
    blue_led = 1;
    parallax_encoder encoder0(pin3, encoder_ticker);
    encoder0.reset();
    pc.baud(9600);

    char xbee_reply[4];

    // XBee setting
    xbee.baud(9600);
    xbee.printf("+++");
    xbee_reply[0] = xbee.getc();
    xbee_reply[1] = xbee.getc();
    if(xbee_reply[0] == 'O' && xbee_reply[1] == 'K'){
        pc.printf("enter AT mode.\r\n");
        xbee_reply[0] = '\0';
        xbee_reply[1] = '\0';
    }
    xbee.printf("ATMY 258\r\n");
    reply_messange(xbee_reply, "setting MY : 258");

    xbee.printf("ATDL 158\r\n");
    reply_messange(xbee_reply, "setting DL : 158");

    xbee.printf("ATID 1\r\n");
    reply_messange(xbee_reply, "setting PAN ID : 1");

    xbee.printf("ATWR\r\n");
    reply_messange(xbee_reply, "write config");

    xbee.printf("ATMY\r\n");
    check_addr(xbee_reply, "MY");

    xbee.printf("ATDL\r\n");
    check_addr(xbee_reply, "DL");

    xbee.printf("ATCN\r\n");
    reply_messange(xbee_reply, "exit AT mode");
    xbee.getc();

    pc.printf("start\r\n");
    thread.start(callback(&queue, &EventQueue::dispatch_forever));
    queue.call_every(1000, GetXBee);


 
    //go straight
    car.goStraight(100);
    wait_ms(8700);
    car.stop();
    //turn left
    a = 1;
    red_led = 1;
    blue_led = 0;
    car.turn(37,0.1);
    wait_ms(3050);
    car.stop();
    // go straight
    a = 0;
    blue_led = 1;
    red_led = 0;
    car.goStraight(100);
    wait_ms(6440);
    car.stop();
    //turn right
    red_led = 1;
    blue_led = 0;
    a = 2;
    car.turn(-31.5,0.1);
    wait_ms(2460);
    car.stop();

    // go backward
    blue_led = 1;
    red_led = 0;
    a = 3;
    car.goStraight(-100);
    wait_ms(2580);
    car.stop();

    //go straight
    red_led = 1;
    blue_led = 0;
    a = 0;
    car.goStraight(100);
    wait_ms(2050);
    car.stop();

    //turn right
    // blue_led = 1;
    // red_led = 0;
    // a = 2;
    // car.turn(-31.5,0.1);
    // wait_ms(850);
    // car.stop();

    // //go straight
    // red_led = 1;
    // blue_led = 0;
    // a = 0;
    // car.goStraight(100);
    // wait_ms(1050);
    // car.stop();

    //turn right
    blue_led = 1;
    red_led = 0;
    a = 2;
    car.turn(-31.5,0.1);
    wait_ms(2450);
    car.stop();

    // //turn right
    // a = 2;
    // car.turn(-31.5,0.1);
    // wait_ms(1420);
    // car.stop();

    //go straight
    red_led = 1;
    blue_led = 0;
    a = 0;
    car.goStraight(100);
    wait_ms(3050);
    car.stop();

    

    //turn left
    blue_led = 1;
    red_led = 0;
    a = 1;
    car.turn(37,0.1);
    wait_ms(2850);
    car.stop();
    
    //go backward
    a = 0;
    car.goStraight(-80);
    wait_ms(1350);
    car.stop();

    // image
    red_led = 0;
    blue_led = 1;
    ClassifyImage();
    wait_ms(3000);

     //go straight
    red_led = 1;
    blue_led = 0;
    a = 0;
    car.goStraight(100);
    wait_ms(2000);
    car.stop();

    
    
    //go Straight
    // red_led = 1;
    // blue_led = 0;
    // a = 0;
    // car.goStraight(100);
    // wait_ms(970);
    // car.stop();

    //turn right
    blue_led = 1;
    red_led = 0;
    a = 2;
    car.turn(-31.5,0.1);
    wait_ms(2500);
    car.stop();

     //go straight
    red_led = 1;
    blue_led = 0;
    a = 0;
    car.goStraight(100);
    wait_ms(2100);
    car.stop();

    //turn right
    blue_led = 1;
    red_led = 0;
    a = 2;
    car.turn(-31.5,0.1);
    wait_ms(2490);
    car.stop();

    //go straight
    red_led = 1;
    blue_led = 0;
    a = 0;
    car.goStraight(100);
    wait_ms(8100);
    car.stop();

    //turn right
    blue_led = 1;
    red_led = 0;
    a = 2;
    car.turn(-31.5,0.1);
    wait_ms(2380);
    car.stop();

    //go straight
    red_led = 1;
    blue_led = 0;
    a = 0;
    car.goStraight(100);
    wait_ms(3900);
    car.stop();

    //turn right
    blue_led = 1;
    red_led = 0;
    a = 2;
    car.turn(-31.5,0.1);
    wait_ms(2405);
    car.stop();

    //go straight
    a = 0;
    car.goStraight(100);
    wait_ms(300);
    car.stop();
    

    
    
    

    //ping object
    a = 4;
    red_led = 1;
    blue_led = 0;
    
    // pc.baud(9600);
    car.stop();
    // encoder0.reset();
    
    car.goStraight(50);
    // wait_ms(100);
    object.start();

    while(object<3.01) //wait_ms(50);
    {
        wait_ms(50);
        red_led = 1;
        blue_led = 0;
        car.goStraight(50);
        ping2.output();
        ping2 = 0; wait_us(200);
        ping2 = 1; wait_us(5);
        ping2 = 0; wait_us(5);

        ping2.input();
        while(ping2.read()==0);
        t2.start();
        while(ping2.read()==1);
        val2 = t2.read();
        printf("ping2 = %lf\r\n", val2*17700.4f);
        now2 = val2*17700;
        
        blue_led = 1;
        red_led = 0;
        t2.stop();
        t2.reset();   
    } 
    wait_ms(50);
    car.stop();
    
    //go backward
    blue_led = 1;
    red_led = 0;
    a = 0;
    car.goStraight(-100);
    wait_ms(2550);
    car.stop();

    //turn left
    red_led = 1;
    blue_led = 0;
    a = 1;
    car.turn(37,0.1);
    wait_ms(2880);
    car.stop();

    //go straight
    blue_led = 1;
    red_led = 0;
    a = 0;
    car.goStraight(100);
    wait_ms(2700);
    car.stop();
    
    //turn right
    red_led = 1;
    blue_led = 0;
    a = 2;
    car.turn(-31.5,0.1);
    wait_ms(2300);
    car.stop();

    //go straight
    blue_led = 1;
    red_led = 0;
    a = 0;
    car.goStraight(100);
    wait_ms(9300);
    car.stop();


    // wait_ms(1000);
    queue.dispatch();

    // while(encoder0.get_cm()<115) //wait_ms(50);
    // {
    //     wait_ms(50);
    //     car.goStraight(100);
    //     ping2.output();
    //     ping2 = 0; wait_us(200);
    //     ping2 = 1; wait_us(5);
    //     ping2 = 0; wait_us(5);

    //     ping2.input();
    //     while(ping2.read()==0);
    //     t2.start();
    //     while(ping2.read()==1);
    //     val2 = t2.read();
    //     printf("ping2 = %lf\r\n", val2*17700.4f);
    //     now2 = val2*17700;

    //     t2.stop();
    //     t2.reset();
    //     if(now2 < 6)
    //     {
    //         car.turn(25, 0.1);
    //         wait_ms(10);
    //         car.stop();
    //     }
    //     else if (now2 > 7)
    //     {
    //         car.turn(-15, 0.1);
    //         wait_ms(10);
    //         car.stop();
    //     }
    // } 
    // wait_ms(50);
    // car.stop();
    
    // //turn left
    // a = 1;
    // car.turn(37,0.1);
    // wait_ms(2720);
    // car.stop();

    // // go straight
    // a = 0;
    // encoder0.reset();
    // car.goStraight(100);

    // while(encoder0.get_cm()<92) wait_ms(50);
    // car.stop();

    // //turn right
    // a = 2;
    // car.turn(-31.5,0.1);
    
    // wait_ms(3250);
    // car.stop();
    // wait_ms(1000);

    // pc.baud(9600);
    // car.goStraight(-100);

    // // go backward
    // a = 3;
    // ping.output();
    // ping = 0; wait_us(200);
    // ping = 1; wait_us(5);
    // ping = 0; wait_us(5);

    // ping.input();
    // while(ping.read()==0);
    // t.start();
    // while(ping.read()==1);
    // val = t.read();
    // printf("Ping = %lf\r\n", val*17700.4f);
    // now = val*17700;
    // t.stop();
    // t.reset();

    // while(now > 20) {

    //     car.goStraight(-100);

    //     ping.output();
    //     ping = 0; wait_us(200);
    //     ping = 1; wait_us(5);
    //     ping = 0; wait_us(5);

    //     ping.input();
    //     while(ping.read()==0);
    //     t.start();
    //     while(ping.read()==1);
    //     val = t.read();
    //     printf("Ping = %lf\r\n", val*17700.4f);
    //     now = val*17700;
    //     t.stop();
    //     t.reset();

    //     // wait(0.5);
    // }
    // car.stop();
    // wait_ms(1000);

    // // go straight 
    // a = 0;
    // encoder0.reset();
    // car.goStraight(100);
    // while(encoder0.get_cm()<35) wait_ms(50);
    // car.stop();
    // wait_ms(100);

    // //turn right
    // a = 2;
    // car.turn(-31.5,0.1);
    // wait_ms(2980);
    // car.stop();

    // // go straight 
    // // a = 0;
    // // encoder0.reset();
    // // car.goStraight(100);
    // // while(encoder0.get_cm()<90) wait_ms(50);
    // // car.stop();

    
    
    // // detect object

    // a = 4;
    // encoder0.reset();
    // car.goStraight(30);

    // while(encoder0.get_cm()<50) //wait_ms(50);
    // {
    //     wait_ms(50);
    //     car.goStraight(30);
    //     ping2.output();
    //     ping2 = 0; wait_us(200);
    //     ping2 = 1; wait_us(5);
    //     ping2 = 0; wait_us(5);

    //     ping2.input();
    //     while(ping2.read()==0);
    //     t2.start();
    //     while(ping2.read()==1);
    //     val2 = t2.read();
    //     printf("ping2 = %lf\r\n", val2*17700.4f);
    //     now2 = val2*17700;

    //     t2.stop();
    //     t2.reset();
    // } 
    // wait_ms(50);
    // car.stop();
    // a = 0;
    // wait_ms(1000);
    // queue.dispatch();
}




