/**
 * XtsTiExpander V2
 * 
 * Xtase - fgalliat @Jul 2021
 */

#define MAIN_CODE 1
#include "globals.h"

TiLink tilink;
Speaker speaker;
Storage storage;
Shell shell;
Network netw;

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


#if TTGO_TDISPLAY
 // FIXME : refactor
    
    #include <SPI.h>
    #include "WiFi.h"
    #include <Wire.h>
    #include "Button2.h"
    #include "esp_adc_cal.h"


// TFT Pins has been set in the TFT_eSPI library in the User Setup file TTGO_T_Display.h
// #define TFT_MOSI            19
// #define TFT_SCLK            18
// #define TFT_CS              5
// #define TFT_DC              16
// #define TFT_RST             23
// #define TFT_BL              4   // Display backlight control pin


#define ADC_EN              14  //ADC_EN is the ADC detection enable port
#define ADC_PIN             34
#define BUTTON_1            35
#define BUTTON_2            0

TFT_eSPI tft = TFT_eSPI(135, 240); // Invoke custom library
Button2 btn1(BUTTON_1);
Button2 btn2(BUTTON_2);

uint16_t ti_bgColor = TFT_GREENYELLOW;
uint16_t ti_fgColor = TFT_BLACK;



// char buff[512];
int vref = 1100;
int btnCick = false;

//! Long time delay, it is recommended to use shallow sleep, which can effectively reduce the current consumption
void espDelay(int ms)
{
    esp_sleep_enable_timer_wakeup(ms * 1000);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
    esp_light_sleep_start();
}

void showVoltage()
{
    static uint64_t timeStamp = 0;
    if (millis() - timeStamp > 1000) {
        timeStamp = millis();
        uint16_t v = analogRead(ADC_PIN);
        float battery_voltage = ((float)v / 4095.0) * 2.0 * 3.3 * (vref / 1000.0);
        String voltage = "Voltage :" + String(battery_voltage) + "V";
        Serial.println(voltage);
        tft.fillScreen(TFT_BLACK);
        tft.setTextDatum(MC_DATUM);
        tft.drawString(voltage,  tft.width() / 2, tft.height() / 2 );
    }
}

void shutdown() {
    int r = digitalRead(TFT_BL);
    digitalWrite(TFT_BL, !r);

    tft.writecommand(TFT_DISPOFF);
    tft.writecommand(TFT_SLPIN);
    //After using light sleep, you need to disable timer wake, because here use external IO port to wake up
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_TIMER);
    // esp_sleep_enable_ext1_wakeup(GPIO_SEL_35, ESP_EXT1_WAKEUP_ALL_LOW);
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_35, 0);
    delay(200);
    esp_deep_sleep_start();
}

void button_init()
{
    btn1.setLongClickHandler([](Button2 & b) {
        btnCick = false;
        int r = digitalRead(TFT_BL);
        tft.fillScreen(TFT_BLACK);
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        tft.setTextDatum(MC_DATUM);
        tft.drawString("Press again to wake up",  tft.width() / 2, tft.height() / 2 );
        espDelay(6000);
        shutdown();
    });
    btn1.setPressedHandler([](Button2 & b) {
        Serial.println("Detect Voltage..");
        btnCick = true;
    });

    btn2.setPressedHandler([](Button2 & b) {
        btnCick = false;
        Serial.println("btn press wifi scan");
        // wifi_scan();
        tilink.requestScreen(NULL, true);
        speaker.tone(440, 50);
    });
}

void button_loop()
{
    btn1.loop();
    btn2.loop();
}

void setupHardware() {
    /*
    ADC_EN is the ADC detection enable port
    If the USB port is used for power supply, it is turned on by default.
    If it is powered by battery, it needs to be set to high level
    */
    pinMode(ADC_EN, OUTPUT);
    digitalWrite(ADC_EN, HIGH);

    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    tft.setTextSize(2);
    tft.setTextColor(TFT_GREEN);
    tft.setCursor(0, 0);
    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(1);

    tft.setRotation(0);
    tft.fillScreen(TFT_RED);

    button_init();

    esp_adc_cal_characteristics_t adc_chars;
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars);    //Check type of calibration value used to characterize ADC
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        Serial.printf("eFuse Vref:%u mV", adc_chars.vref);
        vref = adc_chars.vref;
    } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
        Serial.printf("Two Point --> coeff_a:%umV coeff_b:%umV\n", adc_chars.coeff_a, adc_chars.coeff_b);
    } else {
        Serial.println("Default Vref: 1100mV");
    }


    tft.fillScreen(TFT_BLACK);
    tft.setTextDatum(MC_DATUM);

    tft.drawString("LeftButton:", tft.width() / 2, tft.height() / 2 - 16);
    tft.drawString("[Screen Dump]", tft.width() / 2, tft.height() / 2 );
    tft.drawString("RightButton:", tft.width() / 2, tft.height() / 2 + 16);
    tft.drawString("[Voltage Monitor]", tft.width() / 2, tft.height() / 2 + 32 );
    tft.drawString("RightButtonLongPress:", tft.width() / 2, tft.height() / 2 + 48);
    tft.drawString("[Deep Sleep]", tft.width() / 2, tft.height() / 2 + 64 );
    tft.setTextDatum(TL_DATUM);

}

void ttgo_loop()
{
    if (btnCick) {
        showVoltage();
    }
    button_loop();
}

#endif

// =========== HiLevel ti handle ==========
char tiActionValue[64];

void tiAction(char* request);

void handleTiActionData(uint8_t* segment, int segLen, uint32_t count, uint32_t total) {
    // now : segLen is 64 max
    if ( count > 0 ) {
        // FIXME : ignore > to 64 tiaction
        return;
    }

    // 0x00 <content> 0x00 .....
    char* value = (char*)&segment[1];
    int len = strlen(value);
    if ( len <= 0 ) {
        // beware when multipacket -> will not contains trailling 0x00 in this packet...
        return;
    }
    
    sprintf(tiActionValue, "%s", value);
}

void handleTiActionFlush() {
    if ( strlen( tiActionValue ) > 0 ) {
        tiAction(tiActionValue);
    }
}

void tiAction(char* action) {
  if ( strncmp("play:", action, 5) == 0 ) {
      char* tune = &action[5];
      speaker.playTuneString( tune );
  } else if ( strncmp("get:", action, 4) == 0 ) {
      char* varName = &action[4];
      tilink.sendVar( varName );
  } else if ( strncmp("list:", action, 5) == 0 ) {
      char* filter = &action[5]; // unused for now
      #if HAS_DISPLAY
        storage.lsToScreen();
      #endif
  } else if ( strncmp("wifi:start", action, 10) == 0 ) {
      #if HAS_DISPLAY
        scLandscape();
        scCls();
        tft.println( "Wifi connect" );
        tft.println( "" );
        scRestore();
      #endif
      netw.start();
      #if HAS_DISPLAY
        scLandscape();
        tft.println( netw.getIP() );
        tft.println( netw.getNetname() );
        scRestore();
      #endif
  } else if ( strncmp("wifi:stop", action, 9) == 0 ) {
      netw.stop();
  } else {
      Serial.println("Unknown action");
  }
}

// ============= Arduino ================

void setup() {
    Serial.begin(115200);
    speaker.begin();

    #if TTGO_TDISPLAY
        setupHardware();
    #endif


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

    bool storageSetup = storage.begin();
    if ( !storageSetup ) {
        Serial.println("(!!) Storage Setup failed");
        Serial.println("(!!) Try 'espformater' code");
    }
    setupOk &= storageSetup;

    if ( !setupOk ) {
        Serial.println("(!!) Setup may be incomplete");
        #if HAS_DISPLAY
          tft.println("(!!) Setup may be incomplete");
        #endif
    }

   #if NO_ISR
     setPollMode(false);
   #else
     installISR(ISR_DURATION); // need 16ms to read a byte
   #endif

   #if HAS_DISPLAY
     storage.lsToScreen();
   #endif
}

long lastTime = millis();

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

    #if TTGO_TDISPLAY
      ttgo_loop();
    #endif

#if NO_ISR
    tilink.handleCalc();
#else
    tilink.handleCalc();
#endif

    netw.loop();
    shell.loop();

    if ( Serial.available() ) {
        int b = Serial.read();
        if ( b == 0x03 ) { // Ctrl-C
            tilink.requestScreen(&Serial, true);
        } else if ( b == 0x04 ) { // Ctrl-D
            tilink.sendKeyStroke( KEYCODE_CLEAR );
            tilink.sendKeyStrokes( "Hello world !" );
        } else if ( b == 0x05 ) { // Ctrl-E
            storage.lsToStream(&Serial, SHELL_MODE_SERIAL);
        } else if ( b == 0x06 ) { // Ctrl-F
            tilink.sendVar("keyb");
        } else if ( b == 0x07 ) { // Ctrl-G
            shell.begin(&Serial);
        } else {
            tilink.write(b);
        }
    }

   #if HAS_DISPLAY
    // Power Jauge
    if ( millis() - lastTime > 1000 ) {
        uint16_t v = analogRead(ADC_PIN);
        float battery_voltage = ((float)v / 4095.0) * 2.0 * 3.3 * (vref / 1000.0);
        if (battery_voltage > 5.0) { battery_voltage = 5.0; }

        scLandscape();
        scPowerJauge( battery_voltage );
        scRestore();
        lastTime = millis();
    }
  #endif

}
