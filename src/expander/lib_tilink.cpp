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

TiLink::TiLink() {
}

TiLink::~TiLink() {
}

bool TiLink::begin() {
  return setup_tilink();
}

void TiLink::end() {
}

void TiLink::poll() {
}

int TiLink::available() {
  return 0;
}
int TiLink::read() {
  return ti_read();
}

int TiLink::peek() {
  return -1;
}

void TiLink::flush() {
}

size_t TiLink::write(uint8_t c) {
  return ti_write(c);
}