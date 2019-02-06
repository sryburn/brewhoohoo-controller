#include "RotaryEncoder.h"
#include "application.h"

Timer runDebounce(10, debounce);

int encoderA = D4;
int encoderB = D5;
int button = D6;

volatile bool buttonPressed = false;
volatile bool A_set = false;
volatile bool B_set = false;
volatile bool debounced = true;

bool buttonArmed = false;

void setButtonPressed() {
    buttonPressed = true;
}

void doEncoderA() {
  if(digitalRead(encoderA) != A_set ) {  // debounce once more
    A_set = !A_set;
    // adjust counter + if A leads B
    if (A_set && !B_set && debounced){
      if (dState.mode == 0 && dState.boilPower < 100){
          dState.boilPower += 1;
      } else if (dState.mode == 1 && dState.mashSetpoint < 100){
          dState.mashSetpoint += 0.1;
      } else if (dState.mode == 2 && dState.hltSetpoint < 100){
          dState.hltSetpoint += 0.1;
      } else if (dState.mode == 4 && dState.pump2Power < 100){
          dState.pump2Power += 1;
      } else if (dState.mode == 5 && dState.pump1Power < 100){
          dState.pump1Power += 1;
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
          if (dState.mode == 0 && dState.boilPower > 0){
              dState.boilPower -= 1;
          } else if (dState.mode == 1 && dState.mashSetpoint > 0.05){
              dState.mashSetpoint -= 0.1;
          } else if (dState.mode == 2 && dState.hltSetpoint > 0.05){
              dState.hltSetpoint -= 0.1;
          } else if (dState.mode == 4 && dState.pump2Power > 0){
              dState.pump2Power -= 1;
          } else if (dState.mode == 5 && dState.pump1Power > 0){
              dState.pump1Power -= 1;
          }
          debounced = false;
        }
    }
}

void dealWithButtonPress(){
  if(buttonPressed && (pinReadFast(button) == HIGH)){
    switch(dState.mode){
      case 0:
      if (dState.boilPower > 0){dState.boilPower = 0;}
      else {dState.boilPower = 100;}
      break;

      case 1:
      if (dState.mashSetpoint > 0){dState.mashSetpoint = 0;}
      else {dState.mashSetpoint = 70;}
      break;

      case 2:
      if (dState.hltSetpoint > 0){dState.hltSetpoint = 0;}
      else {dState.hltSetpoint = 70;}
      break;

      case 4:
      if (dState.pump2Power > 0){dState.pump2Power = 0;}
      else {dState.pump2Power = 100;}
      break;

      case 5 :
      if (dState.pump1Power > 0){dState.pump1Power = 0;}
      else {dState.pump1Power = 100;}
      break;
    }
    buttonPressed = false;
  }
}

void setupEncoder(){
    pinMode(encoderA, INPUT_PULLUP);
    pinMode(encoderB, INPUT_PULLUP);
    pinMode(button, INPUT_PULLUP);
    runDebounce.start();
}

void attachInterrupts(){
    attachInterrupt(button, setButtonPressed, FALLING);
    attachInterrupt(encoderA, doEncoderA, CHANGE);
    attachInterrupt(encoderB, doEncoderB, CHANGE);
}

void debounce(){
    debounced = true;
}