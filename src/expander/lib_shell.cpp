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
    if ( !opened ) { return; }
    curClient->println("Goodbye");
    curClient = NULL;
    opened = false;
    resetCurLine();
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
    end();
    curClient = client;
    opened = true;
    resetCurLine();
    setEcho(true);
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
        if ( curline[brPos+1] == '\n' || curline[brPos+1] == '\r' ) {
            // double BR ?
            brPos++;
        }
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
    } else if ( strncmp(cmdline, "/send", 5) == 0 ) {
        return handleVarRecv();
    } else if ( strncmp(cmdline, "hex ", 4) == 0 ) {
        char* varname = &cmdline[4];
        return hexVar(varname);
    } else if ( strncmp(cmdline, "del ", 4) == 0 ) {
        char* varname = &cmdline[4];
        return delVar(varname);
    }

    curClient->print("Ya asked : ");curClient->print(cmdline);curClient->println();

return false;
}

bool Shell::handleVarRecv() {
//   dummyStream.setLoopDisabled(true);
//   bool savedEcho = echo;
//   setEcho(false);

  // handle even Ctrl-C -- waits varName
  resetCurLine();
  char* resp = readLine(!true);

  #if HAS_DISPLAY
    scLandscape();
    scCls();
    tft.println("Var receiving");
  #else
  #endif

  if ( resp == NULL || strlen(resp) == 0 ) {
    // FIXME : warning
    curClient->println("/!\\ Empty varName.");
    #if HAS_DISPLAY
      tft.println("/!\\ Empty varName.");
      scRestore();
    #endif
    resetCurLine();
    // dummyStream.setLoopDisabled(false);
    // setEcho(savedEcho);
    return false;
  }
  char varName[8+1]; sprintf(varName, "%s", resp);
  resetCurLine();

  while( curClient->available() <= 0 ) { delay(5); }
  uint8_t varType = curClient->read();
  while( curClient->available() <= 0 ) { delay(5); }
  uint16_t varLength = (curClient->read() << 8) + curClient->read(); // 64K max
  while( curClient->available() <= 0 ) { delay(5); }
  bool varSendToTi = curClient->read() > 0; // auto send to Ti ?

  int error = 0;
  File f = storage.createTiFile(varName, varType, error);
  if ( error != 0 ) {
    // FIXME : warning
    curClient->println("/!\\ Failed to open file");
    #if HAS_DISPLAY
      tft.println("/!\\ Failed to open file");
      scRestore();
    #endif
    // dummyStream.setLoopDisabled(false);
    // setEcho(savedEcho);
    return false;
  }

  #if HAS_DISPLAY
  tft.print(F("ExpVarRecv >> name : ")); tft.println(varName); 
  tft.print(F("ExpVarRecv >> type : ")); tft.println(varType, HEX); 
  tft.print(F("ExpVarRecv >> varLength : ")); tft.println(varLength); 
  tft.print(F("ExpVarRecv >> sendToTi  : ")); tft.println(varSendToTi ? "1" : "0"); 
  #endif

  while( curClient->available() > 0 ) { curClient->read(); }

  curClient->write(0x01); // CTS
  while( curClient->available() <= 0 ) { delay(5); }
  curClient->read(); // ACK CTS

  const int blocLen = 128; uint8_t bloc[blocLen];
  uint16_t i=0; int read; int cpt = 0;

  #if HAS_DISPLAY
    scLowerJauge( 0 );
  #endif
  while(i < varLength) {
      memset( bloc, 0x00, blocLen );
      while( curClient->available() <= 0 ) { delay(2); }

      //int avail = min(blocLen, curClient->available());
      int avail = curClient->read(); // len to copy

      int tt = 0;
      while(tt < avail) {
        int read = curClient->readBytes(bloc, avail-tt);
        f.write(bloc, read);
        tt+=avail;
      }

      #if HAS_DISPLAY
        if ( cpt >= 3 ) {
          scLowerJauge( 100* ( i+avail ) / varLength );
          cpt = 0;
        }
      #endif

      curClient->write(0x01); // Handshake
      i+= avail;
      cpt++;
  }
  #if HAS_DISPLAY
    scLowerJauge( 100 );
  #endif

  #if HAS_DISPLAY
  tft.println(F("ExpVarRecv >> EOF "));
  #endif

  f.flush();
  f.close();

  if ( varSendToTi ) {
      delay(500);
      tilink.sendVar(varName);
  }

//   dummyStream.setLoopDisabled(false);
//   setEcho(savedEcho);

  return true;
}

bool Shell::delVar(char* varName) {
    storage.eraseTiFile(varName);
    return true;
}

bool Shell::hexVar(char* varName) {
    if ( varName == NULL || strlen(varName) <= 0 ) {
        return false;
    }
    char* filename = storage.findTiFile(varName);
    if ( filename == NULL ) {
        curClient->printf("Var %s was not found\n", varName);
        return false;
    }

    File f = storage.getTiFile(filename);
    if ( !f ) {
        curClient->printf("Var %s was not reachable\n", varName);
        return false;
    }

    int blocLen = 16; uint8_t bloc[blocLen];
    for(int i=0; i < f.size(); i+=blocLen) {
        int remaining = min(blocLen, f.available());
        f.readBytes( (char*)bloc, remaining );
        hexDump(bloc, remaining, true);
    }
    curClient->printf("-EOF- (%d bytes)\n", f.size());
    f.close();

    return true;
}

void Shell::hexDump(uint8_t* data, int dataLen, bool asciiToo) {
    for(int i=0; i < dataLen; i++) {
        if ( asciiToo ) {
            curClient->printf("%02X%c ", data[i], ( data[i] >= 32 && data[i] < 128 ? data[i] : '?' ) );
        } else {
            curClient->printf("%02X ", data[i] );
        }
    }
    curClient->println();
}