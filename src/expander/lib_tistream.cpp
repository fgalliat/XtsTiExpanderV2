/**
 * XtsTiExpander V2
 * 
 * Ti Software Streams stack
 * 
 * Xtase - fgalliat @Aug 2021
 */

#include <Arduino.h>
#include "globals.h"
#include "hw_tilink.h"

TiDummyStream::TiDummyStream() {}
TiDummyStream::~TiDummyStream() {}

void TiDummyStream::resetInputBuff() {
    memset(inputBuff, 0x00, DUMMY_INPUT_LEN+1);
    inputBuffCursor = 0;
}

bool TiDummyStream::isOpened() {
    return opened;
}

bool TiDummyStream::isLoopDisabled() {
    return loopDisabled;
}

void TiDummyStream::setLoopDisabled(bool state) {
    loopDisabled = state;
}

bool TiDummyStream::begin() { 
    resetInputBuff();
    opened = true;
    return true; 
}
void TiDummyStream::end() {
    resetInputBuff();
    opened = false;
}

void TiDummyStream::flush() {;}
size_t TiDummyStream::write(uint8_t c) { 
    if ( ti_write(c) >= 0 ) {
        return 1;
    }
    return 0; 
}
int TiDummyStream::available() { return inputBuffCursor; }
int TiDummyStream::peek() { return inputBuffCursor <= 0 ? -1 : inputBuff[inputBuffCursor-1]; }
int TiDummyStream::read() { 
  if ( inputBuffCursor <= 0 ) { return -1; }
  int ch = inputBuff[inputBuffCursor-1];
  memmove( &inputBuff[0], &inputBuff[1], inputBuffCursor );
  inputBuffCursor--;
  return ch;
}

bool TiDummyStream::appendToInputBuffer(char ch) {
    if ( inputBuffCursor >= DUMMY_INPUT_LEN ) { return false; }
    inputBuff[inputBuffCursor++] = ch;
    return true;
}

// -=========================-

TiNativeStream::TiNativeStream() {}
TiNativeStream::~TiNativeStream() {}

bool TiNativeStream::begin() { return true; }
void TiNativeStream::end() {}

void TiNativeStream::flush() {;}
int TiNativeStream::available() { return 0; }
int TiNativeStream::read() { return -1; }
int TiNativeStream::peek() { return -1; }
size_t TiNativeStream::write(uint8_t c) {return 0;}
