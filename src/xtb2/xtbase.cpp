/**
 * XTBase language compiler / interpreter
 * 
 * Xtase - fgalliat @May 2020
 * 
 * Headers file
 * 
 */

#include "globals.h"

#include "xtbase.h"

#if ( ABSTRACT_MODE == ABSTRACT_TTY )

  void waitAllBtnsReleased() { ; }

  #include "xtb_abstract/xtbase_tty.h"
  #include "xtb_abstract/xtbase_link_tty.h"
#elif ( ABSTRACT_MODE == ABSTRACT_M5 )
  #include "xtb_abstract/xtbase_m5.h"
  #include "xtb_abstract/xtbase_link_m5.h"
#endif

#include "xtbase/xtbase_colls.h"
#include "xtbase/xtbase_enums.h"
#include "xtbase/xtbase_utils.h"

#include "xtbase/xtbase_fcts.h"

#include "xtbase/xtbase_expreval.h"
#include "xtbase/xtbase_analyse.h"
#include "xtbase/xtbase_stmts.h"
#include "xtbase/xtbase_jumps.h"

#include "xtbase/xtbase_menu.h"

XTBaseApp::XTBaseApp() {
    curHeap = 0;
    maxHeap = 0;
    firstTime = true;

    initFirstTime();
}

XTBaseApp::~XTBaseApp() {
    gc( false );
}

char* XTBaseApp::loadScript(char* prgmName, int &len) {
    char filename[128+1]; memset(filename, 0x00, 128+1);
    #if ( ABSTRACT_MODE == ABSTRACT_TTY )
    sprintf( filename, "./script/%s.xtp", prgmName  );
    #elif ( ABSTRACT_MODE == ABSTRACT_M5 )
    sprintf( filename, "/script/%s.xtp", prgmName  );
    #endif
    return (char*)loadFile(filename, len);
}

// ========= Instanciation ================= 

XTBVariable* XTBaseApp::newVariable() {
// printf("(ii) NEW VAR\n");
    incHeapSize( sizeof( XTBVariable ) );
    return new XTBVariable();
}

XTBLabel* XTBaseApp::newLabel() {
// printf("(ii) NEW LBL\n");
    incHeapSize( sizeof(XTBLabel) );
    return new XTBLabel();
}

XTBToken* XTBaseApp::newToken() {
// printf("(ii) NEW TOKEN\n");
    incHeapSize( sizeof(XTBToken) );
    XTBToken* tk = new XTBToken();
    tk->toBeDestroyed = false;
    return tk;
}

XTBStatement* XTBaseApp::newStatement() {
    incHeapSize( sizeof(XTBStatement) );
    return new XTBStatement();
}

// ========= Initialization ================= 

void XTBaseApp::initVars() {
    for(int i=0; i < XTB_NB_VAR_MAX; i++) {
        if ( xtb_vars[i] == NULL ) { xtb_vars[i] = newVariable(); }
        
        xtb_vars[i]->index = i;
    }
    xtb_varCursor = 0;
}

void XTBaseApp::initLabels() {
    for(int i=0; i < XTB_NB_LBL_MAX; i++) {
        if ( xtb_lbls[i] == NULL ) { xtb_lbls[i] = newLabel(); }
        xtb_lbls[i]->index = i;
        memset(xtb_lbls[i]->name, 0x00, 8+1);
    }
    xtb_lblCursor = 0;
}

void XTBaseApp::initStrMemSpace() {
  memset( xtb_strMemSpace, 0x00, XTB_NB_STRINGS_MAX * XTB_STRING_MAX_LEN );
  xtb_strMemCursor = 0;
}

bool XTBaseApp::initFirstTime() {
    if ( !firstTime ) {
        return true;
    }

    for(int i=0; i < XTB_NB_KEYWORD; i++) {
        xtb_KeyWords_lens[i] = strlen( xtb_KeyWords[i] );
    }
    for(int i=0; i < XTB_NB_FUNCTION; i++) {
        xtb_functions_lens[i] = strlen( xtb_functions[i] );
    }
    for(int i=0; i < XTB_NB_BCOND; i++) {
        xtb_boolCond_lens[i] = strlen( xtb_boolConds[i] );
    }

    for(int i=0; i < XTB_TOKENIZED_PRG_LEN; i++) {
        xtb_tokenized_prgm[i] = ( newToken() );
    }

    initVars();
    initLabels();

    firstTime = false;
    return true;
}

// ========= Garbage Collector ================= 
void XTBaseApp::cleanVars() {
    int nbToClean = 0;
    for(int i=0; i < XTB_NB_VAR_MAX; i++) {
        if ( xtb_vars[i] != NULL ) { my_free( xtb_vars[i] ); xtb_vars[i] = NULL; nbToClean++; }
    }
    xtb_varCursor = 0;
}

void XTBaseApp::cleanLabels() {
    int nbToClean = 0;
    for(int i=0; i < XTB_NB_LBL_MAX; i++) {
        if ( xtb_lbls[i] != NULL ) { my_free( xtb_lbls[i] ); xtb_lbls[i] = NULL; nbToClean++; }
    }
    xtb_lblCursor = 0;
}

void XTBaseApp::cleanStatements(bool force) {
    for(int i=0; i < XTB_MAX_STATEMENTS; i++) {
        if ( xtb_statements[i] != NULL ) {
            int t_size = xtb_statements[i]->length * sizeof( Token* );
            xtb_statements[i]->type = XTB_STMT_REGULAR;
            xtb_statements[i]->dest = 0;
            if ( xtb_statements[i]->tokens != NULL ) {
                for (int j=0; j < xtb_statements[i]->length; j++) {
                    if ( xtb_statements[i]->tokens[j] != NULL &&
                         xtb_statements[i]->tokens[j]->toBeDestroyed ) {
                        my_free( xtb_statements[i]->tokens[j] );
                        xtb_statements[i]->tokens[j] = NULL;
                    }
                }
                my_free( xtb_statements[i]->tokens, t_size );
                xtb_statements[i]->tokens = NULL;
            }
            xtb_statements[i]->length = 0;

            if ( force ) {
                my_free( xtb_statements[i] );
                xtb_statements[i] = NULL;
            }
        }
    }
    xtb_nbStatements = 0;
}

void XTBaseApp::gc(bool dispMemUsage) {
    for(int i=0; i < XTB_TOKENIZED_PRG_LEN; i++) {
        if ( xtb_tokenized_prgm[i] != NULL ) {
            my_free( xtb_tokenized_prgm[i] );
            xtb_tokenized_prgm[i] = NULL;
        }
    }

    // println("Z._before "); reportMem();
    cleanVars();
    // println("X.vars "); reportMem();
    cleanLabels();
    // println("A.jump "); reportMem();
    cleanJumps();
    // println("B.jump "); reportMem();
    cleanStatements( true );
    // println("C.stmt "); reportMem();

    firstTime = true;
    // ....
    if (dispMemUsage) { reportMem(); }
}

// ========== Memory managment ===============

int XTBaseApp::incHeapSize(int len) {
    curHeap += len;
    if (curHeap > maxHeap) { maxHeap = curHeap; } 
    return curHeap;
}

void* XTBaseApp::my_malloc(int len) {
    incHeapSize( len );
    return malloc(len);
}

void XTBaseApp::my_free(void* ptr, int ptrSize) {
    if ( ptr == NULL ) { return; }
    int freed = ptrSize == -1 ? sizeof( ptr ) : ptrSize;
    //int freed = sizeof( ptr );
    // int freed = 4;
    curHeap -= freed;
    free(ptr);
}



char memUsageMsg[128+1];
void XTBaseApp::reportMem() {
    memset(memUsageMsg, 0x00, 128+1);
    sprintf(memUsageMsg, "== Mem Usage %d / %d ==", curHeap, maxHeap);
    println( (const char*)memUsageMsg );
}

void XTBaseApp::error(const char* msg, int level) {
    println("---------------------");
    print("| "); println(msg);
    println("---------------------");
    if ( inRun ) {
        printStatement( xtbGetStatement( PC ) );
    }

    #if ( ABSTRACT_MODE == ABSTRACT_M5 )
    Serial.println("---------------------");
    Serial.print("| "); Serial.println(msg);
    Serial.println("---------------------");
    #endif
    // pause();

    inRun = false;
    PC = -100;
}

// ======

const int tokenDispMsgLen = 128; // FIXME
char tokenDispMsg[tokenDispMsgLen+1];
void XTBaseApp::printToken(XTBToken* tk) {
    memset(tokenDispMsg, 0x00, tokenDispMsgLen+1);
    sprintf(tokenDispMsg, "Token type : %02x (%s)\t", tk->tkType, xtb_token_type[ tk->tkType ] );

    if ( tk->tkType == XTB_TK_STR ) {
        sprintf(tokenDispMsg, "%s  val : \"%s\" \n", tokenDispMsg, xtbGetString( tk ) );
    } else if ( tk->tkType == XTB_TK_KEYW ) {
        sprintf(tokenDispMsg, "%s  val : %s \n", tokenDispMsg, xtb_KeyWords[ tk->iValue ] );
    } else if ( tk->tkType == XTB_TK_FCT ) {
        sprintf(tokenDispMsg, "%s  val : %s \n", tokenDispMsg, xtb_functions[ tk->iValue ] );
    } else if ( tk->tkType == XTB_TK_NUM ) {
        sprintf(tokenDispMsg, "%s  val : %g \n", tokenDispMsg, tk->fValue );
    } else if ( tk->tkType == XTB_TK_OPR ) {
        sprintf(tokenDispMsg, "%s  val : %c \n", tokenDispMsg, (char)tk->iValue );
    } else if ( tk->tkType == XTB_TK_OPR_BOOL ) {
        sprintf(tokenDispMsg, "%s  val : %s (%s) \n", tokenDispMsg, xtb_boolConds[ tk->iValue ], isConditionOperator(tk) ? "comp" : "oper" );
    } else if ( tk->tkType == XTB_TK_VAR ) {
        sprintf(tokenDispMsg, "%s  val : %s \n", tokenDispMsg, xtbGetVar( tk )->name );
    } else if ( tk->tkType == XTB_TK_EOL || tk->tkType == XTB_TK_ARGSEP ) {
        sprintf(tokenDispMsg, "%s\n", tokenDispMsg);
    } else if ( tk->tkType == XTB_TK_PARENTOP || tk->tkType == XTB_TK_PARENTCL ) {
        sprintf(tokenDispMsg, "%s\n", tokenDispMsg);
    } else if ( tk->tkType == XTB_TK_ASSIGN ) {
        sprintf(tokenDispMsg, "%s\n", tokenDispMsg);
    } else {
        sprintf(tokenDispMsg, "%s  val : %d\n", tokenDispMsg,tk->iValue);
        sprintf(tokenDispMsg, "%s  val : %g\n", tokenDispMsg,tk->fValue);
        sprintf(tokenDispMsg, "%s    @ : %d\n", tokenDispMsg,tk->addr);
    }

    print( tokenDispMsg );
}

void XTBaseApp::printStatement(XTBStatement* stmt) {
    println("===========================");
    print("= Statement ");

    if ( stmt == NULL ) {
        println("NULL");
        return;
    }

    print(stmt->addr);
    print(" type ");
    println(xtb_stmtTypes[stmt->type]);

    if ( stmt->type == XTB_STMT_JUMP ) {
        if ( stmt->dest < 0 ) {
            println("= Stacked jump ");
        } else {
            print("= Jump goes to : ");
            println( stmt->dest );
        }
    } else {
        // End, For, If, ...
        if ( stmt->dest >= 0 ) {
            print("= Statement linked to : ");
            println( stmt->dest );
        }
    }

    for(int i=0; i < stmt->length; i++) {
        if ( stmt->tokens[i] == NULL ) {
            println("NULL token");
        } else {
            printToken( stmt->tokens[i] );
        }
    }

    println("===========================");
}

// ========== Jumps ===============

void XTBaseApp::jumpTo(int addr) { PC = addr - 1; }
void XTBaseApp::jumpTo(XTBLabel* lbl) { jumpTo(lbl->addr); }


// ========== Code Treatment ===============

void XTBaseApp::load(const char* PRGM) {
    initFirstTime();
    initStrMemSpace();

    memset(xtb_prgmSource, 0x00, XTB_SOURCE_MAX_LEN);
    xtb_prgmSourceCursor = 0;
    xtb_prgmSourceLen = 0;

    int tlen = 0;
    if ( PRGM != NULL ) { 
        tlen = strlen( PRGM );
        memcpy(xtb_prgmSource, PRGM, tlen );
        xtb_prgmSourceLen = tlen;
    }
    print("Loaded ");
    print(tlen);
    println(" bytes.");
}

void XTBaseApp::doBreak() {
    error("Aborted by User", 1);
    waitAllBtnsReleased();
    // pause();
}

bool XTBaseApp::runStatements() {
    PC = 0;
    inRun = true;
    while( inRun ) {

        if ( checkBreak() ) {
            doBreak();
            return false;
        }

        if ( PC < 0 ) {
            inRun = false;
            // error("There was an error",1);
            return false;
        }

        if ( PC >= xtb_nbStatements ) {
            inRun = false;
            println("-EOP-");
            return true;
        }

        if ( treatStatement( xtb_statements[PC] ) ) {
            PC++;
            // return true; // ???????
        } else {
            inRun = false;
            error("There was an error (2)",2);
            pause();
            return false;
        }

        yield();
    }

    return true; // ?
}

bool XTBaseApp::compile(char* sourceCode, bool _printStatements) {
    load(sourceCode);

    XTBToken** tokens = analyse();
    if ( tokens == NULL ) {
        error("Analysis failed (0x01)", 1);
        return false;
    }

    parseStatements();
    parseStatementsJumps();
    parseStatementsEnds();
    if (_printStatements) {
        printStatements();
    }

    return true;
}

bool XTBaseApp::run() {
    println("====================================");
    runStatements();
    // reportMem();
    return true;
}



