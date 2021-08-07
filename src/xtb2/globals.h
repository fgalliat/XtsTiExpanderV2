#ifndef __GLOBALS__H_
#define __GLOBALS__H_ 1

#define ABSTRACT_TTY 1
#define ABSTRACT_M5 2

#ifndef ARDUINO
    #include "stdio.h"
    #include "string.h"
    #include "stdlib.h"
    #include "stdint.h"

    static int min(int a, int b) { return a < b ? a : b; }
    static void yield() { ; }

    #define INTERACTIVE_MODE 1
    #define HAS_MENU 0

    #define ABSTRACT_MODE ABSTRACT_TTY

#else
    #include <Arduino.h>
    #define INTERACTIVE_MODE 1
    #define HAS_MENU 1

    #define ABSTRACT_MODE ABSTRACT_M5
#endif

// #define FGCOLOR WHITE
// #define FGCOLOR GREEN
// orange plasma
#define FGCOLOR color(244, 219, 194)
#define BGCOLOR BLACK



#endif