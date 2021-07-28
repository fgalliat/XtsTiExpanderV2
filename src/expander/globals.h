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

#include "lib_network.h"
#include "lib_storage.h"
#include "lib_tilink.h"

#define TTGO_TDISPLAY 0

// that works !! -- re-check Wiring
#define TI_TIP 19
#define TI_RING 18