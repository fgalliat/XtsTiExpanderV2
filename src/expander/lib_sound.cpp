/**
 * XtsTiExpander V2
 * 
 * sound stack
 * 
 * Xtase - fgalliat @Jul 2021
 */

#include <Arduino.h>
#include "globals.h"

#include "hw_speaker_notes.h"

char charUpCase(char c) {
  if ( c >= 'a' && c <= 'z' ) { c = c - 'a' + 'A'; }
  return c;
}


Speaker::Speaker() {}
Speaker::~Speaker() {}

void Speaker::begin() {
    pinMode( BUZ_PIN, OUTPUT );
    digitalWrite( BUZ_PIN, LOW );
    int freq = 2000;
    ledcSetup(BUZ_channel, freq, BUZ_resolution);
    ledcAttachPin(BUZ_PIN, BUZ_channel);
}

void Speaker::tone(int freq) {
    int vol = 125;
    vol = 200;
    ledcWrite(BUZ_channel, vol); // volume
    ledcWriteTone(BUZ_channel, freq); // freq
}

void Speaker::noTone() {
    ledcWrite(BUZ_channel, 0); // volume
}

void Speaker::tone(int freq, long time) {
  tone(freq);
  delay(time);
  noTone();
}

void Speaker::playNote(int noteOrFreq, int duration) {
    if (this->_mute) { return; } 

    if ( noteOrFreq >= 1 && noteOrFreq <= 48 ) {
        // 0..48 octave2 to 5
        noteOrFreq = notes[ noteOrFreq-1 ];
    } else if ( noteOrFreq >= 49 && noteOrFreq <= 4096 ) {
        // 49..4096 -> 19200/note in Hz
        noteOrFreq *= 20;
    } else {
        noteOrFreq = 0;
    }

    this->noTone();
    this->tone( noteOrFreq, duration*50 );
    ::delay(duration*50);
    this->noTone(); // MANDATORY for ESP32
}

// ex. "AC#B"
void Speaker::playTuneString(const char* strTune) {
    if (this->_mute) { return; } 
    this->noTone();

    int defDuration = 200;
    int slen = strlen( strTune );

    for (int i=0; i < slen; i++) {
        char ch = strTune[i];
        ch = charUpCase(ch);
        bool sharp = false;
        if ( i+1 < slen && strTune[i+1] == '#' ) { 
            sharp = true; 
            i++; 
        }  

        int pitch = 0;
        switch (ch) {
            case 'C' :
                if ( sharp ) { pitch = notes[ NOTE_CS4 ]; }
                else pitch = notes[ NOTE_C4 ];
                break;
            case 'D' :
                if ( sharp ) { pitch = notes[ NOTE_DS4 ]; }
                else pitch = notes[ NOTE_D4 ];
                break;
            case 'E' :
                pitch = notes[ NOTE_E4 ];
                break;
            case 'F' :
                if ( sharp ) { pitch = notes[ NOTE_FS4 ]; }
                else pitch = notes[ NOTE_F4 ];
                break;
            case 'G' :
                if ( sharp ) { pitch = notes[ NOTE_GS4 ]; }
                else pitch = notes[ NOTE_G4 ];
                break;
            case 'A' :
                if ( sharp ) { pitch = notes[ NOTE_AS4 ]; }
                else pitch = notes[ NOTE_A4 ];
                break;
            case 'B' :
                pitch = notes[ NOTE_B4 ];
                break;
        }

        this->tone(pitch, defDuration);
        ::delay(defDuration);
    }
    this->noTone(); // MANDATORY for ESP32
}
