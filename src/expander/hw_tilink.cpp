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

    __resetTILines(true);

    return true;
}

void __resetTILines(bool reboot) {
  pinMode(TIring, INPUT);           // set pin to input
  pinMode(TItip, INPUT);            // set pin to input
  if (reboot) {
    digitalWrite(TIring, LOW);       // for reset purposes
    digitalWrite(TItip, LOW);        // for reset purposes
  }
  digitalWrite(TIring, HIGH);       // turn on pullup resistors
  digitalWrite(TItip, HIGH);        // turn on pullup resistors
}

int ti_write(uint8_t b) {
    long previousMillis = 0;
    int bit;
    uint8_t byte;
    for (bit = 0; bit < 8; bit++) {
      previousMillis = 0;

      while ((digitalRead(TIring) << 1 | digitalRead(TItip)) != 0x03) {
        // delay(1);
        if (previousMillis++ > TIMEOUT) {
          // Serial.print("FAILED @ BYTE "); Serial.println(j);
          return -1;
          //return ERR_WRITE_TIMEOUT + j + 100 * bit;
        }
      };

      if (byte & 1) {
        pinMode(TIring, OUTPUT);
        digitalWrite(TIring, LOW);
        previousMillis = 0;
        while (digitalRead(TItip) == HIGH) {
          if (previousMillis++ > TIMEOUT)
            return -2;
           // return ERR_WRITE_TIMEOUT + 10 + j + 100 * bit;
        };

        __resetTILines();
        previousMillis = 0;
        while (digitalRead(TItip) == LOW) {
          if (previousMillis++ > TIMEOUT)
            return -3;
           // return ERR_WRITE_TIMEOUT + 20 + j + 100 * bit;
        };
      } else {
        pinMode(TItip, OUTPUT);
        digitalWrite(TItip, LOW);     //should already be set because of the pullup resistor register
        previousMillis = 0;
        while (digitalRead(TIring) == HIGH) {
          if (previousMillis++ > TIMEOUT)
            return -4;
            //return ERR_WRITE_TIMEOUT + 30 + j + 100 * bit;
        };

        __resetTILines();
        previousMillis = 0;
        while (digitalRead(TIring) == LOW) {
          if (previousMillis++ > TIMEOUT)
          return -5;
            //return ERR_WRITE_TIMEOUT + 40 + j + 100 * bit;
        };
      }

      byte >>= 1;
    }
    return 0;
}

// default : GET_ENTER_TIMEOUT
int ti_read(long timeout) {
    long previousMillis = 0;
    int bit;
    uint8_t v, byteout = 0;
    for (bit = 0; bit < 8; bit++) {
      previousMillis = 0;
      /**
       * 
       * //Interrupt Modes
        #define RISING    0x01
        #define FALLING   0x02
        #define CHANGE    0x03 <--
        #define ONLOW     0x04
        #define ONHIGH    0x05
        #define ONLOW_WE  0x0C
        #define ONHIGH_WE 0x0D
       * 
       */
      while ((v = (digitalRead(TIring) << 1 | digitalRead(TItip))) == 0x03) {
        if (previousMillis++ > timeout) {
            // return ERR_READ_TIMEOUT + j + 100 * bit;
            return -1;
        }
      }
      if (v == 0x01) {
        byteout = (byteout >> 1) | 0x80;
        pinMode(TItip, OUTPUT);
        digitalWrite(TItip, LOW);     //should already be set because of the pullup resistor register
        previousMillis = 0;
        while (digitalRead(TIring) == LOW) {            //wait for the other one to go low
          if (previousMillis++ > TIMEOUT) {
            // return ERR_READ_TIMEOUT + 10 + j + 100 * bit;
            return -11;
          }
        }
        //pinMode(TIring,OUTPUT);
        digitalWrite(TIring, HIGH);
      } else {
        byteout = (byteout >> 1) & 0x7F;
        pinMode(TIring, OUTPUT);
        digitalWrite(TIring, LOW);     //should already be set because of the pullup resistor register
        previousMillis = 0;
        while (digitalRead(TItip) == LOW) {
          if (previousMillis++ > TIMEOUT) {
            // return ERR_READ_TIMEOUT + 20 + j + 100 * bit;
            return -10;
          }
        }
        //pinMode(TItip,OUTPUT);
        digitalWrite(TItip, HIGH);
      }
      pinMode(TIring, INPUT);           // set pin to input
      digitalWrite(TIring, HIGH);       // turn on pullup resistors
      pinMode(TItip, INPUT);            // set pin to input
      digitalWrite(TItip, HIGH);        // turn on pullup resistors
    }
    return byteout;
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

