/**
 * XtsTiExpander V2
 * 
 * Shell stack
 * 
 * Xtase - fgalliat @Aug 2021
 */

#define SHELL_LINE_MAX 128

class Shell {
    private:
      Stream* curClient = NULL;
      bool opened = false;
      bool echo = true;

      char curline[SHELL_LINE_MAX+1];
      int curlineCursor = 0;
      void resetCurLine();
      bool appendChar(char ch);
      int containsBR();

      // blocking method
      char* readLine(bool echo=true);
      void hexDump(uint8_t* data, int dataLen, bool asciiToo);
      
      bool handleVarRecv(); // var to expander
      bool handleVarSend(char* varname); // var from expander
    public:
      Shell();
      ~Shell();
      
      void begin(Stream* client);
      void end();
      bool loop();
      bool handle(char* cmdline);

      bool isOpened();
      void setEcho(bool _echo=true);

      bool hexVar(char* varName);
      bool delVar(char* varName);

      bool emergency();
};
