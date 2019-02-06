#include "Particle.h"
#include "JsonParserGeneratorRK.h"
#include "Touchscreen.h"
#include "RotaryEncoder.h"
#include "State.h"

#define nexBoilSet 1
#define nexMashSet 3
#define nexHltSet 5
#define nexPump1Set 8
#define nexPump2Set 7
#define nexBoilTemp 0
#define nexMashTemp 2
#define nexHltTemp 4
#define nexCoilTemp 6
#define nexClock 

SYSTEM_THREAD(ENABLED);
JsonParserStatic<1024, 50> parser;
Touchscreen touchscreen;
DeviceState dState;

void setup() {
  Serial.begin();
  Serial1.begin(9600);
  touchscreen.sendCommand("baud=115200");
  delay(50);
  Serial1.end();
  Serial1.begin(115200);
  setupEncoder();
  touchscreen.attachCallbacks();
  attachInterrupts();
  waitUntil(Mesh.ready);
  Mesh.subscribe("status", renderUpdatedState);
}

void loop() {
  touchscreen.checkForTouchEvents();
  dealWithButtonPress();
  publishUpdatedSetpoints(dState);
}

void renderUpdatedState(const char *event, const char *data){
  parser.clear();
  parser.addString(data);
  parser.parse();

  static SystemState state;
  static SystemState prevState;
 
  parser.getOuterValueByKey("boilPower", state.boilPower);
  parser.getOuterValueByKey("mashSetpoint", state.mashSetpoint);
  parser.getOuterValueByKey("hltSetpoint", state.hltSetpoint);
  parser.getOuterValueByKey("pump1Power", state.pump1Power);
  parser.getOuterValueByKey("pump2Power", state.pump2Power);
  parser.getOuterValueByKey("boilTemp", state.boilTemp);
  parser.getOuterValueByKey("mashTemp", state.mashTemp);
  parser.getOuterValueByKey("hltTemp", state.hltTemp);
  parser.getOuterValueByKey("coilTemp", state.coilTemp);
  parser.getOuterValueByKey("countdownTime", state.countdownTime);
  parser.getOuterValueByKey("timerStarted", state.timerStarted);

  if (state.timerStarted != prevState.timerStarted){
    if (state.timerStarted){
      touchscreen.sendCommand("b3.picc=1");
    } else{
      touchscreen.sendCommand("b3.picc=0");
    }
    prevState.timerStarted = state.timerStarted;
  }

  if (state.countdownTime != prevState.countdownTime){ 
    touchscreen.updateClock(state.countdownTime);
    prevState.countdownTime = state.countdownTime;
  }

  if (state.boilPower != prevState.boilPower){ 
    touchscreen.updateText(nexBoilSet,state.boilPower);
    prevState.boilPower = state.boilPower;
  }

  if (state.mashSetpoint != prevState.mashSetpoint){ 
    if (state.mashSetpoint == -1){
      touchscreen.updateText(nexMashSet, "__._");
    } else {
      touchscreen.updateText(nexMashSet,state.mashSetpoint);
    }
    prevState.mashSetpoint = state.mashSetpoint;
  }

  if (state.hltSetpoint != prevState.hltSetpoint){
    if (state.hltSetpoint == -1){
      touchscreen.updateText(nexHltSet,"__._");
    } else{
      touchscreen.updateText(nexHltSet,state.hltSetpoint);
    }
    prevState.hltSetpoint = state.hltSetpoint;
  }

  if (state.pump1Power != prevState.pump1Power){ 
    touchscreen.updateText(nexPump1Set,state.pump1Power);
    prevState.pump1Power = state.pump1Power;
  }

  if (state.pump2Power != prevState.pump2Power){ 
    touchscreen.updateText(nexPump2Set,state.pump2Power);
    prevState.pump2Power = state.pump2Power;
  }

  if (!is_equal_3decplaces(state.boilTemp, prevState.boilTemp)){
    touchscreen.updateText(nexBoilTemp,state.boilTemp);
    prevState.boilTemp = state.boilTemp;
  }

  if (!is_equal_3decplaces(state.mashTemp, prevState.mashTemp)){
    touchscreen.updateText(nexMashTemp,state.mashTemp);
    prevState.mashTemp = state.mashTemp;
  }

  if (!is_equal_3decplaces(state.hltTemp, prevState.hltTemp)){
    touchscreen.updateText(nexHltTemp,state.hltTemp);
    prevState.hltTemp = state.hltTemp;
  }

  if (!is_equal_3decplaces(state.coilTemp, prevState.coilTemp)){
    touchscreen.updateText(nexCoilTemp,state.coilTemp);
    prevState.coilTemp = state.coilTemp;
  }
}

void publishUpdatedSetpoints(DeviceState &dState){
    static int prevBoilPower;
    static double prevHltSetpoint;
    static double prevMashSetpoint;
    static int prevPump2Power;
    static int prevPump1Power;

    if (dState.boilPower != prevBoilPower){
      char publishString[10];
      sprintf(publishString,"%d",dState.boilPower);
      Mesh.publish("setBoilPower", publishString);
      prevBoilPower = dState.boilPower;
    }
    
    if (dState.mashSetpoint != prevMashSetpoint){
      char publishString[10];
      sprintf(publishString,"%f",dState.mashSetpoint);
      Mesh.publish("setMashSetpoint", publishString);
      prevMashSetpoint = dState.mashSetpoint;
    }

    if (dState.hltSetpoint != prevHltSetpoint){
      char publishString[10];
      sprintf(publishString,"%f",dState.hltSetpoint);
      Mesh.publish("setHltSetpoint", publishString);
      prevHltSetpoint = dState.hltSetpoint;
     }

    if (dState.pump1Power != prevPump1Power){
      char publishString[10];
      sprintf(publishString,"%d",dState.pump1Power);
      Mesh.publish("setPump1Power", publishString);
      prevPump1Power = dState.pump1Power;
    }

    if (dState.pump2Power != prevPump2Power){
      char publishString[10];
      sprintf(publishString,"%d",dState.pump2Power);
      Mesh.publish("setPump2Power", publishString);
      prevPump2Power = dState.pump2Power;
    }
}

int is_equal_3decplaces(double a, double b) {
    long long ai = a * 1000;
    long long bi = b * 1000;
    return ai == bi;
}