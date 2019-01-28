#include "Particle.h"
#include "NextionBasic.h"
#include "JsonParserGeneratorRK.h"
#include "CountdownTimer.h"

#define nexBoilSet 1
#define nexMashSet 3
#define nexHltSet 5
#define nexPump1Set 8
#define nexPump2Set 7
#define nexBoilTemp 0
#define nexMashTemp 2
#define nexHltTemp 4
#define nexCoilTemp 6


SYSTEM_THREAD(ENABLED);

Timer mockWaveform(2222, generateWaveform); //270 POINTS ON CHART * 2.22 = 10 MINUTES

Timer runDebounce(10, debounce);
JsonParserStatic<1024, 50> parser;

CountdownTimer countdownTimer;

int counter; //for Mock Waveform

int encoderA = D4;
int encoderB = D5;
int button = D6;
int led = D7;
int pump1 = WKP;
int pump2 = D0;
int boilElement = A2;
int hltElement = A1;

volatile int boilPower = 0; //mode0
// volatile double boilSetpoint = 0; //Not implemented yet
// volatile int hltPower = 0; //Not implemented yet
volatile double hltSetpoint = 0; //mode1
volatile double mashSetpoint = 0; //mode2
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

int mode = 0;
int prevMode = 0;

bool buttonArmed = false;
bool longPressActive = false;
bool timeToPublishFlag = false;

unsigned long buttonTimer = 0;
unsigned long longPressTime = 2000;

volatile bool debounced = true;

bool timeToPublishWaveform = false;


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
  Serial.begin();
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
  countdownTimer.reset();
  mockWaveform.start(); 
  attachInterrupts();
  waitUntil(Mesh.ready);
  Mesh.subscribe("status", renderUpdatedState);
}

void loop() {
  listenForTouchEvents(nex_listen_list);
  dealWithButtonPress();
  // renderUpdatedTemperatures(boilTemp, hltTemp, mashTemp, coilTemp); //TODO move to subscribe event
  publishUpdatedSetpoints(boilPower, mashSetpoint, hltSetpoint, pump1Power, pump2Power);
  
  if (timeToPublishWaveform==true){
    updateChart(0,counter);
    timeToPublishWaveform = false;
  }

  if (countdownTimer.hasUpdated() == true){
    sendCommand(countdownTimer.getClockText());
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
    if (countdownTimer.active()) {
        sendCommand("b3.picc=0");
        countdownTimer.stop();
    } else{
        sendCommand("b3.picc=1");
        countdownTimer.start();
    }
}

void restartPushCallback(void *ptr){
    countdownTimer.reset();  
    sendCommand(countdownTimer.getClockText());
    sendCommand("b1.picc=1");
}

void restartPopCallback(void *ptr){
  sendCommand("b1.picc=0");
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

void updateText(int id, int value){
  Serial1.print("t");
  Serial1.print(id);
  Serial1.print(".txt=\"");
  Serial1.print(value);
  Serial1.print("\"");
  terminateCommand();  
}

void updateText(int id, double value){
  Serial1.print("t");
  Serial1.print(id);
  Serial1.print(".txt=\"");
  Serial1.print(value, 1);
  Serial1.print("\"");
  terminateCommand();  
}

void updateText(int id, const char *string){
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
      updateText(nexBoilTemp,boilTemp);
      prevBoilTemp = boilTemp;
    }

    if  (!is_equal_3decplaces(mashTemp, prevMashTemp)){
      updateText(nexMashTemp,mashTemp);
      prevMashTemp = mashTemp;
    }

    if  (!is_equal_3decplaces(hltTemp, prevHltTemp)){
      updateText(nexHltTemp,hltTemp);
      prevHltTemp = hltTemp;
    }

    if  (!is_equal_3decplaces(coilTemp, prevCoilTemp)){
      updateText(nexCoilTemp,coilTemp);
      prevCoilTemp = coilTemp;
    }
}

void renderUpdatedState(const char *event, const char *data){
  parser.clear();
  parser.addString(data);
  parser.parse();

  static int bsBoilPower;
  static double bsBoilSetpoint;
  static double bsMashSetpoint;
  static int bsHltPower;
  static double bsHltSetpoint;
  static int bsPump1Power;
  static int bsPump2Power;
  static double bsMashTemp;
  static double bsBoilTemp;
  static double bsCoilTemp;
  static double bshHltTemp;
  static bool bsBoilElementOn;
  static bool bsHltElementOn;
  static int bsBoilMode;
  static int bsHltMode;
  static char bsCountdownTime[5];
  static int bsTimerState;
  static int bsCloudStatus;
  static int bsMeshStatus;
  static double bsBatteryVoltage;

  static int prevBsBoilPower;
  static double prevBsBoilSetpoint;
  static double prevBsMashSetpoint;
  static int prevBsHltPower;
  static double prevBsHltSetpoint;
  static int prevBsPump1Power;
  static int prevBsPump2Power;
  static double prevBsMashTemp;
  static double prevBsBoilTemp;
  static double prevBsCoilTemp;
  static double prevBshHltTemp;
  static bool prevBsBoilElementOn;
  static bool prevBsHltElementOn;
  static int prevBsBoilMode;
  static int prevBsHltMode;
  static char prevBsCountdownTime[5];
  static int prevBsTimerState;
  static int prevBsCloudStatus;
  static int prevBsMeshStatus;
  static double prevBsBatteryVoltage;
  
  parser.getOuterValueByKey("boilPower", bsBoilPower);
  parser.getOuterValueByKey("mashSetpoint", bsMashSetpoint);
  parser.getOuterValueByKey("hltSetpoint", bsHltSetpoint);
  parser.getOuterValueByKey("pump1Power", bsPump1Power);
  parser.getOuterValueByKey("pump2Power", bsPump2Power);

  if (bsBoilPower != prevBsBoilPower){ 
    updateText(nexBoilSet,bsBoilPower);
    prevBsBoilPower = bsBoilPower;
  }

  if (bsMashSetpoint != prevBsMashSetpoint){ 
    if (bsMashSetpoint == -1){
      updateText(nexMashSet, "__._");
    } else {
      updateText(nexMashSet,bsMashSetpoint);
    }
    prevBsMashSetpoint = bsMashSetpoint;
  }

  if (bsHltSetpoint != prevBsHltSetpoint){
    if (bsHltSetpoint == -1){
      updateText(nexHltSet,"__._");
    } else{
      updateText(nexHltSet,bsHltSetpoint);
    }
    prevBsHltSetpoint = bsHltSetpoint;
  }

  if (bsPump1Power != prevBsPump1Power){ 
    updateText(nexPump1Set,bsPump1Power);
    prevBsPump1Power = bsPump1Power;
  }

  if (bsPump2Power != prevBsPump2Power){ 
    updateText(nexPump2Set,bsPump2Power);
    prevBsPump2Power = bsPump2Power;
  }

  

}

void publishUpdatedSetpoints(int boilPower, double mashSetpoint, double hltSetpoint, int pump1Power, int pump2Power){
    static int prevBoilPower;
    static double prevHltSetpoint;
    static double prevMashSetpoint;
    static int prevPump2Power;
    static int prevPump1Power;

    if (boilPower != prevBoilPower){
      char publishString[10];
      sprintf(publishString,"%d",boilPower);
      Mesh.publish("setBoilPower", publishString);
      prevBoilPower = boilPower;
    }
    
    if (mashSetpoint != prevMashSetpoint){
      char publishString[10];
      sprintf(publishString,"%f",mashSetpoint);
      Mesh.publish("setMashSetpoint", publishString);
      prevMashSetpoint = mashSetpoint;
      // updateText(3,mashSetpoint);
      // prevMashSetpoint = mashSetpoint;
      // updateText(5,"__._");
    }

    if (hltSetpoint != prevHltSetpoint){
      char publishString[10];
      sprintf(publishString,"%f",hltSetpoint);
      Mesh.publish("setHltSetpoint", publishString);
      prevHltSetpoint = hltSetpoint;
      // updateText(5,hltSetpoint);
      // prevHltSetpoint = hltSetpoint;
      // updateText(3,"__._");
    }

    if (pump1Power != prevPump1Power){
      char publishString[10];
      sprintf(publishString,"%d",pump1Power);
      Mesh.publish("setPump1Power", publishString);
      prevPump1Power = pump1Power;
      // updateText(8,pump1Power);
      // prevPump1Power = pump1Power;
    }

    if (pump2Power != prevPump2Power){
      char publishString[10];
      sprintf(publishString,"%d",pump2Power);
      Mesh.publish("setPump2Power", publishString);
      prevPump2Power = pump2Power;
      // updateText(7,pump2Power);
      // prevPump2Power = pump2Power;
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
    attachInterrupt(button, setButtonPressed, FALLING);
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
      if (mashSetpoint > 0){mashSetpoint = 0;}
      else {mashSetpoint = 70;}
      break;

      case 2:
      if (hltSetpoint > 0){hltSetpoint = 0;}
      else {hltSetpoint = 70;}
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
      } else if (mode == 1 && mashSetpoint < 100){
          mashSetpoint += 0.1;
      } else if (mode == 2 && hltSetpoint < 100){
          hltSetpoint += 0.1;
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
          } else if (mode == 1 && mashSetpoint > 0.05){
              mashSetpoint -= 0.1;
          } else if (mode == 2 && hltSetpoint > 0.05){
              hltSetpoint -= 0.1;
          } else if (mode == 4 && pump2Power > 0){
              pump2Power -= 1;
          } else if (mode == 5 && pump1Power > 0){
              pump1Power -= 1;
          }
          debounced = false;
        }
    }
}