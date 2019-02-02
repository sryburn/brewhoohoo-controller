#ifndef RotaryEncoder_h
#define RotaryEncoder_h

extern int mode;

extern volatile int boilPower; //mode0
// extern volatile double boilSetpoint; //Not implemented yet
// extern volatile int hltPower; //Not implemented yet
extern volatile double hltSetpoint; //mode1
extern volatile double mashSetpoint; //mode2
extern volatile int pump1Power; //mode4
extern volatile int pump2Power; //mode5

void setButtonPressed();
void doEncoderA();
void doEncoderB();
void dealWithButtonPress();
void dealWithButtonPress();
void attachInterrupts();
void debounce();
void setupEncoder();

#endif