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

void TiLink::resetLines() {
  __resetTILines(false);
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
  // return ti_read();
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

void dummyMode() {
  Serial.println("You are in dummy mode");
}

#define ASCII_OUTPUT 1

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
    Serial.print( data[i], HEX ); Serial.print( F(" ") );
  }
  Serial.println();
}

bool TiLink::handleCalc() {
  if ( available() <= 0 ) { return false; }

  int recvNb;

  Serial.println(available());

  // FIXME : better -- Cf further read() single byte
  if ( available() < 2 ) { return false; }

  if ( available() >= 2 ) {
    recvNb = readBytes(recv, 2);

    // ASM version PRGM starts - direct bytes
    if ( recvNb == 2 && recv[0] == 'X' && recv[1] == ':' ) {
      recvNb = readBytes(recv, 7);
      if ( recvNb == 7 && recv[0] == '?' && recv[1] == 'b' ) {
        // X:?begin\n
        // dummy serial mode : XtsTerm.92p
        dummyMode();
      }
    } else if ( recvNb == 2 ) { // any other content
      #define DBG_CBL 1

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
        head = 24;
      }

      uint8_t sendHead[head]; // 2 frst already read ...
      recvNb = readBytes( sendHead, head );

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

Serial.println(available());
resetLines();

        return true;

      } else if ( varSend2 ) {
        #if ASCII_OUTPUT
          if (!false) Serial.println(F("Send for TiVarSend 2nd step"));
        #endif
        recvNb = readBytes( sendHead, head );
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
        while( available() <= 0 ) {
          // FIXME : timeout
          delay(30);
        }

        readBytes( TMP_RAM, 4 ); // read ACK of CTS

        //       v              [   a  b  c      ch ch] 
        // 88 15 C 0 0 0 0 0 0 6 0 61 62 63 0 2D 59 1 0 ....
        // |-------------------|
        const int prePacketLen = 10;
        memset( TMP_RAM, 0x00, prePacketLen );

        //recvNb = readBytes( TMP_RAM, prePacketLen, true ); // ,true -> waits for big variables (ex. popbin.ppg -> 24716 bytes long) [FIX]
        while( available() == 0 ) {
          // FIXME : add timeout
          delay(100);
        }
        recvNb = readBytes( TMP_RAM, prePacketLen);
        
        #if DBG_CBL
          if (!false) Serial.println(F("TiVarSend >> Packet Header"));
          debugDatas( TMP_RAM, prePacketLen );
        #endif

        if ( recvNb <= 0 ) {
          Serial.println( F("E:Failed to read from Ti") );
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
          recvNb = readBytes( TMP_RAM, usedPacketLen );

          if ( recvNb <= 0 ) {
            Serial.println( F("E:Failed to read from Ti") );
            break;
          }

          #if ASCII_OUTPUT
            debugDatas( TMP_RAM, usedPacketLen );
          #else
            Serial.write( TMP_RAM, usedPacketLen );
            while( Serial.available() == 0 ) { delay(2); }
            Serial.read(); // waits an handshake
          #endif
          total += usedPacketLen;
        }

        ti_write( (uint8_t*)cACK, 4 );             // ACK datas -------- ( 0x88 instead of 0x89 for a ti92)
        while( available() <= 0 ) {
          // FIXME : timeout
          delay(30);
        }
        recvNb = readBytes( TMP_RAM, 4 ); // read EOT   certified on V200
        ti_write( (uint8_t*)cACK, 4 );             // ACK EOT   --------

        #if ASCII_OUTPUT
          if (!false) Serial.println(F("TiVarSend >> eof"));
        #else
          Serial.print(F(OUT_BIN_SENDVAR_EOF));
        #endif

      } // end of VarSend2

      else if ( cblSend ) {
        if (false) Serial.println(F("Send for CBL"));
        CBL_ACK();
        CBL_CTS();

        while( available() <= 0 ) {
          // FIXME : timeout
          delay(30);
        }

        // 89 56 0 0 - Ti :ACK
        recvNb = readBytes( recv, 4 );
        if ( recvNb <= 0 ) {
          Serial.println(F("E:CBL/ACK"));

          #if DBG_CBL
            debugDatas(recv, 4);
          #endif

          // if (!true) { relaunchKeybPrgm(); return; }
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


        recvNb = readBytes( recv, 4 );
        if ( recvNb <= 0 ) {
          Serial.println(F("E:CBL/DT-HEAD"));
          #if DBG_CBL
            debugDatas(recv, 4);
          #endif
          return false;
        }

        int d0 = recv[2];
        int d1 = recv[3];
        int dataLen = ( d1 << 8 ) + d0;

        uint8_t cbldata[dataLen+2]; // +2 for CHK

        recvNb = readBytes( cbldata, dataLen+2 );
        if ( recvNb <= 0 ) {
          Serial.println(F("E:CBL/DT-VAL"));
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
        while( available() <= 0 ) {
          // FIXME : timeout
          delay(30);
        }

        // 89 92 0 0 - Ti : EOT
        recvNb = readBytes( recv, 4 );
        if ( recvNb <= 0 ) {
          Serial.println(F("E:CBL/EOT"));
          return false;
        }
        CBL_ACK();
      } // end if cblSend

    }
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