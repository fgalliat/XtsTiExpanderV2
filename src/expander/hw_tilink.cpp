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
  uint8_t byte = b;

  unsigned long previousMicros;
	// if (serial_) {
	// 	serial_.print("Sending byte ");
	// 	serial_.println(byte);
	// }

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

