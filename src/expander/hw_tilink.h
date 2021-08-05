/**
 * XtsTiExpander V2
 * 
 * ti-link stack
 * 
 * Xtase - fgalliat @Jul 2021
 */

#define TI_MODEL_92_OR_V200 1

// ====================
// But : ONLY SEND_MODE_SERIAL is used for all cases
// content sent in one-shot mode from RAM
#define SEND_MODE_RAM    0x01
// content is streamed from PROGMEM
#define SEND_MODE_FLASH  0x02
// content is streamed from Serial Port
#define SEND_MODE_SERIAL 0x03
// ====================

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
int ti_recv(uint8_t* seg, int segMaxLen, bool waitLong=false, long waitOnlyFirstByte=GET_ENTER_TIMEOUT);

bool ti_sendVar(Stream* input);
bool ti_reqScreen(Stream* output, bool ascii);

int ti_sendKeyStrokes(char* data, int len=-1);
int ti_sendKeyStroke(int data);

//uint8_t* ti_chk(uint8_t b[], int len);
uint16_t ti_chk(uint8_t b[], int len);

void ti_header(const char* varName, int fileType, int dataLen, bool silent, int& dtLen, bool send);
void ti_xdp(char data[], int dataLen, int sendingMode, bool silent, int& dtLen, bool archived, Stream* input, bool inputIsSerial);

// call in loop() -> can recvVar, recvCBL, ....
// return true if something happend
bool ti_handle();
