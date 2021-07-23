/**
 * XtsTiExpander V2
 * 
 * Xtase - fgalliat @Jul 2021
 */

#define MAIN_CODE 1
#include "globals.h"

void setup() {
    Serial.begin(115200);

    bool setupOk = setup_tilink();

    if ( !setupOk ) {
        Serial.println("(!!) Setup may be incomplete");
    }
}

void loop() {

}
