#ifndef State_h
#define State_h

#include "application.h"
struct SystemState{
  int boilPower; //mode0
  double hltSetpoint; //mode1
  double mashSetpoint ; //mode2
  int pump1Power; //mode4
  int pump2Power; //mode5

  double mashTemp;
  double boilTemp;
  double coilTemp;
  double hltTemp;
  bool boilElementOn;
  bool hltElementOn;
  String countdownTime;
  bool timerStarted;

  //NOT IMPLEMENTED YET
  // double boilSetpoint
  // int hltPower
  // static int boilMode;
  // static int bltMode;
  // static int timerState;
  // static int cloudStatus;
  // static int meshStatus;
  // static double batteryVoltage;
};

struct DeviceState{
  int mode;
  bool timerStarted;
  volatile int boilPower; //mode0
  // volatile double boilSetpoint = 0; //Not implemented yet
  // volatile int hltPower = 0; //Not implemented yet
  volatile double hltSetpoint; //mode1
  volatile double mashSetpoint ; //mode2
  volatile int pump1Power; //mode4
  volatile int pump2Power; //mode5
};

#endif