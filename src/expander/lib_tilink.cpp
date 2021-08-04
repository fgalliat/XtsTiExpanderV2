/**
 * XtsTiExpander V2
 * 
 * ti-link stack
 * 
 * Xtase - fgalliat @Jul 2021
 */

#include <Arduino.h>
#include "globals.h"
#include "hw_tilink.h"

extern bool isPollMode();
extern void setPollMode(bool state);

extern void handleTiActionData(uint8_t* segment, int segLen, uint32_t count, uint32_t total);
extern void handleTiActionFlush();

uint8_t inputBuff[INPUT_BUFF_LEN+1];
int inputBuffCursor = 0;

TiLink::TiLink() {
}

TiLink::~TiLink() {
}

bool TiLink::begin(int tip, int ring) {
  return setup_tilink(tip, ring);
}

void TiLink::end() {
}

void TiLink::resetLines(bool restoreLines) {
  __resetTILines(restoreLines);
}

void pushToBuffer(uint8_t b) {
  // Serial.print('>');Serial.print(b, HEX);Serial.println('<');
  inputBuff[ inputBuffCursor++ ] = b;
}

int consumeBuffer() {
  if ( inputBuffCursor == 0 ) {
    return -1;
  }
  // int bte = inputBuff[inputBuffCursor-1];
  int bte = (int)inputBuff[0];
  for(int i=0; i < inputBuffCursor; i++) {
    inputBuff[i] = inputBuff[i+1];
  }
  inputBuff[inputBuffCursor] = 0x00;
  //inputBuff[inputBuffCursor-1] = 0x00;
  inputBuffCursor--;
  return bte;
}

void TiLink::poll(bool isrMode) {
  if ( inputBuffCursor >= INPUT_BUFF_LEN ) {
    return;
  }

  int result = ti_read(2,2);
  if ( result < 0 ) {
    return;
  }
  uint8_t bte = (uint8_t)result;
  pushToBuffer(bte);
}

int TiLink::available() {
  return inputBuffCursor;
}
int TiLink::read() {
  if ( !isPollMode() ) {
    return ti_read();
  }

  return consumeBuffer();
}

int TiLink::peek() {
  return inputBuffCursor > 0 ? inputBuff[inputBuffCursor-1]: -1;
}

void TiLink::flush() {
}

size_t TiLink::write(uint8_t c) {
  int result = ti_write(c);
  if ( result < 0 ) {
    Serial.println("WRITEFAIL");
  }
  return result >= 0 ? 1 : 0;
}


bool TiLink::requestScreen(Stream* out, bool asciiMode) {
  return ti_reqScreen(out, asciiMode);
}

int TiLink::sendKeyStrokes(char* data, int len) { return ti_sendKeyStrokes(data, len); }
int TiLink::sendKeyStroke(int keyCode) { return ti_sendKeyStroke(keyCode); }

extern uint8_t recv[];
extern uint8_t screen[];
#define TMP_RAM screen

char intValue[10];

#define ASCII_OUTPUT 1
#define DBUG_DUMMY true

// @Deprecated
void TiLink::dummyMode() {
  Stream* term = &Serial;

term->println("DUMMY mode impl. no more Used !");

  // bool savedPollMode = isPollMode();
  // setPollMode(false); // disable polling
  // delay(ISR_DURATION * 2);

  // int recvNb;
  // #if ASCII_OUTPUT
  //   if (DBUG_DUMMY) term->println(F("DUMMY"));
  // #else
  //   term->print(F(OUT_BIN_ENTER_DUMMY));
  // #endif

  // term->setTimeout(400);

  // resetLines();
  

  //     // see : https://internetofhomethings.com/homethings/?p=927
  //     const int MAX_READ_LEN = 32;
  //     int bytesSentToTi = 0;

  //     memset(screen, 0x00, MAX_READ_LEN+1);
  //     int fullPacketLen = 0;

  //     int toRead;
  //     int cpt=0;
  //     int kc;

  //     char chs[2] = { 0x00, 0x00 };

  //     char msg[30];

  //     while(true) {

  //       while ( term->available() > 0 ) {
  //         // int t = min( MAX_READ_LEN, Serial.available() ); // read only what's available 
  //         // int read = serPort.readBytes( screen, t );
  //         // if ( read <= 0 ) { break; }
  //         // ti_send( screen, read );
  //         // delay( 15 * read ); // wait for XtsTerm ASM Ti

  //         int avail = term->available();
  //         int max = min( MAX_READ_LEN, avail ); // read only what's available 

  //         if ( bytesSentToTi + max > MAX_READ_LEN ) {
  //           max = MAX_READ_LEN - bytesSentToTi;
  //         }

  //         int read = term->readBytes( TMP_RAM, max );
  //         if ( read <= 0 ) { break; }
  //         ti_write( (uint8_t*)TMP_RAM, read );

  //         bytesSentToTi += read;
  //         // internal HandShake System -- BEWARE : no more compatible w/ (devel4ti/test_gcc4ti_1/main.c)
  //         if ( bytesSentToTi == MAX_READ_LEN ) {
  //           int hndTimeout = 0;
  //           recv[0] = 0x00;
  //           while( readBytes(recv, 1) <= 0 && (char)recv[0] != 'h' ) { 
  //             delay(2);
  //             hndTimeout++;
  //             if ( hndTimeout > 2000 ) { break; } // prevent from inf. loop
  //           } // waits for Ti-handshake

  //           chs[0] = 'H';
  //           // ti_send( chs, 1 ); // waits for Pc-Handshake
  //           write(chs[0]);
  //           bytesSentToTi = 0;
  //         }

  //       }

  //       if ( savedPollMode ) {
  //         // waitAvailable(200);
  //         recvNb = readBytes(recv, 2);
  //       } else {
  //         recvNb = ti_recv(recv, 2, false, 130);
  //       }
  //       if ( recvNb > 0 ) {
  //         if ( recv[0] == 'X' && recv[1] == ':' ) {
  //           waitAvailable(200);
  //           recvNb = readBytes(recv, 4+1);
  //           if ( recvNb > 0 && recv[0] == '?' && recv[1] == 'e' ) {
  //             // X:?end\n -> end of serial session
  //             #if ASCII_OUTPUT
  //               if (DBUG_DUMMY) term->println(F("EXIT DUMMY"));
  //             #else
  //               term->print(F(OUT_BIN_EXIT_DUMMY));
  //             #endif
  //             // reboot();
  //             setPollMode(savedPollMode);
  //             return;
  //           } 
  //         } else if ( recv[0] == 'K' && recv[1] == ':' ) {
  //             //K:<code>\n

  //             // found key 97       a
  //             // found key 56       num
  //             // found key 338      arrow
  //             // found key 344      arrow
  //             // found key 264      Esc
  //             // found key 4360     2nd + Esc => Quit
  //             // found key 257      backspace

  //             // waitAvailable(200);
  //             if ( isPollMode() ) {
  //               long curTime = millis();
  //               while( available() < 5 ) {
  //                 delay(30);
  //                 if ( millis() - curTime > 200 ) { break; }
  //               }
  //             }
  //             int toRead = 8;
  //             if ( isPollMode() ) { 
  //               toRead = available(); 
  //               readBytes(recv, toRead );
  //             } else {
  //               ti_recv(recv, toRead, false, true);
  //             }
  //             memset(intValue, 0x00, 10);
  //             for(int i=0; i < toRead; i++) {
  //               if ( recv[i] == '\n' ) { intValue[i] = 0x00; break; }
  //               intValue[i] = recv[i];
  //             }
  //             kc = atoi( intValue );
  //             //Serial.print(F("found key [")); Serial.print(kc); Serial.print(F("] (")); Serial.print( (char)kc ); Serial.print(F(") \n"));
  //             if ( kc == 264 ) { 
  //               // Esc
  //               kc = 27; 
  //               term->write( kc );
  //             }
  //             else if ( kc == 13 ) { 
  //               // Enter
  //               kc = 10; 
  //               term->write( kc );
  //             }
  //             else if ( kc == 257 ) { 
  //               // BackSpace
  //               term->write( 8 );
  //               term->write( 32 );
  //               term->write( 8 );
  //             }
  //             else if ( kc >= 337 && kc <= 348 ) {
  //               // Arrows key
  //               if ( kc == 338 ) { // UP
  //                 term->write( 27 );
  //                 term->print(F("[A"));
  //               } else if ( kc == 344 ) { // DOWN
  //                 term->write( 27 );
  //                 term->print(F("[B"));
  //               } else if ( kc == 337 ) { // LEFT
  //                 term->write( 27 );
  //                 term->print(F("[D"));
  //               } else if ( kc == 340 ) { // RIGHT
  //                 term->write( 27 );
  //                 term->print(F("[C"));
  //               } 
  //             }
  //             else if ( kc == 4360 ) {
  //               // 2nd + Quit
  //               // - dirty trap -
  //               #if ASCII_OUTPUT
  //                 if (DBUG_DUMMY) term->println(F("EXIT DUMMY"));
  //               #else
  //                 term->print(F(OUT_BIN_EXIT_DUMMY));
  //               #endif
  //               readBytes(recv, 2+4+1);
  //               // reboot();
  //               setPollMode(savedPollMode);
  //               return;
  //             }
  //             else if (kc > 0 && kc < 256) {
  //               term->write( kc );
  //             } else {
  //               term->print(">K:");
  //               term->println( intValue );
  //             }
  //         } // end of key read

  //       } // end if recvNb == 0
        
  //       //delay(5);
  //     } // end while dummy

  //     #if ASCII_OUTPUT
  //       if (DBUG_DUMMY) term->println(F("LEAVING DUMMY MODE"));
  //     #else
  //       term->print(F(OUT_BIN_EXIT_DUMMY));
  //     #endif


  // setPollMode(savedPollMode);
}

void TiLink::enterDummyMode() {
  Stream* curClient = &Serial; // FIXME
  inDummyMode = true;
  dummyStream.begin();
  curClient->println("Entry DUMMY");
}
void TiLink::exitDummyMode() {
  Stream* curClient = &Serial; // FIXME
  dummyStream.end();
  inDummyMode = false;
  curClient->println("Exit DUMMY");
}

bool TiLink::loopDummyMode() {
  // FIXME : assume pollMode == false
  bool savedPollMode = isPollMode();

  // Stream curClient = &Serial; // FIXME
  // while(curClient->available() > 0) {
  //   // FIXME : overflow / handshake
  //   int ch = curClient.read();
  //   dummyStream->write(ch);
  // }

  uint8_t recv[16];
  int recvNb = ti_recv(recv, 2, false, 80);
  if ( recvNb == 0 ) { return false; }

  if ( recvNb == 2 ) {
    if ( recv[0] == 'X' && recv[1] == ':' ) {
      // waitAvailable(200);
      recvNb = ti_recv(recv, 4+1);
      if ( recvNb > 0 && recv[0] == '?' && recv[1] == 'e' ) {
        // X:?end\n -> end of serial session
        // #if ASCII_OUTPUT
        //   if (DBUG_DUMMY) term->println(F("EXIT DUMMY"));
        // #else
        //   term->print(F(OUT_BIN_EXIT_DUMMY));
        // #endif
        // // reboot();
        exitDummyMode();
        setPollMode(savedPollMode);
        return true;
      } 
    } else if ( recv[0] == 'K' && recv[1] == ':' ) {
      //K:<code>\n

      // found key 97       a
      // found key 56       num
      // found key 338      arrow
      // found key 344      arrow
      // found key 264      Esc
      // found key 4360     2nd + Esc => Quit
      // found key 257      backspace
      int toRead = 8;
      ti_recv(recv, toRead, false, 300);
      memset(intValue, 0x00, 10);
      for(int i=0; i < toRead; i++) {
        if ( recv[i] == '\n' ) { intValue[i] = 0x00; break; }
        intValue[i] = recv[i];
      }
      int kc = atoi( intValue );

      if ( kc == 264 ) { kc = 27; } // Esc
      else if ( kc == 13 ) { kc = 10; } // Enter
      else if ( kc == 257 ) { kc = 8; } // BckSp
      else if ( kc == 4360 ) { kc = 3; exitDummyMode(); } // 2nd+QUIT

      dummyStream.appendToInputBuffer((char)kc);
      setPollMode(savedPollMode);
      return true;
    }
  }

  setPollMode(savedPollMode);
  return false;
}


// to refactor ======================
#define CBL_92 0x19
#define CALC_92 0x89
#define ACK REP_OK
#define CTS 0x09

void CBL_ACK() {
  static uint8_t E[4] = { CBL_92, ACK, 0x00, 0x00 };
  int ok = ti_write(E, 4); // ...
  if ( ok <= 0 ) { Serial.print(F("(!!) fail sending CBL-ACK : ")); Serial.println(ok); }
  delay(3);
}

void CBL_CTS() {
  static uint8_t E[4] = { CBL_92, CTS, 0x00, 0x00 };
  int ok = ti_write(E, 4); // ...
  if ( ok <= 0 ) { Serial.println(F("(!!) fail sending CBL-CTS")); }
  delay(3);
}
// to refactor ======================

void debugDatas(uint8_t* data, int len) {
  for (int i=0; i < len; i++) {
    uint8_t b = data[i];
    if ( b < 16 ) { Serial.write('0'); }
    Serial.print( b, HEX ); 
    if ( b < 32 || b > 128 ) { Serial.write( '?' ); }
    else { Serial.write(b); }
    Serial.write( ' ' );
  }
  Serial.println();
}

bool TiLink::waitAvailable(long timeout) {
  if ( !isPollMode() ) {
    return true; // beware with that
  }

  long curTime = millis();
  while( available() <= 0 ) {
    delay(30);
    if ( millis() - curTime > timeout ) {
      return false;
    }
  }
  return true;
}

bool TiLink::handleCalc() {
  if ( inDummyMode ) {
    return loopDummyMode();
  }


  bool savedPollMode = isPollMode();

  if ( savedPollMode && available() <= 0 ) { return false; }

  int recvNb;

  // Serial.println(available());

  // FIXME : better -- Cf further read() single byte
  if ( savedPollMode && available() < 2 ) { return false; }

  if ( !savedPollMode || available() >= 2 ) {

    if ( savedPollMode ) {
      recvNb = readBytes(recv, 2);
    } else {
      int byte0 = ti_read(10,10);
      if ( byte0 < 0 ) {
        return false;
      }
      int byte1 = ti_read();
      recv[0] = (uint8_t)byte0;
      recv[1] = (uint8_t)byte1;
      recvNb = 2;
    }

    // ASM version PRGM starts - direct bytes
    if ( recvNb == 2 && recv[0] == 'X' && recv[1] == ':' ) {
      waitAvailable(200);
      recvNb = readBytes(recv, 7);
      if ( recvNb == 7 && recv[0] == '?' && recv[1] == 'b' ) {
        // X:?begin\n
        // dummy serial mode : XtsTerm.92p
        //dummyMode();
        enterDummyMode();
      }
    } else if ( recvNb == 2 ) { // any other content
      if ( isPollMode() ) {
        setPollMode(false); // disable polling
        delay(ISR_DURATION * 2);
      }

      #define DBG_CBL 0

      // HERE : the Ti sends something by itself (CBL, Var)
      bool cblSend = false;
      bool varSend1 = false; // Garbage ? Sending Packet
      bool varSend2 = false;

      if ( recv[0] == 0x89 && recv[1] == 0x06 ) {
        cblSend = true;  // Ti sends to "CBL"
      } else if ( recv[0] == 0x89 && recv[1] == 0x68 ) {
        varSend1 = true; // Ti send 1st time (garbage ?)
      } else if ( recv[0] == 0x88 && recv[1] == 0x06 ) {
        varSend2 = true; // Ti send a Var (manual mode)
      } else {
        Serial.print(F("E:Not a KNOWN Header : "));
        debugDatas( recv, 2 );
      }

      // 07 & 08 for CBL data - 0 whwn Ti Sends a Var
      uint8_t headLength[1];
      waitAvailable(100);
      readBytes( headLength, 1 );
      #if DBG_CBL
        Serial.print(F("i:Len to read : "));Serial.println(headLength[0], HEX);
      #endif

      // 7 is 11 for ti92
      // 8 is 12 for ti voyage 200
      int head = ( headLength[0] + 4 ) -1; // -1 for head len

      if ( varSend1 ) {
        // 1st step only
        // just a try ==> don't know why -- TiVoyage200
        // have to read : 0 55 1 55 1 53 2 65 0 1 1 80 7F 0 0 0 8 CB 7 48 8 CB F B1 
        
        // head = 24; // for Promini version
        head = 1; // fix on direct ESP32 version ?
      }

      uint8_t sendHead[head]; // 2 frst already read ...
      waitAvailable(200);
      recvNb = readBytes( sendHead, head );

      Serial.print("found bytes to read "); Serial.print( recvNb ); Serial.print(" of "); Serial.println( head );

      if ( cblSend && ( recvNb <= 0 || sendHead[ 6-1 ] != 0x04 ) ) { // -1 for head len
        Serial.println(F("E:Not CBL packet"));
        debugDatas( sendHead, head );
        cblSend = false;
      }
      #if DBG_CBL
        debugDatas(sendHead, head);
      #endif

      if ( varSend1 ) {
        #if ASCII_OUTPUT
          if (!false) Serial.println(F("Send for TiVarSend (? garbage ?)"));
        #endif
        // just ignore this time wait for next packet reading loop
        setPollMode(savedPollMode); // restore PollMode
        return true;

      } else if ( varSend2 ) {
        #if ASCII_OUTPUT
          if (!false) Serial.println(F("Send for TiVarSend 2nd step"));
        #endif
        // waitAvailable(200);
        // recvNb = readBytes( sendHead, head );

        // -> 8 0 0 0 = could be the var size LSB -> MSB (A + (B * 256) + (C * 65536) + ... ) WITH +2 for CHK
        // C = var Type : String
        // 1 = name len => 
        // 64 => 'd' : name
        // 79 0 => can be CHK
        // 0 8 0 0 0 C 1 64 0 79 0
        #if DBG_CBL
          debugDatas( sendHead, head );
        #endif

        int vnLen = sendHead[6];
        char varName[8+1]; memset(varName, 0x00, 8+1);
        for(int i=0; i < vnLen; i++) { varName[i]= sendHead[6+1+i]; }

        uint8_t varType = sendHead[5]; // 0C -> STR
        uint32_t varLength = sendHead[1] + ( sendHead[2] << 8 ) + ( sendHead[3] << 16 ) + ( sendHead[4] << 24 );

        bool isSpeVarForHandle = false;
        if ( strncmp(varName, "tiaction", 8) == 0 ) {
          // except varType 0x0C -> String var
          isSpeVarForHandle = true;
        }

        #if HAS_DISPLAY
          scLandscape();
          scCls();
          tft.print(F("TiVarSend >> name : ")); tft.println(varName); 
          tft.print(F("TiVarSend >> type : ")); tft.println(varType, HEX); 
          tft.print(F("TiVarSend >> varLength : ")); tft.println(varLength); 
          scRestore();
        #endif

        File f;
        bool store = !isSpeVarForHandle && storage.isReady();
        if ( store ) {
          int error = 0;
          f = storage.createTiFile(varName, varType, error);
          if ( error != 0 ) {
            // FIXME : refactor : WARNING code
            scLandscape();
            tft.println(F(""));
            tft.println(F("[EE] Failed to open file !"));
            scRestore();
            store = false;
          }
        }

        #if ASCII_OUTPUT
          if (!false) { 
            Serial.print(F("TiVarSend >> name : ")); Serial.println(varName); 
            Serial.print(F("TiVarSend >> type : ")); Serial.println(varType, HEX); 
            Serial.print(F("TiVarSend >> varLength : ")); Serial.println(varLength); 
          }
        #else
          Serial.print(F(OUT_BIN_SENDVAR_NAME)); Serial.print(varName); Serial.write( 0x00 ); // VARNAME myVar 0-terminated
          Serial.print(F(OUT_BIN_SENDVAR_TYPE)); Serial.write( varType );                     // VARTYPE 0x0C
          Serial.print(F(OUT_BIN_SENDVAR_SIZE)); Serial.write( sendHead[4] ); Serial.write( sendHead[3] ); Serial.write( sendHead[2] ); Serial.write( sendHead[1] ); // VARSIZE MSB-LSB 
        #endif

        // 0x89 -> 0x09 - Ti92
        // 0x88 -> 0x08 - Ti89 & tiVoyage200
        const static uint8_t cACK[] = { 0x08, REP_OK, 0x00, 0x00 };
        const static uint8_t cCTS[] = { 0x08, CTS, 0x00, 0x00 };
        ti_write( (uint8_t*)cACK, 4 ); // send ACK
        ti_write( (uint8_t*)cCTS, 4 ); // send CTS

        waitAvailable(100);
        readBytes( TMP_RAM, 4 ); // read ACK of CTS

        //       v              [   a  b  c      ch ch] 
        // 88 15 C 0 0 0 0 0 0 6 0 61 62 63 0 2D 59 1 0 ....
        // |-------------------|
        const int prePacketLen = 10;
        memset( TMP_RAM, 0x00, prePacketLen );

        //recvNb = readBytes( TMP_RAM, prePacketLen, true ); // ,true -> waits for big variables (ex. popbin.ppg -> 24716 bytes long) [FIX]
        if ( !isPollMode() ) { 
          // delay(500); 
        }
        else { waitAvailable(5000); }
        recvNb = readBytes( TMP_RAM, prePacketLen);
        
        #if DBG_CBL
          if (!false) Serial.println(F("TiVarSend >> Packet Header"));
          debugDatas( TMP_RAM, prePacketLen );
        #endif

        if ( recvNb <= 0 ) {
          Serial.println( F("E:Failed to read from Ti") );
          #if HAS_DISPLAY
            int rot = tft.getRotation();
            tft.setRotation(1);
            tft.println( F("E:Failed to read from Ti") );
            tft.setRotation(rot);
          #endif
        }

        // CHK is a part of Var ? -> YES
        // int usedPacketLen = min( __SCREEN_SEG_MEM, varLength ); // do not overflow MCU's RAM for now ....
        int usedPacketLen = min( (uint32_t)64, varLength ); // Hey, we have to send it via UART HERE ....

        #if ASCII_OUTPUT
          if (!false) Serial.println(F("TiVarSend >> data : "));
        #else
          Serial.print(F(OUT_BIN_SENDVAR_DATA));
        #endif
        uint32_t total = 0;
        while( total < varLength ) {
          if ( total + usedPacketLen > varLength ) { usedPacketLen = (varLength - total); }
          memset( TMP_RAM, 0x00, usedPacketLen );
          waitAvailable(100);
          recvNb = readBytes( TMP_RAM, usedPacketLen );

          if ( recvNb <= 0 ) {
            Serial.println( F("E:Failed to read from Ti") );
            #if HAS_DISPLAY
              scLandscape();
              tft.println( F("E:Failed to read from Ti (2)") );
              scRestore();
            #endif
            break;
          }

          #if ASCII_OUTPUT
            debugDatas( TMP_RAM, usedPacketLen );

            #if HAS_DISPLAY
              int p = ( 100 * (total + usedPacketLen) / varLength );
              scLandscape();
              // tft.print( F("TiVarSend >> "));
              // tft.print( p );
              // tft.println( " %" );
              scLowerJauge(p);
              scRestore();
            #endif

          #else
            // FIXME no more 2 MCUs -> BEWARE
            Serial.write( TMP_RAM, usedPacketLen );
            while( Serial.available() == 0 ) { delay(2); }
            Serial.read(); // waits an handshake
          #endif

          if (isSpeVarForHandle) {
            handleTiActionData( TMP_RAM, usedPacketLen, total, varLength );
          } else if ( store ) {
            f.write( TMP_RAM, usedPacketLen );
          }

          total += usedPacketLen;
        }

        #if ASCII_OUTPUT
          if (!false) Serial.println(F("TiVarSend >> eof"));
          #if HAS_DISPLAY
            scLandscape();
            tft.println( F("TiVarSend >> eof"));
            scRestore();
          #endif
        #endif

        ti_write( (uint8_t*)cACK, 4 );             // ACK datas -------- ( 0x88 instead of 0x89 for a ti92)
        waitAvailable(100);
        recvNb = readBytes( TMP_RAM, 4 ); // read EOT   certified on V200
        ti_write( (uint8_t*)cACK, 4 );             // ACK EOT   --------

        #if ASCII_OUTPUT
          if (!false) Serial.println(F("TiVarSend >> eot"));
          #if HAS_DISPLAY
            scLandscape();
            tft.println( F("TiVarSend >> eot"));
            scRestore();
          #endif
        #else
          Serial.print(F(OUT_BIN_SENDVAR_EOF));
        #endif

        if (isSpeVarForHandle) {
          handleTiActionFlush();
        } else if ( store ) {
          f.flush();
          f.close();
        }

      } // end of VarSend2

      else if ( cblSend ) {
        if (false) Serial.println(F("Send for CBL"));
        CBL_ACK();
        CBL_CTS();

        if ( !isPollMode() ) { delay(50); }
        else { waitAvailable(100); }
        // 89 56 0 0 - Ti :ACK
        recvNb = readBytes( recv, 4 );
        if ( recvNb <= 0 ) {
          Serial.println(F("E:CBL/ACK"));

          #if DBG_CBL
            debugDatas(recv, 4);
          #endif

          // if (!true) { relaunchKeybPrgm(); return; }
          setPollMode(savedPollMode); // restore PollMode
          return false;
        }

        // 89 6 7 0 3 0 0 0 4 1 FF 7 1
        // 89 15 7 0 1 0 0 0 20 31 00 52 0 // Send {1}
        // 89 15 7 0 1 0 0 0 20 32 00 53 0 // Send {2}
        // 89 15 7 0 1 0 0 0 20 33 00 54 0 // Send {3}

        // 89 6 7 0 5 0 0 0 4 1 FF 9 1
        // 89 15 9 0 1 0 0 0 20 32 35 35 0 BD 0 // Send {255}
        //                       2  5  5 as chars

        // Send {102,103} -> 20 -> ',' (aka ' ')
        // 89 6 7 0 9 0 0 0 4 1 FF D 1 
        // 89 15 D 0 2 0 0 0 20 31 30 32 20 31 30 33 0 69 1

        // 32(h) -> 50(10) -> '2'
        // 35(h) -> 53(10) -> '5'
        // from byte #9 sent as string (Cf compat Ti82 legacy ...)
        // from 0x20 to 0x00 or read len in headers

        //       L M 1 2 3 4 5  6  7  CHK CHK
        // 89 15 7 0 1 0 0 0 20 33 00 54 0 // Send {3}

        waitAvailable(100);
        recvNb = readBytes( recv, 4 );
        if ( recvNb <= 0 ) {
          Serial.println(F("E:CBL/DT-HEAD"));
          #if DBG_CBL
            debugDatas(recv, 4);
          #endif
          setPollMode(savedPollMode); // restore PollMode
          return false;
        }

        int d0 = recv[2];
        int d1 = recv[3];
        int dataLen = ( d1 << 8 ) + d0;

        uint8_t cbldata[dataLen+2]; // +2 for CHK

        waitAvailable(200);
        recvNb = readBytes( cbldata, dataLen+2 );
        if ( recvNb <= 0 ) {
          Serial.println(F("E:CBL/DT-VAL"));
          setPollMode(savedPollMode); // restore PollMode
          return false;
        }

        cbldata[dataLen] = 0x00; // remove CHK
        cbldata[dataLen+1] = 0x00; // remove CHK

        char* value = (char*)&cbldata[5]; // just after 0x20, ends w/ 0x00
    #if 0
        Serial.print(F("CBL:"));
        Serial.println(value);
    #else
        int kc = atoi( value ); // assumes that there is only one value in { list }

        char msg[4];
        msg[0] = 'K';
        msg[1] = kc < 256 ? (char)kc : 0xFF;
        msg[2] = kc >> 8; // beware often be 0x00
        msg[3] = kc % 256;

        #if ASCII_OUTPUT
          Serial.write( (const uint8_t*)msg, 4);
        #else
          // CBL_SEND MSB-LSB
          Serial.print(F(OUT_BIN_SENDCBL)); Serial.write( (uint8_t)(kc >> 8) ); Serial.write( (uint8_t)(kc % 256) );
        #endif

    #endif

        CBL_ACK();
        waitAvailable(100);
        // 89 92 0 0 - Ti : EOT
        recvNb = readBytes( recv, 4 );
        if ( recvNb <= 0 ) {
          Serial.println(F("E:CBL/EOT"));
          setPollMode(savedPollMode); // restore PollMode
          return false;
        }
        CBL_ACK();
      } // end if cblSend

      setPollMode(savedPollMode); // restore PollMode
    } // endOf any other content
  } 
  // else {
  //   int b = read();
  //   if ( b < ' ' || b > 128 ) {
  //       Serial.print( "0x" );
  //       Serial.print( b, HEX );
  //       Serial.write( ' ' );
  //   } else {
  //       Serial.write( b );
  //   }
  //   resetLines();
  // }

  return true;
}

bool TiLink::sendVar(char* varName, bool silent) {
  if ( !silent ) {
    Serial.println("Send NOT SILENT -> NYI.");
    return false;
  }

  resetLines();

  char* filename = storage.findTiFile(varName);
  if ( filename == NULL ) {
    // FIXME : WARNING code
    Serial.println("Var NOT found");
    return false;   
  }

  File f = storage.getTiFile(filename);
  if ( !f ) {
    // FIXME : WARNING code
    Serial.println("Var NOT reachable");
    return false;
  }

  bool savedMode = isPollMode();
  setPollMode(false);
  delay(ISR_DURATION * 2);
  resetLines();

  long fileSize = f.size();
  long varSize = fileSize-2; // w/o chkLen
  //long varSize = fileSize;

  int nlength = strlen( filename );
  char* strVarType = &filename[ nlength - 2 ];
  uint8_t varType = (uint8_t)hexStrToInt( strVarType );
  
  #if HAS_DISPLAY
    scLandscape();
    scCls();
    tft.print(F("TiVarSend << name : ")); tft.println(varName); 
    tft.print(F("TiVarSend << type : ")); tft.println(varType, HEX); 
    tft.print(F("TiVarSend << varLength : ")); tft.println(varSize); 
    scRestore();
  #endif


  long initDatasLen = varSize;
  char data[ 20 + (/*sendingMode == SEND_MODE_RAM ? initDatasLen :*/ 0 ) ];
  data[0] = (unsigned char) ((initDatasLen + 0) / 256);
  data[1] = (unsigned char) ((initDatasLen + 0) % 256);
  long dataLen = 2 + initDatasLen;

  #if HAS_DISPLAY
    scLandscape();
    scLowerJauge( 0 );
    scRestore();
  #endif

  // == RTS ==
  int len = 0;
  if ( !false ) { Serial.println("ti_header"); }
  ti_header(varName, varType, dataLen, silent, len, true);
  delay(DEFAULT_POST_DELAY);

  uint8_t recv[4];
  
  // == ACK ==
  if ( !false ) { Serial.println("wait ACK"); }
  memset(recv, 0x00, 4);
  ti_recv(recv, 4, true); if ( recv[1] == 0x5a ) { Serial.println(F("E:failed to read ACK")); return false; }
  
  // == CTS ==
  if ( !false ) { Serial.println("wait CTS"); }
  memset(recv, 0x00, 4);
  ti_recv(recv, 4, true); if ( recv[1] == 0x5a ) { Serial.println(F("E:failed to read CTS")); return false; }
  
  // == ACK ==
  static const uint8_t B[4] = { 0x09, 0x56, 0x00, 0x00 };
  if ( !false ) { Serial.println("send ACK"); }
  ti_write((uint8_t*)B, 4);
  delay(DEFAULT_POST_DELAY/2);
  
  // == XDP == 
  bool archived = false;
  int sendingMode = SEND_MODE_SERIAL;
  if ( !false ) { Serial.println("ti_xdp"); }
  ti_xdp(data, dataLen, sendingMode, silent, len, archived, &f, false);
  delay(DEFAULT_POST_DELAY/2);
  
  // == ACK ==
  if ( !false ) { Serial.println("wait ACK"); }
  ti_recv(recv, 4, true); if ( recv[1] == 0x5a ) { Serial.println(F("E:failed to read ACK (2)")); return false; }
  
  // == EOT ==
  static const uint8_t D[4] = { 0x09, 0x92, 0x00, 0x00 };
  if ( !false ) { Serial.println("send EOT"); }
  ti_write((uint8_t*)D, 4);
  delay(DEFAULT_POST_DELAY/2); 
  
  // ACK ==
  if ( !false ) { Serial.println("wait ACK"); }
  memset(recv, 0x00, 4);
  ti_recv(recv, 4, true); if ( recv[1] == 0x5a ) { Serial.println(F("E:failed to read ACK (3)")); return false; }

  Serial.println(F("I:Var sent"));
  #if HAS_DISPLAY
    scLandscape();
    scLowerJauge( 100 );
    scRestore();
  #endif

  f.close();

  resetLines(true);
  
  setPollMode(savedMode);
  return true;
}