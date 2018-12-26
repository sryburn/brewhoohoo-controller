// This #include statement was automatically added by the Particle IDE.
#include <ITEADLIB_Nextion.h>
#include<string>
USARTSerial& nexSerial = Serial1;

SYSTEM_THREAD(ENABLED);

Timer mockWaveform(500, generateWaveform);
Timer countdownTimer(1000, updateCountdownTimer);

int counter = 1;
int decaminutes = 1;
int minutes = 1;
int decaseconds = 0;
int seconds = 0;

//declare stuff that we receive from display
NexButton boil = NexButton(0,22,"m0");
NexButton mash = NexButton(0,23,"m1");
NexButton hlt = NexButton(0,24,"m2");
NexButton coil = NexButton(0,25,"m3"); //not used
NexButton pump1 = NexButton(0,26,"m4");
NexButton pump2 = NexButton(0,27,"m5");

NexButton power = NexButton(0,6,"b0");
NexButton restart = NexButton(0,8,"b1");
NexButton countdown = NexButton(0,9,"b2");
NexButton play = NexButton(0,10,"b3");
NexButton settings = NexButton(0,11,"b4");

NexWaveform chart = NexWaveform(0,21,"s0");
NexTouch *nex_listen_list[] =
{
   &boil,
   &countdown,
   NULL
};

void boilPushCallback(void *ptr){
    digitalWrite(D7, HIGH);
    countdown.setText("60:00");
}

void boilPopCallback(void *ptr){
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
    
    boil.attachPush(boilPushCallback);
    boil.attachPop(boilPopCallback);
    pinMode(D7, OUTPUT);
    mockWaveform.start();
    countdownTimer.start();
    
}

void loop() {
    nexLoop(nex_listen_list);
}

void generateWaveform(){
    if (counter>100){
        counter = 1;
    } else{
        counter++;
    }
    chart.addValue(0,counter);
}

void updateCountdownTimer(){
    //Serial1.print("b2.txt=");
    if (decaminutes==0&&minutes==0&&decaseconds==0&&seconds==0){
        countdown.setText("ALARM");
    } else{
        if (seconds>0){
            seconds--;
        } else if (decaseconds>0){
            decaseconds--;
            seconds=9;
        } else if (minutes>0){
            minutes--;
            decaseconds=5;
            seconds=9;
        } else if (decaminutes>0){
            decaminutes--;
            minutes=9;
            decaseconds=5;
            seconds=9;
        }
        char buf[10];
        snprintf(buf, sizeof(buf), "%d%d:%d%d", decaminutes, minutes, decaseconds, seconds);
        countdown.setText(buf);
    }
        // }
        // Serial1.print("\"");
        // if (minutes<10){Serial1.print("0");}
        // Serial1.print(minutes);
        // Serial1.print(":");
        // if (seconds<10){Serial1.print("0");}
        // Serial1.print(seconds);
        // Serial1.print("\"");
        // Serial1.write(0xff);
        // Serial1.write(0xff);
        // Serial1.write(0xff);
}