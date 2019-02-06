#ifndef RotaryEncoder_h
#define RotaryEncoder_h

#include "State.h"

extern DeviceState dState;

void setButtonPressed();
void doEncoderA();
void doEncoderB();
void dealWithButtonPress();
void dealWithButtonPress();
void attachInterrupts();
void debounce();
void setupEncoder();

#endif