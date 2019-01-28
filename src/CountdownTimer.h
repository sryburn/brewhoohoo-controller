#ifndef CountdownTimer_h
#define CountdownTimer_h

class CountdownTimer 
{
  public:
    void start();
    void stop();
    void reset();
    bool active();
    void countdown();
    bool hasUpdated();
    const char* getClockText() const;  

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