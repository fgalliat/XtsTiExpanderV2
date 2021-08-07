/**
 * XTBase - Xtase language parser
 * 
 * Xtase - fgalliat @May 2020
 * 
 * small compiler/interpreter
 * 
 * looks like TI-Basic (for ti8x)
 * 
 */

#ifndef ARDUINO

#include "globals.h"

#include "xtbase.h"
XTBaseApp xtbase;


int main(int argc, char** argv) {
    int filelen = 0;

    char* scriptName = (char*)"script";
    if ( argc > 1 ) { scriptName = argv[1]; }

    char* script = xtbase.loadScript( scriptName, filelen );
    if ( script == NULL ) { xtbase.println("Wrong script file"); return 1; }

    xtbase.print("Read ");
    xtbase.print(filelen);
    xtbase.println(" bytes");

    bool compiled = xtbase.compile(script);
    free(script);

    if ( !compiled ) {
        xtbase.gc();
        xtbase.println("Failed to compile");
        return 1;
    }

    bool exec = xtbase.run();
    xtbase.gc();

    if ( !exec ) {
        xtbase.println("Execution failure");
        return 1;
    }

    return 0;
}

#endif