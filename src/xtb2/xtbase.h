/**
 * XTBase language compiler / interpreter
 * 
 * Xtase - fgalliat @May 2020
 * 
 * Headers file
 * 
 */

#ifndef __XTBASE_H__
#define __XTBASE_H__ 1

  #include "globals.h"
  #include "xtbase/xtbase_struct.h"


  class XTBaseApp {
      private:
        int maxHeap, curHeap;
        int PC;
        bool inRun;
        bool firstTime;

        virtual uint8_t* loadFile(char* filename, int &len);
        virtual bool checkBreak();
        // virtual void doBreak();

        // ==== expr eval ====
        float getNumValue(XTBToken* tk);
        bool isNumValue(XTBToken* tk);
        bool isFctCall( XTBToken* tk );
        float fctCall( XTBToken** tokens, int nbTokens, int tkNum, int& tkNumRead );
        float factor(XTBToken** tokens, int nbTokens, int& tkNum);
        float term(XTBToken** tokens, int nbTokens, int& tkNum);
        float arith_expression(XTBToken** tokens, int nbTokens, int& tkNum);
        bool isConditionOperator(XTBToken* peeked);
        bool testCondition(float value, XTBToken* compOp, float value2);
        float bool_cond(XTBToken** tokens, int nbTokens, int& tkNum);
        float bool_factor(XTBToken** tokens, int nbTokens, int& tkNum);
        float bool_term(XTBToken** tokens, int nbTokens, int& tkNum);
        float expression(XTBToken** tokens, int nbTokens, int& tkNum);
        XTBToken* getUnit( XTBToken** tokens, int nbTokens, int tkNum, int &unitLen );
        
        // ==== Tokens ==========
        void resetToken(XTBToken* dest);
        void copyToken(XTBToken* dest, XTBToken* src);

        XTBToken* parseToken();
        XTBToken** analyse();
        int pAvailable();
        char pPeek();
        char pRead();
        char lower(char c);
        bool pEOL();
        bool pStringStart();
        bool pNumStart();
        bool pNumOperatorStart();
        bool pAssignStart();
        bool pBoolOperatorStart();
        bool pParenthesisStop();
        bool pParenthesisStart();
        bool pArgSeparator();
        bool pLetter();
        void consumeWhiteSpaces();
        char* consumeLetters();
        float consumeNumber();
        char* consumeString();
        // =====================

        bool initFirstTime();
        // void cleanLastTime();
        void freeArgs(Token** args, int length, bool force=false);

        void cleanStatements(bool force=false);
        void cleanVars();
        void cleanLabels();
        void cleanJumps();
        void initVars();
        void initLabels();
        void initStrMemSpace();

        XTBVariable* newVariable();
        XTBStatement* newStatement();
        XTBLabel* newLabel();
        XTBToken* newToken();
        XTBJump* newJump();

        int addStatement(int type, Token** tokens, int length);
        XTBLabel* addLbl(char* name, int addr);

        void my_free(void* ptr, int ptrSize=-1);
        int incHeapSize(int len);
        void* my_malloc(int len);


        void printStatement(XTBStatement* stmt);
        void printToken(XTBToken* tk);

        void load(const char* PRGM);
        void parseStatements();
        void parseStatementsJumps();
        void parseStatementsEnds();
        void printStatements();

        bool runStatements();
        bool treatStatement(XTBStatement* stmt);
        bool treatJumpStatement(XTBStatement* stmt);
        bool treatConditionalStatement(XTBStatement* stmt);
        bool treatLoopStatement(XTBStatement* stmt);
        bool treatRegularStatement(XTBStatement* stmt);

        XTBJump* pushGosub(XTBStatement* stmt);
        XTBJump* popGosub();

      public:
        XTBaseApp();
        ~XTBaseApp();

        virtual void doBreak();

        // ----- I/O ---------
        virtual void  print(const char* text);
        virtual void  println(const char* line);
        virtual void  print(int num);
        virtual void  println(int num);
        virtual void  print(float num);
        virtual void  println(float num);

        virtual void  cls();
        virtual void  pause();
        virtual float inputFloat(const char* prompt);

        virtual int menu(int itmn, char** itmv, char* title=NULL);
        virtual bool confirm(char* title, char* msg);
        virtual void alert(char* title, char* msg);

        void  error(const char* msg, int level);

        void jumpTo(int addr);
        void jumpTo(XTBLabel* tk);
        // -------------------

        char* loadScript(char* prgmName, int &len);

        bool compile(char* sourceCode, bool _printStatements=true);
        bool run();
        void gc(bool dispMemUsage=true);
        void reportMem();

        // ----- LINK --------
        bool link_recvBin();
        bool link_recvPRGM();
        bool link_sendPRGM(char* prgmName);
  };

  extern XTBaseApp xtbase;

#endif


