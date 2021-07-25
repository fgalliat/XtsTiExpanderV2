/**
 * XtsTiExpander V2
 * 
 * ti-link stack
 * 
 * Xtase - fgalliat @Jul 2021
 */

#define INPUT_BUFF_LEN 128

class TiLink : public Stream {
    public:
      TiLink();
      ~TiLink();
      
      bool begin(int tip, int ring);
      void end();

      void resetLines();

      void poll(bool isrMode=false); // ...

      // - Stream extension - 
      int available();
      int read();
      int peek();
      void flush();
      size_t write(uint8_t c);

};
