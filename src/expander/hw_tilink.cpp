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

extern void lockISR();

#define ISR_DURATION 20
extern bool isPollMode();
extern void setPollMode(bool state);

// ------ Wiring ------------
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
// void __resetTILines(bool reboot=false);

bool setup_tilink(int tip, int ring) {
    TIring = ring;
    TItip = tip;
    if ( TIring <= 0 || TItip <= 0 ) {
        Serial.println("/!\\ TIring ot TItip not setted");
        return false;
    }

    __resetTILines(true);
    return true;
}

void __resetTILines(bool reboot) {
  pinMode(TIring, INPUT_PULLUP);
  pinMode(TItip, INPUT_PULLUP);

  if ( reboot ) {
	digitalWrite(TIring, LOW);       // for reset purposes
	digitalWrite(TItip, LOW);        // for reset purposes	
	delay(5);
	digitalWrite(TIring, HIGH);       // turn on pullup resistors
	digitalWrite(TItip, HIGH);        // turn on pullup resistors
	delay(10);
  }

}

#define serial_ Serial
#define ERR_WRITE_TIMEOUT -1
#define ERR_READ_ENTER_TIMEOUT -1

void resetLines(bool reboot=false) {
  __resetTILines(reboot);
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

// return read bytes
int ti_recv(uint8_t* seg, int segMaxLen, bool waitLong, long waitOnlyFirstByte) {
	int tm = waitOnlyFirstByte;
	for(int i=0; i < segMaxLen; i++) {
		if ( i == 0 && waitLong ) {
			int tmp = ti_read(10000, 800);
			if ( tmp < 0 ) {
				return 0;
			}
			seg[0] = (uint8_t)tmp;
		} else {
			int tmp = ti_read(tm, tm);
			if ( tmp < 0 ) {
				return i;
			}
			seg[i] = (uint8_t)tmp;
			if ( waitOnlyFirstByte != GET_ENTER_TIMEOUT ) { tm = 80; } // read other bytes faster
		}
	}
	return segMaxLen;
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

// int ti_recv(uint8_t* seg, int segMaxLen) {
//     int result;
//     for(int i=0; i < segMaxLen; i++) {
//         result = ti_read();
//         // TODO : need WAIT ?
//         if ( result < 0 ) {
//             return i;
//         }
//         seg[i] = (uint8_t)result;
//     }
//     return segMaxLen;
// }

bool ti_reqScreen(Stream* output, bool ascii) {
  bool savedPollMode = isPollMode();
  setPollMode(false); // disable polling
  delay(ISR_DURATION * 2);

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
	setPollMode(savedPollMode);
    return false;
  }
  
  if ( !ascii && output != NULL ) {
    output->write( (uint8_t)(MAX_TI_SCR_SIZE >> 8) );
    output->write( (uint8_t)(MAX_TI_SCR_SIZE % 256) );
  }

  #if HAS_DISPLAY
    int rot = tft.getRotation();
    tft.setRotation(1);
	scCls();
  #endif

  // Dumping screen raster
  int xx = 0, yy = 0;
  for(int j=0; j < MAX_TI_SCR_SIZE; j+=SCREEN_SEG_MEM) {
    int howMany = (j+SCREEN_SEG_MEM) < MAX_TI_SCR_SIZE ? SCREEN_SEG_MEM : SCREEN_SEG_MEM - ( (j+SCREEN_SEG_MEM) % MAX_TI_SCR_SIZE );

    ti_recv(screen, howMany);
	if ( !ascii && output != NULL ) {
		output->write( screen, howMany );
	} else {
		for (int i = 0; i < howMany; i++) {
			for (int j = 7; j >= 0; j--) {
				if (screen[i] & (1 << j)) {
					#if HAS_DISPLAY
					  // +7 => ti 128px / T_DISPLAY 135px height
					  if ( yy < tft.height() ) { tft.drawPixel( xx, yy+7, ti_fgColor ); }
					#endif
					if (output != NULL) output->write('#');
				} else {
					if (output != NULL) output->write('.');
				}
				xx++;
			}
			if (i % (TI_SCREEN_WIDTH/8) == (TI_SCREEN_WIDTH/8)-1) { // 240/8 => 30 bytes
				if (output != NULL) output->println();
				yy++;
				xx = 0;
			}
		}
	}
  }

  #if TTGO_TDISPLAY
    tft.setRotation(rot);
  #endif

  recvNb = ti_recv(recv, 2); // checksum from TI
  //Serial.println(recvNb);
  
  data[1] = REP_OK;
  ti_write(data, 4); // Arduino's ACK
  // delay(50);
  setPollMode(savedPollMode);
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

// ============================================
void ti_header(const char* varName, int fileType, int dataLen, bool silent, int& dtLen, bool send) {
	int i;

	int nameLength = strlen(varName);
	// packLen = 4 bytes for dataLen
	// + 1 for dataType
	// + 1 for fileName.length
	// + n for filename
	// + 1 for 0 terminating
	int packLen = 4 + 1 + 1 + nameLength + 1;
	// finalLen is : 1 byte for machine ID
	// + 1 for cmd
	// + 2 for packLen
	// + packLen
	// + 2 for CHK
	const int finalLen = 1 + 1 + 2 + packLen + 2;

    // static : the ONLY way to be sure to return correctly the given array (when converting to pointer)
    // but consume all RAM
    uint8_t result[ finalLen ]; memset(result, 0x00, finalLen);
	//static char result[ 256 ];

	// MACHINE ID
	result[0] = (uint8_t) (silent ? 0x09 : 0x08);
	// CMD
	result[1] = (uint8_t) (silent ? 0xC9 : 0x06);
    //result[1] = (char) (silent ? 0x06 : 0xC9);

	// packet length
	result[2] = (uint8_t) (packLen % 256);
	result[3] = (uint8_t) (packLen / 256);

	// dataLen :: TODO : manager more than 64KB files
	result[4] = (uint8_t) (dataLen % 256);
	result[5] = (uint8_t) (dataLen / 256);
	result[6] = 0x00;
	result[7] = 0x00;

	// file type
	result[8] = (uint8_t) fileType;

	// file name length
	result[9] = (uint8_t) nameLength;

	// file name
	for (i = 0; i < nameLength; i++) {
		result[10 + i] = varName[i];
	}
	// zero-terminated
	result[10 + nameLength] = 0x00;

	// 11+nameLength => 0
	// 12+nameLength => 0

	// CHK
	// uint8_t* chk = ti_chk(result, finalLen);
    // result[11 + nameLength] = *(chk+0);
	// result[12 + nameLength] = *(chk+1);
	
	// LSB / MSB
	uint16_t chk = ti_chk(result, finalLen);
	result[11 + nameLength] = chk % 256;
	result[12 + nameLength] = chk >> 8;

                

    if (send) {
       int sent = ti_write(result, finalLen);
	   if (!false) { Serial.print("ti_header sent = "); Serial.print(sent); Serial.print(" / "); Serial.println(finalLen); }
	   resetLines();
    }
    dtLen = finalLen;
}

#define MSW(msg) ( (int) (msg >> 16) )
#define LSW(msg) ( (int) (msg & 65535) )
#define LSB( byt ) ( (int) (byt & 255) )
#define MSB( byt ) ( (int) (byt >> 8) )


void ti_xdp(char data[], int dataLen, int sendingMode, bool silent, int& dtLen, bool archived, Stream* input, bool inputIsSerial) {
	// packLen = 4 bytes for dataLen
	// + n for filename
	int packLen = 4 + dataLen;

	// finalLen is : 1 byte for machine ID
	// + 1 for cmd
	// + 2 for packLen
	// + packLen
	// + 2 for CHK
	int finalLen = 1 + 1 + 2 + packLen + 2;

    uint8_t result[8];

	// MACHINE ID
	result[0] = (char) (silent ? 0x09 : 0x08);
	// CMD
	result[1] = (char) 0x15;

	// packet length
	result[2] = (char) (packLen % 256);
	result[3] = (char) (packLen / 256);

	// dataLen :: TODO : ???
	result[4] = 0;
	result[5] = 0;
    result[6] = 0;
	result[7] = 0;
    // result[4] = LSB(LSW(dataLen));
    // result[5] = MSB(LSW(dataLen));
    // result[6] = LSB(MSW(dataLen));
    // result[7] = MSB(MSW(dataLen));

    uint16_t sum = 0;

    // head
    ti_write(result, 8);
    delay( DEFAULT_POST_DELAY/2 );

    // data
	// FIXME : better refacto

    // if ( sendingMode == SEND_MODE_RAM ) {
    //   sum = 0;
    //   for (int i = 0; i < dataLen; i++) { sum += (uint8_t)data[i];	}
    //   ti_send( (uint8_t*)data, dataLen);
    //   delay( DEFAULT_POST_DELAY/2 );
    // } else if ( sendingMode == SEND_MODE_FLASH ) {
    //   // Var len
    //   sum = 0;
    //   sum += (uint8_t)data[0];	
    //   sum += (uint8_t)data[1];	
    //   ti_send( (uint8_t*)data, 2);
    //   // outprintln(F("FLH > VarLen"));

    //   uint8_t D[1];
    //   for (int i = 0; i < dataLen-2; i++) { 
    //     D[0] = pgm_read_byte_near(FILE_CONTENT + VAR_FILE_DATA_OFFSET + i);

    //     #if MODE_92P_ASM
    //     if ( i == dataLen - 2 - 3 ) {
    //        D[0] = (uint8_t)( archived ? VAR_ARCHIVED_YES : VAR_ARCHIVED_NO );
    //     }
    //     #endif
        
    //     sum += (uint8_t)D[0];	
    //     ti_send( (uint8_t*)D, 1);
    //   }
    //   delay( DEFAULT_POST_DELAY/2 );
    // } else 
	if ( sendingMode == SEND_MODE_SERIAL ) {
      // Var len
      sum = 0;
      sum += (uint8_t)data[0];	
      sum += (uint8_t)data[1];	
      ti_write( (uint8_t*)data, 2);

      //const int BLOC_LEN =128;
      //uint8_t D[BLOC_LEN]; 
      const int BLOC_LEN =64;
      uint8_t* D = screen;
      int e;
      for (int i = 0; i < dataLen-2; i+=BLOC_LEN) { 

        e = BLOC_LEN;
        if ( i+BLOC_LEN > dataLen-2 ) { e = (dataLen-2)-i; }

if (inputIsSerial) {
       // request to send - handshake
       input->write( 0x02 );
       while( input->available() == 0 ) {delay(1);}
}

        // even if ends w/ garbages
       // serPort.readBytes( D, BLOC_LEN );
        input->readBytes( D, BLOC_LEN );

        for(int j=0; j < e; j++) { 
          // if ( i+j == dataLen - 2 - 3 ) {
          //   D[j] = (uint8_t)( archived ? VAR_ARCHIVED_YES : VAR_ARCHIVED_NO );
          // }
          sum += D[j];	
        } 
        ti_write( D, e);

		#if HAS_DISPLAY
		    int percent = ( 100 * i / dataLen );
			scLandscape();
			scLowerJauge( 100 );
			scRestore();
		#endif

      }
      //delay( DEFAULT_POST_DELAY/2 );
    } else {
      Serial.println(F("E:OUPS !!!"));
    } 

    delay( DEFAULT_POST_DELAY/2 );
	// 8+nameLength => 0
	// 9+nameLength => 0
    sum &= 0xFFFF;
	int checksum0 = (unsigned char)(sum % 256);
	int checksum1 = (unsigned char)(sum / 256);// & 0xFF;

    result[0] = checksum0;
    // temp Cf 2A 27 instead of 8C 27
    // result[0] = 0x8C; // 8C 27 two last bytes of file
    result[1] = checksum1;

    dtLen = finalLen;

    ti_write(result, 2);
    delay( DEFAULT_POST_DELAY );
    // outprintln(F("FLH > CHKSUM"));
    // DBUG(result, 2); // just to verify
}

// uint8_t* ti_chk(uint8_t b[], int len) {
// 	int sum = 0;
// 	for (int i = 4; i < len - 2; i++) {
// 		sum += b[i];
// 	}
// 	int checksum0 = sum % 256;
// 	int checksum1 = (sum / 256) & 0xFF;

//     // static -> the ONLY way to be sure to return correctly the given array (when converting to pointer)
//     static uint8_t result[2] = { (uint8_t) checksum0, (uint8_t) checksum1 };
// 	return result;
// }

uint16_t ti_chk(uint8_t b[], int len) {
	uint16_t sum = 0;
	for (int i = 4; i < len - 2; i++) {
		sum += b[i];
	}
	// int checksum0 = sum % 256;
	// int checksum1 = (sum / 256) & 0xFF;

    // // static -> the ONLY way to be sure to return correctly the given array (when converting to pointer)
    // static uint8_t result[2] = { (uint8_t) checksum0, (uint8_t) checksum1 };
	// return result;
	return sum;
}