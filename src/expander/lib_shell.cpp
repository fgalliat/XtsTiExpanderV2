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

// blocking method
char* Shell::readLine(bool echo) {
    resetCurLine();
    bool canAppend = false;
    while(true) {
        while(curClient->available() <= 0) { delay(30); }
        while(curClient->available() > 0) {
            int ch = curClient->peek();

            if ( ch == 0x03 ) { // Ctrl-C
                return NULL;
            }

            canAppend = appendChar((char)ch);
            if ( !canAppend ) { break; }
            curClient->read();
            if ( echo ) { curClient->write( ch ); }

            if ( containsBR() > -1 ) {
                break;
            }

        }
        if ( containsBR() > -1 ) {
            break;
        }
    }
    if (curlineCursor > 0) {
        curline[ curlineCursor-1 ] = 0x00;
    }
    return curline;
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
    } else if ( strncmp(cmdline, "wifistart", 9) == 0 ) {
        netw.start();
    } else if ( strncmp(cmdline, "wifistop", 8) == 0 ) {
        netw.stop();
    } else if ( strncmp(cmdline, "wifidel", 7) == 0 ) {
        curClient->print("Sure to erase WiFi config ? ");
        char* resp = readLine(true);
        if ( resp != NULL && strncmp(resp, "y", 1) == 0 ) {
            resetCurLine();
            netw.eraseConfig();
            curClient->println("Erased");
            return true;
        }
        resetCurLine();
        return false;
    } else if ( strncmp(cmdline, "wifipsk", 7) == 0 ) {
        curClient->print("New SSID ? ");
        char* resp = readLine(true);
        if ( resp == NULL || strlen( resp ) == 0 ) {
            resetCurLine();
            curClient->print("Abord...");
            return false;
        }
        char newSSID[64+1]; sprintf(newSSID, "%s", resp);
        curClient->print("New PSK ? ");
        resp = readLine(true);
        if ( resp == NULL ) {
            resetCurLine();
            curClient->print("Abord...");
            return false;
        }
        char newPSK[64+1]; sprintf(newPSK, "%s", resp);
        resetCurLine();

        bool ok = netw.addConfig(newSSID, newPSK);
        if ( ok ) {
            curClient->println("New config added");
        } else {
            curClient->println("Failed to add config");
        }

        return true;
    }

return false;
}