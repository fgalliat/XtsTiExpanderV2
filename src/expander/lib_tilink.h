/**
 * XtsTiExpander V2
 * 
 * ti-link stack
 * 
 * Xtase - fgalliat @Jul 2021
 */

#define TI_MODEL_92_OR_V200 1

// return true if init
bool setup_tilink();

// return < 0 if failed to write / 0 otherwise
int ti_write(uint8_t b);

// < 0 failure
int ti_read();

// return written bytes nb
int ti_write(uint8_t* seg, int segLen);
// return read bytes nb
int ti_recv(uint8_t* seg, int segMaxLen);

bool ti_sendVar(Stream* input);
bool ti_reqScreen(Stream* output);

// call in loop() -> can recvVar, recvCBL, ....
// return true if something happend
bool ti_handle();
