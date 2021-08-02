/**
 * XtsTiExpander V2
 * 
 * Display stack
 * 
 * Xtase - fgalliat @Jul 2021
 */

// FIXME : refactor
void scCls();
void scLandscape();
void scRestore();
void scLowerJauge(int percent);
void scPowerJauge(float voltage);

class Display {
  public:
    Display();
    ~Display();

   void readColorScheme();
   void setColorScheme(uint16_t fg, uint16_t bg);
   void swapColorScheme();

   void warning(char* text);
};