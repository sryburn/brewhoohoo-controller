#include "Particle.h"
#include "JsonParserGeneratorRK.h"
#include "Touchscreen.h"
#include "CountdownTimer.h"
#include "RotaryEncoder.h"

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
JsonParserStatic<1024, 50> parser;
CountdownTimer countdownTimer;
Touchscreen touchscreen;

int mode = 0;

volatile int boilPower = 0; //mode0
// volatile double boilSetpoint = 0; //Not implemented yet
// volatile int hltPower = 0; //Not implemented yet
volatile double hltSetpoint = 0; //mode1
volatile double mashSetpoint = 0; //mode2
volatile int pump1Power = 0; //mode4
volatile int pump2Power = 0; //mode5

double mashTemp = 0.0;
double boilTemp = 0.0;
double coilTemp = 0.0;
double hltTemp = 0.0;
double boilElementState;
double hltElementState;

bool timeToPublishFlag = false;

void setup() {
  Serial.begin();
  Serial1.begin(9600);
  touchscreen.sendCommand("baud=115200");
  delay(50);
  Serial1.end();
  Serial1.begin(115200);
  setupEncoder();
  touchscreen.attachCallbacks();
  countdownTimer.reset();
  attachInterrupts();
  waitUntil(Mesh.ready);
  Mesh.subscribe("status", renderUpdatedState);
}

void loop() {
  touchscreen.checkForTouchEvents();
  dealWithButtonPress();
  // renderUpdatedTemperatures(boilTemp, hltTemp, mashTemp, coilTemp); //TODO move to subscribe event
  publishUpdatedSetpoints(boilPower, mashSetpoint, hltSetpoint, pump1Power, pump2Power);

  if (countdownTimer.hasUpdated() == true){
    touchscreen.sendCommand(countdownTimer.getClockText());
  }
}

void renderUpdatedTemperatures(double boilTemp, double hltTemp, double coilTemp, double mashTemp){
    static double prevMashTemp;
    static double prevBoilTemp;
    static double prevCoilTemp;
    static double prevHltTemp;

    if  (!is_equal_3decplaces(boilTemp, prevBoilTemp)){
      touchscreen.updateText(nexBoilTemp,boilTemp);
      prevBoilTemp = boilTemp;
    }

    if  (!is_equal_3decplaces(mashTemp, prevMashTemp)){
      touchscreen.updateText(nexMashTemp,mashTemp);
      prevMashTemp = mashTemp;
    }

    if  (!is_equal_3decplaces(hltTemp, prevHltTemp)){
      touchscreen.updateText(nexHltTemp,hltTemp);
      prevHltTemp = hltTemp;
    }

    if  (!is_equal_3decplaces(coilTemp, prevCoilTemp)){
      touchscreen.updateText(nexCoilTemp,coilTemp);
      prevCoilTemp = coilTemp;
    }
}

void renderUpdatedState(const char *event, const char *data){
  parser.clear();
  parser.addString(data);
  parser.parse();

  static int bsBoilPower;
 // static double bsBoilSetpoint;
  static double bsMashSetpoint;
 // static int bsHltPower;
  static double bsHltSetpoint;
  static int bsPump1Power;
  static int bsPump2Power;
  // static double bsMashTemp;
  // static double bsBoilTemp;
  // static double bsCoilTemp;
  // static double bshHltTemp;
  // static bool bsBoilElementOn;
  // static bool bsHltElementOn;
  // static int bsBoilMode;
  // static int bsHltMode;
  // static char bsCountdownTime[5];
  // static int bsTimerState;
  // static int bsCloudStatus;
  // static int bsMeshStatus;
  // static double bsBatteryVoltage;

  static int prevBsBoilPower;
//  static double prevBsBoilSetpoint;
  static double prevBsMashSetpoint;
//  static int prevBsHltPower;
  static double prevBsHltSetpoint;
  static int prevBsPump1Power;
  static int prevBsPump2Power;
  // static double prevBsMashTemp;
  // static double prevBsBoilTemp;
  // static double prevBsCoilTemp;
  // static double prevBshHltTemp;
  // static bool prevBsBoilElementOn;
  // static bool prevBsHltElementOn;
  // static int prevBsBoilMode;
  // static int prevBsHltMode;
  // static char prevBsCountdownTime[5];
  // static int prevBsTimerState;
  // static int prevBsCloudStatus;
  // static int prevBsMeshStatus;
  // static double prevBsBatteryVoltage;
  
  parser.getOuterValueByKey("boilPower", bsBoilPower);
  parser.getOuterValueByKey("mashSetpoint", bsMashSetpoint);
  parser.getOuterValueByKey("hltSetpoint", bsHltSetpoint);
  parser.getOuterValueByKey("pump1Power", bsPump1Power);
  parser.getOuterValueByKey("pump2Power", bsPump2Power);

  if (bsBoilPower != prevBsBoilPower){ 
    touchscreen.updateText(nexBoilSet,bsBoilPower);
    prevBsBoilPower = bsBoilPower;
  }

  if (bsMashSetpoint != prevBsMashSetpoint){ 
    if (bsMashSetpoint == -1){
      touchscreen.updateText(nexMashSet, "__._");
    } else {
      touchscreen.updateText(nexMashSet,bsMashSetpoint);
    }
    prevBsMashSetpoint = bsMashSetpoint;
  }

  if (bsHltSetpoint != prevBsHltSetpoint){
    if (bsHltSetpoint == -1){
      touchscreen.updateText(nexHltSet,"__._");
    } else{
      touchscreen.updateText(nexHltSet,bsHltSetpoint);
    }
    prevBsHltSetpoint = bsHltSetpoint;
  }

  if (bsPump1Power != prevBsPump1Power){ 
    touchscreen.updateText(nexPump1Set,bsPump1Power);
    prevBsPump1Power = bsPump1Power;
  }

  if (bsPump2Power != prevBsPump2Power){ 
    touchscreen.updateText(nexPump2Set,bsPump2Power);
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
    }

    if (hltSetpoint != prevHltSetpoint){
      char publishString[10];
      sprintf(publishString,"%f",hltSetpoint);
      Mesh.publish("setHltSetpoint", publishString);
      prevHltSetpoint = hltSetpoint;
     }

    if (pump1Power != prevPump1Power){
      char publishString[10];
      sprintf(publishString,"%d",pump1Power);
      Mesh.publish("setPump1Power", publishString);
      prevPump1Power = pump1Power;
    }

    if (pump2Power != prevPump2Power){
      char publishString[10];
      sprintf(publishString,"%d",pump2Power);
      Mesh.publish("setPump2Power", publishString);
      prevPump2Power = pump2Power;
    }
}

int is_equal_3decplaces(double a, double b) {
    long long ai = a * 1000;
    long long bi = b * 1000;
    return ai == bi;
}