/**
 * XtsTiExpander V2
 * 
 * storage stack
 * 
 * Xtase - fgalliat @Jul 2021
 */

#include <Arduino.h>
#include "globals.h"

 // (!!) no real directory inSPIFFS
 // ex.
 // Listing directory: /
 // FILE: /logs/log.txt   SIZE: 14
 #define TIVAR_DIR "/tivars/"

// FIXME : do better
void Storage::format() {
    Serial.println("Formatting");
    SPIFFS.begin(true);
    Serial.println("done");
}

Storage::Storage() {}
Storage::~Storage() {}

bool storage_ready = false;

bool Storage::isReady() {
  return storage_ready;
}

bool Storage::begin() {
  storage_ready = false;
  if(! SPIFFS.begin(false) ){
      return false;
  }
  storage_ready = true;
  return true;
}

void Storage::end() {
}

void Storage::lsToStream(Stream* client, int shellMode) {
   if ( !storage_ready ) {
      client->println("No FileSystem mounted");
      return;
   }

   File root = SPIFFS.open("/");
   if(!root){
      client->println("− failed to open directory");
      return;
   }

   File file = root.openNextFile();
   while(file){
      if(file.isDirectory()){
         // .. never appens on SPIFFS
      } else {
         char* entryName = (char*)file.name();
         int from = strlen(TIVAR_DIR);

         // list only TiVars
         if ( startsWith( entryName, TIVAR_DIR ) ) {
            client->print( &entryName[from] );
            client->print(" (");
            client->print(file.size());
            client->println(")");

            // slow down in dummy mode
            if ( shellMode == SHELL_MODE_DUMMY ) {
               delay(150);
            }
         }
      }
      file = root.openNextFile();
   }
   client->println( "-EOL-" );
}

File Storage::createTiFile(const char * varName, uint8_t varType, int &error) {
  //  if ( !storage_ready ) {
  //     Serial.println("No FileSystem mounted");
  //     return;
  //  }

   char path[64+1]; memset( path, 0x00, 64+1 );
   sprintf( path, "%s%s.%02X", TIVAR_DIR, varName, varType );
   Serial.printf("Writing file: %s\r\n", path);

   File file = SPIFFS.open(path, FILE_WRITE);
   if(!file){
      Serial.println("− failed to open file for writing");
      error = 1;
      return file;
   }
   error = 0;
   return file;
}

char* Storage::findTiFile(const char * varName) {
  if ( !storage_ready ) {
      return NULL;
   }

   File root = SPIFFS.open("/");
   if(!root){
      return NULL;
   }

   char toFind[64+1]; memset( toFind, 0x00, 64+1 );
   sprintf(toFind, "%s%s.", TIVAR_DIR, varName);

   File file = root.openNextFile();
   while(file){
      if(file.isDirectory()){
         // .. never appens on SPIFFS
      } else {
         char* entryName = (char*)file.name();

         if ( startsWith( entryName, toFind ) ) {
            return entryName;
         }

      }
      file = root.openNextFile();
   }
   return NULL;
}

File Storage::getTiFile(const char* fullfilename) {
  char path[64+1]; memset( path, 0x00, 64+1 );
  // sprintf( path, "%s%s.%02X", TIVAR_DIR, varName, varType );
  // sprintf( path, "%s%s", TIVAR_DIR, fullfilename );
  sprintf( path, "%s", fullfilename );
  Serial.printf("Reading file: %s\r\n", path);

  File file = SPIFFS.open(path, FILE_READ);
  return file;
}