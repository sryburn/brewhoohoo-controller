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