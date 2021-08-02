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

// =========================================
#include <WiFi.h>
#include <WiFiMulti.h>

WiFiMulti wifiMulti;

//how many clients should be able to telnet to this ESP32
#define MAX_SRV_CLIENTS 1

WiFiServer server(23);
WiFiClient serverClients[MAX_SRV_CLIENTS];

bool WIFI_READY = false;
// =========================================

Network::Network() {}
Network::~Network() {}

void Network::start() {
  if ( WIFI_READY ) { return; }
  connected = false;

  int error = 0;
  File conf = storage.getConfFileRead(WIFI_PSK_CONFIG, error);
  if ( error != 0 || conf.available() == 0 ) {
      // FIXME : display
      Serial.println("No available WiFi config");
      return;
  }


  // FIXME : display
//   displayCls();
//   displayPrintln("Connecting to WiFi");
//   displayBlitt();
  Serial.println("Connecting to WiFi");
  
//   while (conf.available() > 0) {
//       Serial.write( conf.read() );
//   }
//   conf.seek(0);

  while (conf.available() > 0) {
      char ssid[64+1]; memset( ssid, 0x00, 64+1 );
      conf.readBytesUntil('\n', ssid, 64);
      if ( ssid[strlen(ssid)-1] == '\n' ) { ssid[strlen(ssid)-1] = 0x00; }
      if ( ssid[strlen(ssid)-1] == '\r' ) { ssid[strlen(ssid)-1] = 0x00; }
      char psk[64+1]; memset( psk, 0x00, 64+1 );
      conf.readBytesUntil('\n', psk, 64);
      if ( psk[strlen(psk)-1] == '\n' ) { psk[strlen(psk)-1] = 0x00; }
      if ( psk[strlen(psk)-1] == '\r' ) { psk[strlen(psk)-1] = 0x00; }

      Serial.print("try:");Serial.print(ssid);/*Serial.print("/");Serial.print(psk);*/Serial.println();
      wifiMulti.addAP(ssid, psk);
  }
  conf.close();
  
  for (int loops = 10; loops > 0; loops--) {
    if (wifiMulti.run() == WL_CONNECTED) {
      memset(curIp, 0x00, 16+1);
      memset(curSSID, 0x00, 64+1);
      const char* ipAddr = WiFi.localIP().toString().c_str();
      sprintf( curIp, "%s", ipAddr );
      const char* ssidName = WiFi.SSID().c_str();
      sprintf( curSSID, "%s", ssidName );

      // FIXME : display
    //   displayPrintln( (char*)ipAddr );
    //   displayBlitt();
      Serial.println((char*)ipAddr);

      connected = true;
      WIFI_READY = true;
      break;
    }
    else {
      // Serial.println(loops);
      delay(1000);
    }
  }
  if (wifiMulti.run() != WL_CONNECTED) {
      // FIXME : display
    // displayPrintln( "WiFi connect failed" );
    // displayBlitt();
    Serial.println( "WiFi connect failed" );
    WIFI_READY = false;
    return;
  }

  server.begin();
  server.setNoDelay(true);
}

void Network::stop() {
  if ( ! WIFI_READY ) { return; }

    // FIXME : display
//   displayCls();
//   displayPrintln("Closing WiFi");
//   displayBlitt();
Serial.print("Closing WiFi");

  for(int i = 0; i < MAX_SRV_CLIENTS; i++) {
    if (serverClients[i]) serverClients[i].stop();
  }
  server.close();
  delay(500);
  // wifiMulti.disconnect(); // need to delete ref ... WiFiMulti::~WiFiMulti()

  // FIXME : display
//   displayPrintln("Closed WiFi");
//   displayBlitt();
  Serial.print("Closed WiFi");
  connected = false;
  WIFI_READY = false;
}

void Network::loop() {
    if ( !isConnected() ) { return; }
    loopTelnet();
}

// FIXME : move to main code ...
/** return true if sessionStream/Mode should be closed */
bool sessionLoop(Stream* client, int shellMode) {
    client->println("Welcome !");
    client->println("Bye");
    return true;
}


void Network::loopTelnet() {
    if (wifiMulti.run() != WL_CONNECTED) {
        return;
    }
    int i;

    //check if there are any new clients
    if (server.hasClient()){
      for(i = 0; i < MAX_SRV_CLIENTS; i++){
        //find free/disconnected spot
        if (!serverClients[i] || !serverClients[i].connected()){
          if(serverClients[i]) serverClients[i].stop();
          serverClients[i] = server.available();
          if (!serverClients[i]) Serial.println("available broken");
          Serial.print("New client: ");
          Serial.print(i); Serial.print(' ');
          Serial.println(serverClients[i].remoteIP());
          break;
        }
      }
      if (i >= MAX_SRV_CLIENTS) {
        //no free/disconnected spot so reject
        server.available().stop();
      }
    }
    //check clients for data
    for(i = 0; i < MAX_SRV_CLIENTS; i++){
      if (serverClients[i] && serverClients[i].connected()){
        // if(serverClients[i].available()){
        //   //get data from the telnet client and push it to the UART
        //   while(serverClients[i].available()) Serial1.write(serverClients[i].read());
        // }

        // serverClients[i].write(sbuf, len);

        // // --------- sequential session ---------
        // // read potential connection termcap ...
        // while(serverClients[i].available()) serverClients[i].read();

        // serverClients[i].println( "Hello telnet stranger" );
        // while(serverClients[i].available() == 0) { delay(10); }
        // serverClients[i].println( "Bye" );
        // serverClients[i].stop();
        // // --------- sequential session ---------

        bool shouldKill = sessionLoop( &serverClients[i], SHELL_MODE_TELNET);
        if ( shouldKill ) {
          serverClients[i].stop();
        }

      }
      else {
        if (serverClients[i]) {
          serverClients[i].stop();
        }
      }
    }
}

void Network::eraseConfig() {
    storage.eraseConfFile(WIFI_PSK_CONFIG);
}

bool Network::addConfig(char* ssid, char* psk) { 
    int error = 0;
    File conf = storage.createConfFileAppend(WIFI_PSK_CONFIG, error);
    if ( error != 0 ) {
        // FIXME : WARNING code
        Serial.println("failed to create conf entry");
        return false;
    }
    // FIXME : display
    Serial.println("Will add :");
    Serial.println(ssid);
    Serial.println(psk);

    conf.println(ssid);
    conf.println(psk);
    conf.flush();
    conf.close();
    return true; 
}

bool Network::isConnected() { return connected; }

char* Network::getIP() {
    if ( !isConnected() ) return "0.0.0.0";
    return curIp;
}
char* Network::getNetname() {
    if ( !isConnected() ) return "NotConnected";
    return curSSID;
}

