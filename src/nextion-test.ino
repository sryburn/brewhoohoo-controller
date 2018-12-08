// This #include statement was automatically added by the Particle IDE.
#include <ITEADLIB_Nextion.h>

USARTSerial& nexSerial = Serial1;

int counter = 1;

//declare stuff that we receive from display
NexButton b0 = NexButton(0,2,"b0");


NexTouch *nex_listen_list[] =
{
   &b0,
   NULL
};

void b0PushCallback(void *ptr){
    digitalWrite(D7, HIGH);
}

void b0PopCallback(void *ptr){
    digitalWrite(D7, LOW);
}


void setup() {
    Serial1.begin(9600);
    Serial1.print("baud=115200");
    Serial1.write(0xff);
    Serial1.write(0xff);
    Serial1.write(0xff);
    delay(50);
    Serial1.end();
    Serial1.begin(115200);
    
    b0.attachPush(b0PushCallback);
    b0.attachPop(b0PopCallback);
    pinMode(D7, OUTPUT);
    
}

void loop() {
    nexLoop(nex_listen_list);
    
    if (counter>100){
        counter = 1;
    } else{
        counter++;
    }
    
    Serial1.print("n0.val=");
    Serial1.print(counter);
    Serial1.write(0xff);
    Serial1.write(0xff);
    Serial1.write(0xff);
    delay(500);
    
}