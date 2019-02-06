#ifndef Touchscreen_h
#define Touchscreen_h
#include "NextionBasic.h"
#include "State.h"

extern DeviceState dState;

class Touchscreen 
{
  public:
  void attachCallbacks();
  void checkForTouchEvents();
  void updateChart(int channel, int value);
  void updateClock(const char *clock);
  void updateText(int id, int value);
  void updateText(int id, double value);
  void updateText(int id, const char *string);
  static void terminateCommand();
  static void sendCommand(const char *command);
  static void boilPushCallback(void *ptr);
  static void boilPopCallback(void *ptr);
  static void mashPushCallback(void *ptr);
  static void hltPushCallback(void *ptr);
  static void pump1PushCallback(void *ptr);
  static void pump2PushCallback(void *ptr);
  static void playPushCallback(void *ptr);
  static void restartPushCallback(void *ptr);
  static void restartPopCallback(void *ptr);


  private:
  bool timeToPublishWaveform;
  static void resetMode();
  static int prevMode;

};

#endif