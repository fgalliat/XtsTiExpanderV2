/**
 * XtsTiExpander V2
 * 
 * Xtase - fgalliat @Jul 2021
 */

#define MAIN_CODE 1
#include "globals.h"

TiLink tilink;

void setup() {
    Serial.begin(115200);

    bool setupOk = true;
    bool tilinkOk = tilink.begin();
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
}

void loop() {
    tilink.poll();

    if ( tilink.available() ) {
        Serial.write( tilink.read() );
    }
}
