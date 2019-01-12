/*
 * Project scottCustomNextion
 * Description:
 * Author:
 * Date:
 */

#include "Particle.h"
#include "NextionBasic.h"

SYSTEM_THREAD(ENABLED);

Timer mockWaveform(2222, generateWaveform); //270 POINTS ON CHART * 2.22 = 10 MINUTES
Timer countdownTimer(1000, updateCountdownTimer);

int counter;
int decaminutes;
int minutes;
int decaseconds;
int seconds;

bool timeToPublishWaveform = false;
bool timeToPublishCountdown = false;

//declare stuff that we receive from display
NexTouch boil = NexTouch(0,22,"m0");
NexTouch mash = NexTouch(0,23,"m1");
NexTouch hlt = NexTouch(0,24,"m2");
NexTouch coil = NexTouch(0,25,"m3"); //not used
NexTouch pump1 = NexTouch(0,26,"m4");
NexTouch pump2 = NexTouch(0,27,"m5");
NexTouch power = NexTouch(0,6,"b0");
NexTouch restart = NexTouch(0,8,"b1");
NexTouch countdown = NexTouch(0,9,"b2");
NexTouch play = NexTouch(0,10,"b3");
NexTouch settings = NexTouch(0,11,"b4");

NexTouch *nex_listen_list[] =
{
  &boil,
  &countdown,
  &play,
  &restart,
  NULL
};

void setup() {
  Serial1.begin(9600);
  Serial1.print("baud=115200");
  terminateCommand();
  delay(50);
  Serial1.end();
  Serial1.begin(115200);
  pinMode(D7, OUTPUT);
  boil.attachPush(boilPushCallback);
  boil.attachPop(boilPopCallback);
  play.attachPush(playPushCallback);
  restart.attachPush(restartPushCallback);
  restart.attachPop(restartPopCallback);
  resetTimer();
  mockWaveform.start(); 
  timeToPublishCountdown=true;
}

void loop() {
  listenForTouchEvents(nex_listen_list);
  if (timeToPublishWaveform==true){
    Serial1.print("add 21,0,");
    Serial1.print(counter);
    terminateCommand();
    timeToPublishWaveform = false;
  }

  if (timeToPublishCountdown==true){
      renderCounter();
      timeToPublishCountdown = false;
  }
}

//Callback Functions
void boilPushCallback(void *ptr){
    digitalWrite(D7, HIGH);
}

void boilPopCallback(void *ptr){
    digitalWrite(D7, LOW);
}

void playPushCallback(void *ptr){
    if (countdownTimer.isActive()) {
        Serial1.print("b3.picc=0");
        terminateCommand();
        countdownTimer.stop();
    } else{
        Serial1.print("b3.picc=1");
        terminateCommand();
        countdownTimer.start();
    }
}

void restartPushCallback(void *ptr){
    resetTimer();
    renderCounter();
    // Serial1.print("b1.picc=1");
}

void restartPopCallback(void *ptr){
  Serial1.print("b1.picc=0");
}

//Timer Functions
void updateCountdownTimer(){
    // if (decaminutes==0&&minutes==0&&decaseconds==0&&seconds==0){
    //     countdown.setText("ALARM");
    // } else{
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
    timeToPublishCountdown = true;
    // }
}
void resetTimer(){
  decaminutes = 6;
  minutes = 0;
  decaseconds = 0;
  seconds = 0;
}
void renderCounter(){
    Serial1.print("b2.txt=");
    Serial1.print("\"");
    Serial1.print(decaminutes);
    Serial1.print(minutes);
    Serial1.print(":");
    Serial1.print(decaseconds);
    Serial1.print(seconds);
    Serial1.print("\"");
    terminateCommand();
}

//Helper Functions
void generateWaveform(){
    if (counter>100){
        counter = 1;
    } else{
        counter++;
    }
    timeToPublishWaveform = true;
}

void terminateCommand(){
  Serial1.write(0xff);
  Serial1.write(0xff);
  Serial1.write(0xff);
}