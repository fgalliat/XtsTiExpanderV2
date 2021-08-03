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
    public:
      Shell();
      ~Shell();
      
      void begin(Stream* client);
      void end();
      bool loop();
      bool handle(char* cmdline);

      bool isOpened();
      void setEcho(bool _echo=true);
};