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