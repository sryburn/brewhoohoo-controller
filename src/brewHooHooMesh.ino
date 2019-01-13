/*
 * Project BrewHooHoo
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

int mode = 0;
int prevMode = 0;

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
  &mash,
  &hlt,
  &pump1,
  &pump2,
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
  mash.attachPush(mashPushCallback);
  hlt.attachPush(hltPushCallback);
  pump1.attachPush(pump1PushCallback);
  pump2.attachPush(pump2PushCallback);
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
    updateChart(0,counter);
    timeToPublishWaveform = false;
  }

  if (timeToPublishCountdown==true){
      renderCounter();
      timeToPublishCountdown = false;
  }
}

//Callback Functions
void boilPushCallback(void *ptr){
    prevMode=mode;
    mode=0;
    resetMode();
    digitalWrite(D7, HIGH);
    sendCommand("q0.picc=1");
}

void boilPopCallback(void *ptr){
    digitalWrite(D7, LOW);
}

void mashPushCallback(void *ptr){
    prevMode=mode;
    mode=1;
    resetMode();
    sendCommand("q1.picc=1");
}

void hltPushCallback(void *ptr){
    prevMode=mode;
    mode=2;
    resetMode();
    sendCommand("q2.picc=1");
}

void pump1PushCallback(void *ptr){
    prevMode=mode;
    mode=4;
    resetMode();
    sendCommand("q4.picc=1");
}

void pump2PushCallback(void *ptr){
    prevMode=mode;
    mode=5;
    resetMode();
    sendCommand("q5.picc=1");
}

void playPushCallback(void *ptr){
    if (countdownTimer.isActive()) {
        sendCommand("b3.picc=0");
        countdownTimer.stop();
    } else{
        sendCommand("b3.picc=1");
        countdownTimer.start();
    }
}

void restartPushCallback(void *ptr){
    resetTimer();
    renderCounter();
    // Serial1.print("b1.picc=1");
}

void restartPopCallback(void *ptr){
  sendCommand("b1.picc=0");
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
    char clockText[20];
    snprintf(clockText, sizeof(clockText), "b2.txt=\"%d%d:%d%d\"", decaminutes, minutes, decaseconds, seconds);
    sendCommand(clockText);
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

void sendCommand(char *command){
  Serial1.print(command);
  terminateCommand();
}

void updateChart(int channel, int value){
  Serial1.print("add 21,");
  Serial1.print(channel);
  Serial1.print(",");
  Serial1.print(value);
  terminateCommand();  
}

void resetMode(){
  Serial1.print("q");
  Serial1.print(prevMode);
  Serial1.print(".picc=0");
  terminateCommand();
}

void terminateCommand(){
  Serial1.write(0xff);
  Serial1.write(0xff);
  Serial1.write(0xff);
}