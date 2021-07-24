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

uint8_t inputBuff[INPUT_BUFF_LEN];
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

void pushToBuffer(uint8_t b) {
  inputBuff[ inputBuffCursor++ ] = b;
}

int consumeBuffer() {
  if ( inputBuffCursor == 0 ) {
    return -1;
  }
  int bte = inputBuff[inputBuffCursor];
  inputBuff[inputBuffCursor] = 0x00;
  inputBuffCursor--;
  return bte;
}

void TiLink::poll() {
  // FIXME : poll() :> incoming bytes to a queue
  // then available() gives queue size
  // read() consumes the queue
  // peek() returns top of queue
  if ( inputBuffCursor >= INPUT_BUFF_LEN ) {
    return;
  }

  int result = ti_read(5); // 5msec waiting each time
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
  return ti_write(c);
}