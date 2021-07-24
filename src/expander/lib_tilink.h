/**
 * XtsTiExpander V2
 * 
 * ti-link stack
 * 
 * Xtase - fgalliat @Jul 2021
 */

class TiLink : public Stream {
    public:
      TiLink();
      ~TiLink();
      
      bool begin();
      void end();

      void poll(); // ...

      // - Stream extension - 
      int available();
      int read();
      int peek();
      void flush();
      size_t write(uint8_t c);

};
