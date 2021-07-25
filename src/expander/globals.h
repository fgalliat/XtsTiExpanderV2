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

// weird chars
// #define TI_TIP 32
// #define TI_RING 33
// #define TI_TIP 33
// #define TI_RING 32

// write fail
// #define TI_TIP 35
// #define TI_RING 34

// weird chars - but no failure
// #define TI_TIP 18
// #define TI_RING 19

// that works !! -- re-check Wiring
#define TI_TIP 19
#define TI_RING 18