/**
 * XTBase (Xtase May2020)
 * 
 * Jumps manipulation
 * 
 */ 

#include "../globals.h"
#include "../xtbase.h"

XTBJump* XTBaseApp::newJump() {
// printf("(ii) NEW JUMP\n");

    incHeapSize( sizeof(XTBJump*) );
    return new XTBJump();
}

XTBJump* XTBaseApp::pushGosub(XTBStatement* stmt) {
    if (xtb_jumpCursor >= XTB_MAX_JUMPS) {
        error("Jump Overflow",1);
        return NULL;
    }

    int addr = stmt->addr;
    int dest = stmt->dest;
    int type = XTB_JPTYPE_GOSUB;

    if ( xtb_jumps[ xtb_jumpCursor ] == NULL ) { xtb_jumps[ xtb_jumpCursor ] = newJump(); }
    xtb_jumps[ xtb_jumpCursor ]->from = addr;
    xtb_jumps[ xtb_jumpCursor ]->to   = dest;
    xtb_jumps[ xtb_jumpCursor ]->type = type;

    return xtb_jumps[ xtb_jumpCursor++ ];
}

XTBJump* XTBaseApp::popGosub() {
    if ( xtb_jumpCursor <= 0 ) {
        error("Jump Underflow",1);
        return NULL;
    }
    return xtb_jumps[ --xtb_jumpCursor ];
}

void XTBaseApp::cleanJumps() {
    int nbToClean = 0;
    for(int i=0; i < XTB_MAX_JUMPS; i++) {
        if ( xtb_jumps[i] != NULL ) { my_free(xtb_jumps[i]); xtb_jumps[i] = NULL; nbToClean++; }
    }
    xtb_jumpCursor = 0;
}
