/**
 * XtsTiExpander V2
 * 
 * Network stack
 * 
 * Xtase - fgalliat @Aug 2021
 */

#include <Arduino.h>
#include "globals.h"

#define WIFI_PSK_CONFIG "wifi.psk"

Network::Network() {}
Network::~Network() {}

void Network::start() {}
void Network::stop() {}

bool Network::addConfig(char* ssid, char* psk) { 
    int error = 0;
    File conf = storage.createConfFileAppend(WIFI_PSK_CONFIG, error);
    if ( error != 0 ) {
        // FIXME : WARNING code
        Serial.println("failed to create conf entry");
        return false;
    }
    conf.println(ssid);
    conf.println(psk);
    conf.flush();
    conf.close();
    return true; 
}

bool Network::isConnected() {return false;}

char* Network::getIP() {
    if ( isConnected() ) return "0.0.0.0";
    return NULL;
}
char* Network::getNetname() {
    if ( isConnected() ) return "NotConnected";
    return NULL;
}

