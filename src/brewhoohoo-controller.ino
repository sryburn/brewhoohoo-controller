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
Timer runDebounce(10, debounce);

int encoderA = D4;
int encoderB = D5;
int button = D6;
int led = D7;
int pump1 = WKP;
int pump2 = D0;
int boilElement = A2;
int hltElement = A1;

volatile int boilPower = 0; //mode0
volatile double hltSet = 0; //mode1
volatile double mashSet = 0; //mode2
volatile int pump1Power = 0; //mode4
volatile int pump2Power = 0; //mode5
volatile bool buttonPressed = false;
volatile bool A_set = false;
volatile bool B_set = false;

double mashTemp = 0.0;
double boilTemp = 0.0;
double coilTemp = 0.0;
double hltTemp = 0.0;
double boilElementState;
double hltElementState;

int counter;
int decaminutes;
int minutes;
int decaseconds;
int seconds;

int mode = 0;
int prevMode = 0;

bool buttonArmed = false;
bool longPressActive = false;
bool timeToPublishFlag = false;

unsigned long buttonTimer = 0;
unsigned long longPressTime = 2000;

volatile bool debounced = true;

bool timeToPublishWaveform = false;
bool timeToPublishCountdown = false;

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

void setup() {
  Serial1.begin(9600);
  sendCommand("baud=115200");
  delay(50);
  Serial1.end();
  Serial1.begin(115200);
  setPinModes();
  runDebounce.start();
  boilButton.attachPush(boilPushCallback);
  boilButton.attachPop(boilPopCallback);
  mashButton.attachPush(mashPushCallback);
  hltButton.attachPush(hltPushCallback);
  pump1Button.attachPush(pump1PushCallback);
  pump2Button.attachPush(pump2PushCallback);
  playButton.attachPush(playPushCallback);
  restartButton.attachPush(restartPushCallback);
  restartButton.attachPop(restartPopCallback);
  resetTimer();
  mockWaveform.start(); 
  timeToPublishCountdown=true;
  attachInterrupts();
}

void loop() {
  listenForTouchEvents(nex_listen_list);
  dealWithButtonPress();
  renderUpdatedTemperatures(boilTemp, hltTemp, mashTemp, coilTemp);
  renderUpdatedSetpoints(boilPower, hltSet, pump1Power, pump2Power);
  
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
  decaminutes = 9;
  minutes = 0;
  decaseconds = 0;
  seconds = 0;
}
void renderCounter(){
    char clockText[20];
    snprintf(clockText, sizeof(clockText), "b2.txt=\"%d%d:%d%d\"", decaminutes, minutes, decaseconds, seconds);
    sendCommand(clockText);
}

void debounce(){
    debounced = true;
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

void sendCommand(const char *command){
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

void updateTextInteger(int id, int value){
  Serial1.print("t");
  Serial1.print(id);
  Serial1.print(".txt=\"");
  Serial1.print(value);
  Serial1.print("\"");
  terminateCommand();  
}

void updateTextDouble(int id, double value){
  Serial1.print("t");
  Serial1.print(id);
  Serial1.print(".txt=\"");
  Serial1.print(value, 1);
  Serial1.print("\"");
  terminateCommand();  
}

void updateTextString(int id, const char *string){
  Serial1.print("t");
  Serial1.print(id);
  Serial1.print(".txt=\"");
  Serial1.print(string);
  Serial1.print("\"");
  terminateCommand();  
}

void renderUpdatedTemperatures(double boilTemp, double hltTemp, double coilTemp, double mashTemp){
    static double prevMashTemp;
    static double prevBoilTemp;
    static double prevCoilTemp;
    static double prevHltTemp;

    if  (!is_equal_3decplaces(boilTemp, prevBoilTemp)){
      updateTextDouble(0,boilTemp);
      prevBoilTemp = boilTemp;
    }

    if  (!is_equal_3decplaces(mashTemp, prevMashTemp)){
      updateTextDouble(2,mashTemp);
      prevMashTemp = mashTemp;
    }

    if  (!is_equal_3decplaces(hltTemp, prevHltTemp)){
      updateTextDouble(4,hltTemp);
      prevHltTemp = hltTemp;
    }

    if  (!is_equal_3decplaces(coilTemp, prevCoilTemp)){
      updateTextDouble(6,coilTemp);
      prevCoilTemp = coilTemp;
    }
}

void renderUpdatedSetpoints(int boilPower, double hltSet, int pump1Power, int pump2Power){
    static int prevBoilPower = -1;
    static double prevHltSet = -1;
    static double prevMashSet = -1;
    static int prevPump2Power = -1;
    static int prevPump1Power = -1;

    if (boilPower != prevBoilPower){
        updateTextInteger(1,boilPower);
        prevBoilPower = boilPower;
    }
    
    if (mashSet != prevMashSet){
        updateTextDouble(3,mashSet);
        prevMashSet = mashSet;
        updateTextString(5,"__._");
    }

    if (hltSet != prevHltSet){
        updateTextDouble(5,hltSet);
        prevHltSet = hltSet;
        updateTextString(3,"__._");
    }

    if (pump2Power != prevPump2Power){
        updateTextInteger(7,pump2Power);
        prevPump2Power = pump2Power;
    }

    if (pump1Power != prevPump1Power){
        updateTextInteger(8,pump1Power);
        prevPump1Power = pump1Power;
    }
}

int is_equal_3decplaces(double a, double b) {
    long long ai = a * 1000;
    long long bi = b * 1000;
    return ai == bi;
}

void resetMode(){
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

void terminateCommand(){
  Serial1.write(0xff);
  Serial1.write(0xff);
  Serial1.write(0xff);
}

void attachInterrupts(){
    attachInterrupt(button, setButtonPressed, RISING);
    attachInterrupt(encoderA, doEncoderA, CHANGE);
    attachInterrupt(encoderB, doEncoderB, CHANGE);
}

void setPinModes(){
    pinMode(encoderA, INPUT_PULLUP);
    pinMode(encoderB, INPUT_PULLUP);
    pinMode(button, INPUT_PULLUP);
    pinMode(pump1, OUTPUT);
    pinMode(pump2, OUTPUT);
    pinMode(boilElement, OUTPUT);
    pinMode(hltElement, OUTPUT);
    pinMode(led, OUTPUT);
}

void dealWithButtonPress(){
  if(buttonPressed && (pinReadFast(button) == HIGH)){
    switch(mode){
      case 0:
      if (boilPower > 0){boilPower = 0;}
      else {boilPower = 100;}
      break;

      case 1:
      if (mashSet > 0){mashSet = 0;}
      else {mashSet = 70;}
      break;

      case 2:
      if (hltSet > 0){hltSet = 0;}
      else {hltSet = 70;}
      break;

      case 4:
      if (pump2Power > 0){pump2Power = 0;}
      else {pump2Power = 100;}
      break;

      case 5 :
      if (pump1Power > 0){pump1Power = 0;}
      else {pump1Power = 100;}
      break;
    }
    buttonPressed = false;
  }
}

//interrupt functions
//***INTERRUPT FUNCTIONS***//

void setButtonPressed() {
    buttonPressed = true;
}

void doEncoderA() {
  if(digitalRead(encoderA) != A_set ) {  // debounce once more
    A_set = !A_set;
    // adjust counter + if A leads B
    if (A_set && !B_set && debounced){
      if (mode == 0 && boilPower < 100){
          boilPower += 1;
      } else if (mode == 1 && mashSet < 100){
          mashSet += 0.1;
      } else if (mode == 2 && hltSet < 100){
          hltSet += 0.1;
      } else if (mode == 4 && pump2Power < 100){
          pump2Power += 1;
      } else if (mode == 5 && pump1Power < 100){
          pump1Power += 1;
      }
      debounced = false;
    }
  }
}

void doEncoderB() {
    if(digitalRead(encoderB) != B_set) {
    B_set = !B_set;
    //  adjust counter - 1 if B leads A
        if(B_set && !A_set && debounced){
          if (mode == 0 && boilPower > 0){
              boilPower -= 1;
          } else if (mode == 1 && mashSet > 0.05){
              mashSet -= 0.1;
          } else if (mode == 2 && hltSet > 0.05){
              hltSet -= 0.1;
          } else if (mode == 4 && pump2Power > 0){
              pump2Power -= 1;
          } else if (mode == 5 && pump1Power > 0){
              pump1Power -= 1;
          }
          debounced = false;
        }
    }
}