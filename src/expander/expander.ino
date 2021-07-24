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

    bool setupOk = tilink.begin();

    if ( !setupOk ) {
        Serial.println("(!!) Setup may be incomplete");
    }
}

void loop() {

}
