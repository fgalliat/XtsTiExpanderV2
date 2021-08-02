/**
 * XtsTiExpander V2
 * 
 * Display stack
 * 
 * Xtase - fgalliat @Jul 2021
 */

#include <Arduino.h>
#include "globals.h"

int rot = 0;

void scCls() { tft.fillRect(0,0,tft.width(), tft.height(), ti_bgColor); tft.setCursor(0,0); tft.setTextColor(ti_fgColor); }

void scLandscape() {
 if (tft.getRotation() == 1) { return; }
 rot = tft.getRotation();
 tft.setRotation(1);  
}

void scRestore() {
  tft.setRotation(rot);
}

void scLowerJauge(int percent) {
    if ( percent > 100 ) { percent = 100; }
    int xx = 20; // (240 - 2 * 100 ) / 2
    int yy = 100;
    int ww = percent * 2;
    int www = (100 * 2) + 2 + 2;
    int hh = 2;
    int hhh = hh + 2 + 2;
    tft.fillRect(xx-4,yy, www, hhh, ti_bgColor);
    tft.drawRoundRect(xx-4, yy, www, hhh, 3, ti_fgColor);
    tft.fillRect(xx, yy+1+1, ww, hh, ti_fgColor);
}

// long map(long x, long in_min, long in_max, long out_min, long out_max) {
//   return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
// }

long voltsToLong(float v) {
  return (long)( v * 100.0 );
}

void scPowerJauge(float voltage) {
  int percent = (int)map( voltsToLong(voltage), voltsToLong(3.0), voltsToLong(5.0), 0, 100 );
  if ( percent > 100 ) { percent = 100; }

  int wMax = 30;
  int www = wMax + 2 + 2;
  int ww = wMax * percent / 100;
  int xx = 240 - www - 5;
  int yy = 0;
  int hh = 2;
  int hhh = hh + 2 + 2;

  tft.fillRect(xx-4,yy, www, hhh, ti_bgColor);
  tft.drawRoundRect(xx-4, yy, www, hhh, 3, ti_fgColor);
  tft.fillRect(xx, yy+1+1, ww, hh, ti_fgColor);
}