/**
 * XtsTiExpander V2
 * 
 * Xtase - fgalliat @Jul 2021
 */

#define MAIN_CODE 1
#include "globals.h"

TiLink tilink;

// ================================================
// Specific ESP32 Timer
volatile bool ISRLOCKED = false;
volatile bool pollMode = true;

hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

// volatile int interruptCounter;

void IRAM_ATTR onTimer() {
  portENTER_CRITICAL_ISR(&timerMux);

  if ( !ISRLOCKED ) { 
    // interruptCounter++;
    
    // if takes long more than ISR frequency -> Exception
    // (Guru Meditation Error: Core  1 panic'ed (Interrupt wdt timeout on CPU1)
    if (pollMode) tilink.poll(true);
  }
  ISRLOCKED = false; // auto release

  portEXIT_CRITICAL_ISR(&timerMux);
}

void setPollMode(bool state) {
  pollMode = state;
}

bool isPollMode() {
  return pollMode;
}

void lockISR() {
    if ( ISRLOCKED ) {
        return;
    }
    ISRLOCKED = true;
    if ( isPollMode() ) delay(ISR_DURATION); // ensure exit from ISR
}

void installISR(int msec) {
    // ------------------------------
    // Specific ESP32 Timer
    timer = timerBegin(0, 80, true);
    timerAttachInterrupt(timer, &onTimer, true);
    timerAlarmWrite(timer, msec * 1000, true);
    timerAlarmEnable(timer);
    // ------------------------------
    ISRLOCKED = false;
}
// ================================================

#define NO_ISR 1

void setup() {
    Serial.begin(115200);

    bool setupOk = true;
    bool tilinkOk = tilink.begin(TI_TIP, TI_RING);
    if ( !tilinkOk ) {
        Serial.println("(!!) TiLink Setup failed");
        Serial.println("(!!) this is the main feature -> STOP");
        while(true) {
            delay(1000);
        }
    }
    setupOk &= tilinkOk;

    if ( !setupOk ) {
        Serial.println("(!!) Setup may be incomplete");
    }

   #if NO_ISR
     setPollMode(false);
   #else
     installISR(ISR_DURATION); // need 16ms to read a byte
   #endif
}

void loop() {
    // tilink.poll();

    // if ( tilink.available() ) {
    //     int b = tilink.read();
    //     if ( b < ' ' || b > 128 ) {
    //         Serial.write( 'x' );
    //         Serial.print( b, HEX );
    //         Serial.write( ' ' );
    //     } else {
    //         Serial.write( b );
    //     }
    //     tilink.resetLines();
    // }
#if NO_ISR
    tilink.handleCalc();
#else
    tilink.handleCalc();
#endif

    if ( Serial.available() ) {
        int b = Serial.read();
        if ( b == 0x03 ) { // Ctrl-C
            tilink.requestScreen(&Serial, true);
        } else if ( b == 0x04 ) { // Ctrl-D
            tilink.sendKeyStroke( KEYCODE_CLEAR );
            tilink.sendKeyStrokes( "Hello world !" );
        }
        tilink.write(b);
    }

}
