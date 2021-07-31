/**
 * XtsTiExpander V2
 * 
 * storage stack
 * 
 * Xtase - fgalliat @Jul 2021
 */

#include <Arduino.h>
#include "globals.h"

#include "FS.h"
#include "SPIFFS.h"

// FIXME : do better
void Storage::format() {
    Serial.println("Formatting");
    SPIFFS.begin(true);
    Serial.println("done");
}

Storage::Storage() {}
Storage::~Storage() {}

bool Storage::begin() {
  if(! SPIFFS.begin(false) ){
      return false;
  }
  return true;
}

void Storage::end() {

}