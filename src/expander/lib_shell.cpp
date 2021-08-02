/**
 * XtsTiExpander V2
 * 
 * Shell stack
 * 
 * Xtase - fgalliat @Aug 2021
 */

#include <Arduino.h>
#include "globals.h"

Shell::Shell() {
}

Shell::~Shell() {
}

// char* Shell::readLine(bool echo) {
//   return NULL;
// }

void Shell::end() {
    curClient->println("Goodbye");
    curClient = NULL;
    opened = false;
}

bool Shell::isOpened() {
    return opened;
}

void Shell::setEcho(bool _echo) {
    echo = _echo;
}

void Shell::resetCurLine() {
    memset(curline, 0x00, SHELL_LINE_MAX+1);
    curlineCursor = 0;
}

bool Shell::appendChar(char ch) {
    if ( curlineCursor >= SHELL_LINE_MAX ) { return false; }
    curline[curlineCursor++] = ch;
}

int Shell::containsBR() {
    if ( curlineCursor == 0 ) { return false; }
    for(int i=0; i < curlineCursor; i++) {
        if ( curline[i] == '\r' || curline[i] == '\n' ) {
            return i;
        }
    }
    return -1;
}

void Shell::begin(Stream* client) {
    curClient = client;
    opened = true;
    resetCurLine();
    curClient->println("Hello world");
}

bool Shell::loop() {
    if ( !opened ) { return false; }

    if ( curClient->available() <= 0 ) {
        return false;
    }

    bool canAppend = false;
    do {
        int ch = curClient->peek();
        canAppend = appendChar((char)ch);
        if ( !canAppend ) { break; }
        // curClient->print("<"); curClient->print(curline); curClient->print(">");
        curClient->read();
        if ( echo ) { curClient->write( ch ); }

        if ( containsBR() > -1 ) {
            // curClient->println("FOUND BR 1");
            break;
        }
    } while(curClient->available() > 0);

    int brPos = containsBR();
    if ( brPos > -1 ) {
        // curClient->println("FOUND BR 2");
        char cmdline[brPos+1];
        memcpy(cmdline, curline, brPos+1);
        cmdline[brPos] = 0x00;
        memmove(&curline[0], &curline[brPos+1], SHELL_LINE_MAX-brPos);
        curlineCursor -= brPos+1;

        // curClient->print("LEFT :"); curClient->println(cmdline);
        // curClient->print("RIGHT:"); curClient->println(curline);

        return handle(cmdline);
    }

    return false;
}

extern void shutdown();

bool Shell::handle(char* cmdline) {
    curClient->print("Ya asked : ");curClient->print(cmdline);curClient->println();

    if ( strncmp(cmdline, "quit", 4) == 0 ) {
        end();
        return true;
    } else if ( strncmp(cmdline, "ls", 2) == 0 ) {
        storage.lsToScreen();
        return true;
    } else if ( strncmp(cmdline, "screen", 6) == 0 ) {
        tilink.requestScreen(NULL, true);
        return true;
    } else if ( strncmp(cmdline, "halt", 4) == 0 ) {
        shutdown();
        return true;
    }

return false;
}