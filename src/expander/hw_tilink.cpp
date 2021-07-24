/**
 * XtsTiExpander V2
 * 
 * ti-link stack
 * 
 * Xtase - fgalliat @Jul 2021
 */

#include <Arduino.h>
#include "hw_tilink.h"

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
  // pinMode(TIring, INPUT);           // set pin to input
  // pinMode(TItip, INPUT);            // set pin to input
  // if (reboot) {
  //   digitalWrite(TIring, LOW);       // for reset purposes
  //   digitalWrite(TItip, LOW);        // for reset purposes
  // }
  // digitalWrite(TIring, HIGH);       // turn on pullup resistors
  // digitalWrite(TItip, HIGH);        // turn on pullup resistors
  pinMode(TIring, INPUT_PULLUP);
  pinMode(TItip, INPUT_PULLUP);
}

#define serial_ Serial
#define ERR_WRITE_TIMEOUT -1

void resetLines(void) {
	// pinMode(ring_, INPUT_PULLUP);           // set pin to input with pullups
	// pinMode(tip_, INPUT_PULLUP);            // set pin to input with pullups
  __resetTILines(false);
}

int ti_write(uint8_t b) {
  int ring_ = TIring;
  int tip_ = TItip;
  uint8_t byte = b;

  unsigned long previousMicros;
	if (serial_) {
		serial_.print("Sending byte ");
		serial_.println(byte);
	}

	// Send all of the bits in this byte
	for(int bit = 0; bit < 8; bit++) {
		
		// Wait for both lines to be high before sending the bit
		previousMicros = micros();
		while (digitalRead(ring_) == LOW || digitalRead(tip_) == LOW) {
			if (micros() - previousMicros > TIMEOUT) {
				resetLines();
				return ERR_WRITE_TIMEOUT;
			}
		}
		
		// Pull one line low to indicate a new bit is going out
		bool bitval = (byte & 1);
		int line = (bitval)?ring_:tip_;
		pinMode(line, OUTPUT);
		digitalWrite(line, LOW);
		
		// Wait for peer to acknowledge by pulling opposite line low
		line = (bitval)?tip_:ring_;
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


    // long previousMillis = 0;
    // int bit;
    // uint8_t byte;
    // for (bit = 0; bit < 8; bit++) {
    //   previousMillis = 0;

    //   while ((digitalRead(TIring) << 1 | digitalRead(TItip)) != 0x03) {
    //     // delay(1);
    //     if (previousMillis++ > TIMEOUT) {
    //       // Serial.print("FAILED @ BYTE "); Serial.println(j);
    //       return -1;
    //       //return ERR_WRITE_TIMEOUT + j + 100 * bit;
    //     }
    //   };

    //   if (byte & 1) {
    //     pinMode(TIring, OUTPUT);
    //     digitalWrite(TIring, LOW);
    //     previousMillis = 0;
    //     while (digitalRead(TItip) == HIGH) {
    //       if (previousMillis++ > TIMEOUT)
    //         return -2;
    //        // return ERR_WRITE_TIMEOUT + 10 + j + 100 * bit;
    //     };

    //     __resetTILines();
    //     previousMillis = 0;
    //     while (digitalRead(TItip) == LOW) {
    //       if (previousMillis++ > TIMEOUT)
    //         return -3;
    //        // return ERR_WRITE_TIMEOUT + 20 + j + 100 * bit;
    //     };
    //   } else {
    //     pinMode(TItip, OUTPUT);
    //     digitalWrite(TItip, LOW);     //should already be set because of the pullup resistor register
    //     previousMillis = 0;
    //     while (digitalRead(TIring) == HIGH) {
    //       if (previousMillis++ > TIMEOUT)
    //         return -4;
    //         //return ERR_WRITE_TIMEOUT + 30 + j + 100 * bit;
    //     };

    //     __resetTILines();
    //     previousMillis = 0;
    //     while (digitalRead(TIring) == LOW) {
    //       if (previousMillis++ > TIMEOUT)
    //       return -5;
    //         //return ERR_WRITE_TIMEOUT + 40 + j + 100 * bit;
    //     };
    //   }

    //   byte >>= 1;
    // }
    // return 0;
}

#define ERR_READ_ENTER_TIMEOUT -1

// default : GET_ENTER_TIMEOUT
int ti_read(long timeout) {
int ring_ = TIring;
  int tip_ = TItip;

unsigned long previousMicros = 0;
	// *byte = 0;
  int byte = 0;
	
	// Pull down each bit and store it
	for (int bit = 0; bit < 8; bit++) {
		int linevals;

		previousMicros = micros();
		while ((linevals = ((digitalRead(ring_) << 1) | digitalRead(tip_))) == 0x03) {
			if (micros() - previousMicros > timeout) {
				resetLines();
				if (serial_ && bit > 0) {
					serial_.print("died waiting for bit "); serial_.println(bit);
				}
				return ERR_READ_ENTER_TIMEOUT;
			}
		}
		
		// Store the bit, then acknowledge it
		//*byte = (*byte >> 1) | ((linevals == 0x01)?0x80:0x00);
    byte = (byte >> 1) | ((linevals == 0x01)?0x80:0x00);

		int line = (linevals == 0x01)?tip_:ring_;
		pinMode(line, OUTPUT);
		digitalWrite(line, LOW);
		
		// Wait for the peer to indicate readiness
		line = (linevals == 0x01)?ring_:tip_;		
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
	if (serial_) {
		serial_.print("Got byte ");
		// serial_.println(*byte);
    serial_.println(byte);
	}
	// return 0;
  return (int)byte;


    // long previousMillis = 0;
    // int bit;
    // uint8_t v, byteout = 0;
    // for (bit = 0; bit < 8; bit++) {
    //   previousMillis = 0;

    //   while ((v = (digitalRead(TIring) << 1 | digitalRead(TItip))) == 0x03) {
    //     if (previousMillis++ > timeout) {
    //         // return ERR_READ_TIMEOUT + j + 100 * bit;
    //         return -1;
    //     }
    //   }
    //   if (v == 0x01) {
    //     // Serial.write('1');
    //     byteout = (byteout >> 1) | 0x80;
    //     pinMode(TItip, OUTPUT);
    //     digitalWrite(TItip, LOW);     //should already be set because of the pullup resistor register
    //     previousMillis = 0;
    //     while (digitalRead(TIring) == LOW) {            //wait for the other one to go low
    //       if (previousMillis++ > TIMEOUT) {
    //         // return ERR_READ_TIMEOUT + 10 + j + 100 * bit;
    //         return -11;
    //       }
    //     }
    //     //pinMode(TIring,OUTPUT);
    //     digitalWrite(TIring, HIGH);
    //   } else {
    //     // Serial.write('0');
    //     byteout = (byteout >> 1) & 0x7F;
    //     pinMode(TIring, OUTPUT);
    //     digitalWrite(TIring, LOW);     //should already be set because of the pullup resistor register
    //     previousMillis = 0;
    //     while (digitalRead(TItip) == LOW) {
    //       if (previousMillis++ > TIMEOUT) {
    //         // return ERR_READ_TIMEOUT + 20 + j + 100 * bit;
    //         return -10;
    //       }
    //     }
    //     //pinMode(TItip,OUTPUT);
    //     digitalWrite(TItip, HIGH);
    //   }
    //   pinMode(TIring, INPUT);           // set pin to input
    //   digitalWrite(TIring, HIGH);       // turn on pullup resistors
    //   pinMode(TItip, INPUT);            // set pin to input
    //   digitalWrite(TItip, HIGH);        // turn on pullup resistors
    // }
    // return (int)byteout;
}

int ti_writes(uint8_t* seg, int segLen) {
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

