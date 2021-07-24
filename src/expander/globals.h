/**
 * XtsTiExpander V2
 * 
 * Xtase - fgalliat @Jul 2021
 */

#if MAIN_CODE
#endif

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