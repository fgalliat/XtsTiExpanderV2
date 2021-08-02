/**
 * XtsTiExpander V2
 * 
 * storage stack
 * 
 * Xtase - fgalliat @Jul 2021
 */

#include "FS.h"
#include "SPIFFS.h"

class Storage {
    private:
      void format();
    public:
      Storage();
      ~Storage();
      
      bool begin();
      void end();
      bool isReady();

      // // ex. writeFile(SPIFFS, "toto.txt", "Hello World !");
      // void writeFile(fs::FS &fs, const char * path, const char * message);
      // void appendFile(fs::FS &fs, const char * path, const char * message);
      // void readFile(fs::FS &fs, const char * path, Stream* output);

      long diskUsage();
      long diskSpace();
      long diskFree();

      void lsToStream(Stream* client, int shellMode);
      char** lsToArray(int& size, bool sort);
      void lsToScreen();

      // TiVar specific
      File createTiFile(const char * varName, uint8_t varType, int &error);
      char* findTiFile(const char * varName);
      File getTiFile(const char* fullfilename);

      // Config specific
      File createConfFileAppend(const char * confName, int &error);
      File getConfFileRead(const char * confName, int &error);
      void eraseConfFile(const char * confName);
};