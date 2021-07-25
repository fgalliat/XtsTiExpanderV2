/**
 * XtsTiExpander V2
 * 
 * ti-link stack
 * 
 * Xtase - fgalliat @Jul 2021
 */

#define TI_MODEL_92_OR_V200 1

// ------ Ti Commands ---------
#define REQ_SCREENSHOT 0x6D
#define REQ_BACKUP     0xA2
#define REP_OK         0x56
#define CMD_REMOTE     0x87

// ------ Timeouts ------------
#define TIMEOUT 300
#define GET_ENTER_TIMEOUT 1000
#define ERR_READ_TIMEOUT 300

#define DEFAULT_POST_DELAY 100

// return true if init
bool setup_tilink(int tip, int ring);

void __resetTILines(bool reboot);

// return < 0 if failed to write / 0 otherwise
int ti_write(uint8_t b);

// < 0 failure
int ti_read(long enterTimeout=GET_ENTER_TIMEOUT, long nextTimeout=GET_ENTER_TIMEOUT);

// return written bytes nb
int ti_write(uint8_t* seg, int segLen);
// return read bytes nb
int ti_recv(uint8_t* seg, int segMaxLen);

bool ti_sendVar(Stream* input);
bool ti_reqScreen(Stream* output, bool ascii);

// call in loop() -> can recvVar, recvCBL, ....
// return true if something happend
bool ti_handle();
