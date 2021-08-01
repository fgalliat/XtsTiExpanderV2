/**
 * XtsTiExpander V2
 * 
 * Xtase - fgalliat @Jul 2021
 */

#if MAIN_CODE
#endif

// CLEAR from TI92 manual p.484 (0x0107 > 263)
#define KEYCODE_CLEAR 0x0107
#define KEYCODE_ENTER 0x0D

// because requires ~16ms max to read a byte from TI
#define ISR_DURATION 20

#include "utils.h"

#include "lib_network.h"
#include "lib_storage.h"
#include "lib_tilink.h"

#define TTGO_TDISPLAY 1

#if TTGO_TDISPLAY
    #include <TFT_eSPI.h>

    // no 2, 12
    // can write but not read
    // #define TI_TIP 13
    // #define TI_RING 17

    // I was not able to use others pins from that board ..
    // works in non pollMode -- re-check Wiring (red Vs white)
    #define TI_TIP 22
    #define TI_RING 21

    #if not MAIN_CODE
        extern TFT_eSPI tft;
        extern uint16_t ti_bgColor;
        extern uint16_t ti_fgColor;
    #endif

    #define HAS_DISPLAY 1
#else
    // tested on ESP32 DevKit r1 
    // that works !! -- re-check Wiring (red Vs white)
    #define TI_TIP 19
    #define TI_RING 18

    #define HAS_DISPLAY 0
#endif

#include "lib_display.h"
#include "lib_sound.h"

#if not MAIN_CODE
  extern Speaker speaker;
  extern Storage storage;
#endif

#define SHELL_MODE_TELNET 0x01
#define SHELL_MODE_SERIAL 0x02
#define SHELL_MODE_DUMMY  0x03

static bool startsWith(char* strFull, char* strToFind) {
    if ( strToFind == NULL || strFull == NULL ) { return false; }
    if ( strlen( strFull ) < strlen( strToFind ) ) { return false; }
    return strncmp( strFull, strToFind, strlen( strToFind ) ) == 0;
}

// 0C -> 12
static int hexStrToInt(char* str) {
    int hexNumber;
    sscanf(str, "%02X", &hexNumber);
    return hexNumber;
}