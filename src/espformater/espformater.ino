/**
 * Just to pre-format ESP32 SPIFFS
 * Xtase fgalliat @Jul 2021
 */

#include "FS.h"
#include "SPIFFS.h"

#define HAS_DISPLAY 1

#if HAS_DISPLAY
#include <SPI.h>
#include <TFT_eSPI.h>
TFT_eSPI tft = TFT_eSPI(135, 240); // Invoke custom library
#endif


void setup() {
  Serial.begin(115200);

#if HAS_DISPLAY
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    tft.setTextSize(2);
    tft.setTextColor(TFT_GREEN);
    tft.setCursor(0, 0);
    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(1);
#endif

  if(! SPIFFS.begin(false) ){
      Serial.println("SPIFFS Mount Failed");
      tft.println("SPIFFS Mount Failed");

      Serial.println("Need to format it !");
      tft.println("Need to format it !");
      Serial.println("format in progress !");
      tft.println("format in progress !");
      if(! SPIFFS.begin(true) ){
          Serial.println("SPIFFS Mount Failed (2)");
          Serial.println("You have a problem ...");
          tft.println("SPIFFS Mount Failed (2)");
          tft.println("You have a problem ...");
      }

  }

  Serial.println("SPIFFS Mounted Successfully !");
  tft.println("SPIFFS Mounted Successfully !");


}

void loop() {
  delay(1000);
}