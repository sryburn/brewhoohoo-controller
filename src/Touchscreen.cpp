#include "Touchscreen.h"
#include "application.h"

int Touchscreen::prevMode = 0;

//declare stuff that we receive from display
NexTouch boilButton = NexTouch(0,22,"m0");
NexTouch mashButton = NexTouch(0,23,"m1");
NexTouch hltButton = NexTouch(0,24,"m2");
NexTouch coilButton = NexTouch(0,25,"m3"); //not used
NexTouch pump1Button = NexTouch(0,26,"m4");
NexTouch pump2Button = NexTouch(0,27,"m5");
NexTouch powerButton = NexTouch(0,6,"b0");
NexTouch restartButton = NexTouch(0,8,"b1");
NexTouch countdownButton = NexTouch(0,9,"b2");
NexTouch playButton = NexTouch(0,10,"b3");
NexTouch settingsButton = NexTouch(0,11,"b4");

NexTouch *nex_listen_list[] =
{
  &boilButton,
  &mashButton,
  &hltButton,
  &pump1Button,
  &pump2Button,
  &countdownButton,
  &playButton,
  &restartButton,
  NULL
};

void Touchscreen::terminateCommand(){
  Serial1.write(0xff);
  Serial1.write(0xff);
  Serial1.write(0xff);
}

void Touchscreen::sendCommand(const char *command){
  Serial1.print(command);
  terminateCommand();
}

void Touchscreen::updateChart(int channel, int value){
  Serial1.print("add 21,");
  Serial1.print(channel);
  Serial1.print(",");
  Serial1.print(value);
  terminateCommand();  
}

void Touchscreen::updateClock(const char *clock){
  Serial1.print("b2.txt=\"");
  Serial1.print(clock);
  Serial1.print("\"");
  terminateCommand();  
}

void Touchscreen::updateText(int id, int value){
  Serial1.print("t");
  Serial1.print(id);
  Serial1.print(".txt=\"");
  Serial1.print(value);
  Serial1.print("\"");
  terminateCommand();  
}

void Touchscreen::updateText(int id, double value){
  Serial1.print("t");
  Serial1.print(id);
  Serial1.print(".txt=\"");
  Serial1.print(value, 1);
  Serial1.print("\"");
  terminateCommand();  
}

void Touchscreen::updateText(int id, const char *string){
  Serial1.print("t");
  Serial1.print(id);
  Serial1.print(".txt=\"");
  Serial1.print(string);
  Serial1.print("\"");
  terminateCommand();  
}

void Touchscreen::resetMode(){
  Serial1.print("q");
  Serial1.print(prevMode);
  Serial1.print(".picc=0");
  terminateCommand();
  sendCommand("t0.font=2");
  sendCommand("t1.font=2");
  sendCommand("t2.font=2");
  sendCommand("t3.font=2");
  sendCommand("t4.font=2");
  sendCommand("t5.font=2");
  sendCommand("t6.font=2");
  sendCommand("t7.font=2");
  sendCommand("t8.font=2");
}

void Touchscreen::boilPushCallback(void *ptr){
    prevMode=dState.mode;
    dState.mode=0;
    resetMode();
    digitalWrite(D7, HIGH);
    sendCommand("q0.picc=1");
}

void Touchscreen::boilPopCallback(void *ptr){
    digitalWrite(D7, LOW);
}

void Touchscreen::mashPushCallback(void *ptr){
    prevMode=dState.mode;
    dState.mode=1;
    resetMode();
    sendCommand("q1.picc=1");
}

void Touchscreen::hltPushCallback(void *ptr){
    prevMode=dState.mode;
    dState.mode=2;
    resetMode();
    sendCommand("q2.picc=1");
}

void Touchscreen::pump1PushCallback(void *ptr){
    prevMode=dState.mode;
    dState.mode=4;
    resetMode();
    sendCommand("q4.picc=1");
}

void Touchscreen::pump2PushCallback(void *ptr){
    prevMode=dState.mode;
    dState.mode=5;
    resetMode();
    sendCommand("q5.picc=1");
}

void Touchscreen::playPushCallback(void *ptr){
  if (dState.timerStarted){
    Mesh.publish("setTimerStop");
    sendCommand("b3.picc=0");
  } else{
    Mesh.publish("setTimerStart");
    sendCommand("b3.picc=1");
  }
  dState.timerStarted = !dState.timerStarted;
}

void Touchscreen::restartPushCallback(void *ptr){
    Mesh.publish("setTimerReset");
    sendCommand("b1.picc=1");
}

void Touchscreen::restartPopCallback(void *ptr){
  sendCommand("b1.picc=0");
}

void Touchscreen::attachCallbacks(){
  boilButton.attachPush(boilPushCallback);
  boilButton.attachPop(boilPopCallback);
  mashButton.attachPush(mashPushCallback);
  hltButton.attachPush(hltPushCallback);
  pump1Button.attachPush(pump1PushCallback);
  pump2Button.attachPush(pump2PushCallback);
  playButton.attachPush(playPushCallback);
  restartButton.attachPush(restartPushCallback);
  restartButton.attachPop(restartPopCallback);
}

void Touchscreen::checkForTouchEvents(){
  listenForTouchEvents(nex_listen_list);
}
