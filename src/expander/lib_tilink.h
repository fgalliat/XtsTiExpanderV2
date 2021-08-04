/**
 * XtsTiExpander V2
 * 
 * ti-link stack
 * 
 * Xtase - fgalliat @Jul 2021
 */

#define INPUT_BUFF_LEN 128

class TiLink : public Stream {
    private:
      void dummyMode(); // Deprecated

      bool inDummyMode = false;
      void enterDummyMode();
      void exitDummyMode();
      bool loopDummyMode();
    public:
      TiLink();
      ~TiLink();
      
      bool begin(int tip, int ring);
      void end();

      void resetLines(bool restoreLines=false);

      bool requestScreen(Stream* out, bool asciiMode = false); 
      int sendKeyStrokes(char* data, int len=-1);
      int sendKeyStroke(int data);
      bool sendVar(char* varName, bool silent=true);

      bool handleCalc();

      bool waitAvailable(long timeout=1000);
      void poll(bool isrMode=false); // ...

      // - Stream extension - 
      int available();
      int read();
      int peek();
      void flush();
      size_t write(uint8_t c);

};
