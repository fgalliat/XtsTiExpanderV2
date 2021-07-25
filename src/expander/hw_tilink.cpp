/**
 * XtsTiExpander V2
 * 
 * ti-link stack
 * 
 * Xtase - fgalliat @Jul 2021
 */

#include <Arduino.h>
#include "hw_tilink.h"

extern void lockISR();

// ------ Wiring ------------
//  //White 
// #define TIring -256
//  //Red 
// #define TItip -255

//White
int TIring = -1;
//Red
int TItip = -1;

// ------ Hardware IDs -------
#if TI_MODEL_92_OR_V200
  #define PC_2_TI 0x09
  #define TI_2_PC 0x89

  #define TI_SCREEN_WIDTH 240
  #define TI_SCREEN_HEIGHT 128
#else
  #error "No TI model selected";
#endif

// ===================== RAM Settings ==================
// max alowed to store big packets
// based on Arduino UNO available RAM size - but is really enough
#define MAX_ARDUINO_MEM_SIZE 870

// 3840 bytes for a TI-92 => too HUDGE to fit in RAM
#define MAX_TI_SCR_SIZE ( TI_SCREEN_HEIGHT*(TI_SCREEN_WIDTH/8) )

// to have complete scanlines
#define nbScanlines ( MAX_ARDUINO_MEM_SIZE / (TI_SCREEN_WIDTH/8) )

#define SCREEN_SEG_MEM ( nbScanlines * (TI_SCREEN_WIDTH/8) )

// ===================== RAM Settings ==================

// big data purpose RAM seg
uint8_t screen[SCREEN_SEG_MEM];
// generic purpose RAM seg
uint8_t recv[16];
uint8_t data[4] = { PC_2_TI, REQ_SCREENSHOT, 0x00, 0x00 };

// ------ Forward symbols -----
void __resetTILines(bool reboot=false);

bool setup_tilink(int tip, int ring) {
    TIring = ring;
    TItip = tip;
    if ( TIring <= 0 || TItip <= 0 ) {
        Serial.println("/!\\ TIring ot TItip not setted");
        return false;
    }

    __resetTILines(!true);

    return true;
}

void __resetTILines(bool reboot) {
  pinMode(TIring, INPUT_PULLUP);
  pinMode(TItip, INPUT_PULLUP);
  // if (reboot) {
  //   digitalWrite(TIring, LOW);       // for reset purposes
  //   digitalWrite(TItip, LOW);        // for reset purposes
  // }
  // digitalWrite(TIring, HIGH);       // turn on pullup resistors
  // digitalWrite(TItip, HIGH);        // turn on pullup resistors
}

#define serial_ Serial
#define ERR_WRITE_TIMEOUT -1
#define ERR_READ_ENTER_TIMEOUT -1

void resetLines(void) {
  __resetTILines(false);
}

// impl. based on ArTiCl code
int ti_write(uint8_t b) {
	lockISR(); // will auto release
	uint8_t byte = b;

  	unsigned long previousMicros;

	// Send all of the bits in this byte
	for(int bit = 0; bit < 8; bit++) {
		
		// Wait for both lines to be high before sending the bit
		previousMicros = micros();
		while (digitalRead(TIring) == LOW || digitalRead(TItip) == LOW) {
			if (micros() - previousMicros > TIMEOUT) {
				resetLines();
				return ERR_WRITE_TIMEOUT;
			}
		}
		
		// Pull one line low to indicate a new bit is going out
		bool bitval = (byte & 1);
		int line = (bitval)?TIring:TItip;
		pinMode(line, OUTPUT);
		digitalWrite(line, LOW);
		
		// Wait for peer to acknowledge by pulling opposite line low
		line = (bitval)?TItip:TIring;
		previousMicros = micros();
		while (digitalRead(line) == HIGH) {
			if (micros() - previousMicros > TIMEOUT) {
				resetLines();
				return ERR_WRITE_TIMEOUT;
			}
		}

		// Wait for peer to indicate readiness by releasing that line
		resetLines();
		previousMicros = micros();
		while (digitalRead(line) == LOW) {
			if (micros() - previousMicros > TIMEOUT) {
				resetLines();
				return ERR_WRITE_TIMEOUT;
			}
		}
		resetLines();
		
		// Rotate the next bit to send into the low bit of the byte
		byte >>= 1;
	}

	return 0;
}

// default : GET_ENTER_TIMEOUT
// impl. based on ArTiCl code
int ti_read(long enterTimeout, long nextTimeout) {
	lockISR(); // prevent from take long more time than ISR frequency

  	long timeout = enterTimeout * 1000; // mS to uS

	unsigned long previousMicros = 0;
	// *byte = 0;
  	int byte = 0;
	
	// Pull down each bit and store it
	for (int bit = 0; bit < 8; bit++) {
		int linevals;

		previousMicros = micros();
		while ((linevals = ((digitalRead(TIring) << 1) | digitalRead(TItip))) == 0x03) {
			if (micros() - previousMicros > timeout) {
				resetLines();
				if (serial_ && bit > 0) {
					serial_.print("died waiting for bit "); serial_.println(bit);
				}
				return ERR_READ_ENTER_TIMEOUT;
			}
		}
		timeout = nextTimeout * 1000; // after bit 0
		
		// Store the bit, then acknowledge it
		//*byte = (*byte >> 1) | ((linevals == 0x01)?0x80:0x00);
    	byte = (byte >> 1) | ((linevals == 0x01)?0x80:0x00);

		int line = (linevals == 0x01)?TItip:TIring;
		pinMode(line, OUTPUT);
		digitalWrite(line, LOW);
		
		// Wait for the peer to indicate readiness
		line = (linevals == 0x01)?TIring:TItip;		
		previousMicros = micros();
		while (digitalRead(line) == LOW) {            //wait for the other one to go high again
			if (micros() - previousMicros > TIMEOUT) {
				resetLines();
				if (serial_) {
					serial_.print("died waiting for bit ack "); serial_.println(bit);
				}
				return ERR_READ_TIMEOUT;
			}
		}

		// Now set them both high and to input
		resetLines();
	}
	// if (serial_) {
	// 	serial_.print("Got byte ");
	// 	// serial_.println(*byte);
    //   serial_.println(byte);
	// }
	// return 0;
  return (int)byte;
}

int ti_write(uint8_t* seg, int segLen) {
    int result;
    for(int i=0; i < segLen; i++) {
        result = ti_write(seg[i]);
        // TODO : need WAIT ?
        if ( result < 0 ) {
            return i;
        }
    }
    return segLen;
}

int ti_recv(uint8_t* seg, int segMaxLen) {
    int result;
    for(int i=0; i < segMaxLen; i++) {
        result = ti_read();
        // TODO : need WAIT ?
        if ( result < 0 ) {
            return i;
        }
        seg[i] = (uint8_t)result;
    }
    return segMaxLen;
}

bool ti_reqScreen(Stream* output, bool ascii) {
  // lockISR(); // not for now

  int recvNb = -1;
  resetLines();
  
  data[1] = REQ_SCREENSHOT;
  ti_write(data, 4);
  delay(50);

  resetLines();
  delay(100);
  recvNb = ti_recv(recv, 4); // => calculator's ACK
  resetLines();
  recvNb = ti_recv(recv, 4); // ? 89 15 00 0F ? <TI92> <?> <LL> <HH> => 00 0F => 0F 00 = 3840 screen mem size
  if ( recvNb != 4 ) {
    serial_.println(F("E:TI did not ACK'ed, abort"));
    return false;
  }
  
  if ( !ascii ) {
    output->write( (uint8_t)(MAX_TI_SCR_SIZE >> 8) );
    output->write( (uint8_t)(MAX_TI_SCR_SIZE % 256) );
  }

  // Dumping screen raster
  for(int j=0; j < MAX_TI_SCR_SIZE; j+=SCREEN_SEG_MEM) {
    int howMany = (j+SCREEN_SEG_MEM) < MAX_TI_SCR_SIZE ? SCREEN_SEG_MEM : SCREEN_SEG_MEM - ( (j+SCREEN_SEG_MEM) % MAX_TI_SCR_SIZE );

    ti_recv(screen, howMany);
    //dispScreenMem(howMany, ascii);
	if ( !ascii ) {
		output->write( screen, howMany );
	} else {
		for (int i = 0; i < howMany; i++) {
			for (int j = 7; j >= 0; j--) {
				if (screen[i] & (1 << j)) {
					output->write('#');
				} else {
					output->write('.');
				}
			}
			if (i % (TI_SCREEN_WIDTH/8) == (TI_SCREEN_WIDTH/8)-1) { // 240/8 => 30 bytes
				output->println();
			}
		}
	}
  }

  recvNb = ti_recv(recv, 2); // checksum from TI
  //Serial.println(recvNb);
  
  data[1] = REP_OK;
  ti_write(data, 4); // Arduino's ACK
  // delay(50);
  return true;
}


int ti_sendKeyStrokes(char* data, int len) {
  if ( len < 0 ) { len = strlen(data); } 
  for(int i=0; i < len; i++) { ti_sendKeyStroke( (int)data[i] ); }
  return 0;
}

int ti_sendKeyStroke(int data) {
  // http://merthsoft.com/linkguide/ti92/remote.html
  uint8_t D[4] = { PC_2_TI, CMD_REMOTE, (uint8_t)(data%256), (uint8_t)(data/256) };
  ti_write(D, 4);
  delay(DEFAULT_POST_DELAY/2);
  ti_recv(D, 4); if ( D[1] != REP_OK ) { serial_.print("E:failed to read ACK ->"); serial_.println(D[1], HEX); return -1; }
  delay(DEFAULT_POST_DELAY/2);
  return 0;
}

// ============================================

bool ti_handle() {
	return false;
}