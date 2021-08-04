/**
 * XtsTiExpander V2
 * 
 * Ti Software Streams stack
 * 
 * Xtase - fgalliat @Aug 2021
 */

#define DUMMY_INPUT_LEN 128

// in DummyMode
class TiDummyStream : public Stream {
  private:
    char inputBuff[DUMMY_INPUT_LEN+1];
    int inputBuffCursor = 0;
    void resetInputBuff();
    bool opened = false;
    bool loopDisabled = false;
  public:
    TiDummyStream();
    ~TiDummyStream();
    
    bool begin();
    void end();

    bool isOpened();
    bool isLoopDisabled();
    void setLoopDisabled(bool state);

    bool appendToInputBuffer(char ch);

    // - Stream extension - 
    int available();
    int read();
    int peek();
    void flush();
    size_t write(uint8_t c);
};


// sends KeyStroke, reads CBL datas
class TiNativeStream : public Stream {
  public:
    TiNativeStream();
    ~TiNativeStream();
    
    bool begin();
    void end();

    // - Stream extension - 
    int available();
    int read();
    int peek();
    void flush();
    size_t write(uint8_t c);
};