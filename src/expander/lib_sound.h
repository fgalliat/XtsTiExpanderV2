/**
 * XtsTiExpander V2
 * 
 * sound stack
 * 
 * Xtase - fgalliat @Jul 2021
 */

#define BUZ_PIN 25
#define BUZ_channel 0
#define BUZ_resolution 8

class Speaker {
  private:
    bool _mute = false;
  public:
    Speaker();
    ~Speaker();
    void begin();
    void end();

    void noTone();
    void tone(int freq);
    void tone(int freq, long duration);
    void playNote(int noteOrFreq, int duration);
    void playTuneString(const char* strTune);
};

