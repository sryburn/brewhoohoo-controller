#ifndef CountdownTimer_h
#define CountdownTimer_h

class CountdownTimer 
{
  public:
    static void start();  //added static
    static void stop(); //added static
    static void reset();//added static
    static bool active();//added static
    void countdown();
    bool hasUpdated();
    static const char* getClockText();  //added static, removed trailing char

  private:
    //these are static as the countdown method is called from a separate timer instance
    static char clockText[20];
    static int decaminutes;
    static int minutes;
    static int decaseconds;
    static int seconds;
    static bool updated;
};
#endif