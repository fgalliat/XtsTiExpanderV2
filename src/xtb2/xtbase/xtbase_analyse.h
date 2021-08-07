/**
 * XTBase language compiler / interpreter
 * 
 * Xtase - fgalliat @May 2020
 * 
 * Token analyser
 * 
 */

#include "../globals.h"
#include "../xtbase.h"

void XTBaseApp::resetToken(XTBToken* dest) {
    dest->tkType = XTB_TK_EOP;
    dest->addr   = 0;
    dest->iValue = 0;
    dest->fValue = 0.0;
}

void XTBaseApp::copyToken(XTBToken* dest, XTBToken* src) {
    dest->tkType = src->tkType;
    dest->addr   = src->addr;
    dest->iValue = src->iValue;
    dest->fValue = src->fValue;
}

int XTBaseApp::pAvailable() {
    return xtb_prgmSourceLen - xtb_prgmSourceCursor;
}

char XTBaseApp::pPeek() {
    if ( pAvailable() <= 0 ) { return 0xFF; }
    return xtb_prgmSource[xtb_prgmSourceCursor];
}

char XTBaseApp::pRead() {
    if ( pAvailable() <= 0 ) { return 0xFF; }
    return xtb_prgmSource[xtb_prgmSourceCursor++];
}

char XTBaseApp::lower(char c) {
    if ( c >= 'A' && c <= 'Z' ) {
        c = c - 'A' + 'a';
    }
    return c;
}


bool XTBaseApp::pEOL() {
    if ( pPeek() == ':' || pPeek() == '\n' ) { return true; }
    return false;
}

bool XTBaseApp::pStringStart() {
    if ( pPeek() == '"' ) { return true; }
    return false;
}

bool XTBaseApp::pNumStart() {
    char peeked = pPeek();
    if ( peeked == '.' || ( peeked >= '0' && peeked <= '9' ) ) { return true; }
    return false;
}

bool XTBaseApp::pNumOperatorStart() {
    char peeked = pPeek();
    if ( peeked == '-' || peeked == '+' || peeked == '*' || peeked == '/' || peeked == '%' ) { return true; }
    return false;
}

bool XTBaseApp::pAssignStart() {
    char peeked = pPeek();
    if ( peeked == '=' ) { return true; }
    return false;
}

bool XTBaseApp::pBoolOperatorStart() {
    char peeked = pPeek();
    if ( peeked == '=' || peeked == '!' || peeked == '<' || peeked == '>' || peeked == '&' || peeked == '|' ) { return true; }
    return false;
}

bool XTBaseApp::pParenthesisStop() {
    char peeked = pPeek();
    if ( peeked == ')' ) { return true; }
    return false;
}

bool XTBaseApp::pParenthesisStart() {
    char peeked = pPeek();
    if ( peeked == '(' ) { return true; }
    return false;
}

bool XTBaseApp::pArgSeparator() {
    char peeked = pPeek();
    if ( peeked == ',' ) { return true; }
    return false;
}


bool XTBaseApp::pLetter() {
    char peeked = lower( pPeek() );
    if ( peeked >= 'a' && peeked <= 'z' ) { return true; }
    return false;
}


void XTBaseApp::consumeWhiteSpaces() {
    while ( pPeek() == ' ' ) { 
        pRead();
    }
}

char letters[16+1];
char* XTBaseApp::consumeLetters() {
    memset(letters, 0x00, 16+1);
    int cur=0;
    while ( pLetter() ) { 
        letters[ cur++ ] = lower( pRead() );
    }
    return letters;
}

char number[16+1];
float XTBaseApp::consumeNumber() {
    memset(number, 0x00, 16+1);
    int cur=0;
    while ( pNumStart() ) { 
        number[ cur++ ] = pRead();
    }
    return atof( number );
}



char string[XTB_STRING_MAX_LEN+1];
char* XTBaseApp::consumeString() {
    memset(string, 0x00, XTB_STRING_MAX_LEN+1);
    int cur=0;
    if ( pStringStart() ) {
        pRead();
    } else {
        return NULL;
    }
    while ( ! pStringStart() ) { 
        if ( pAvailable() <= 0 ) {
            return NULL;
        }
        string[ cur++ ] = pRead();
        if ( cur >= XTB_STRING_MAX_LEN ) {
            return NULL;
        }
    }
    pRead(); // trailing '"'
    return string;
}



XTBToken curToken;

XTBToken* XTBaseApp::parseToken() {
    resetToken(&curToken);

    consumeWhiteSpaces();

    if ( pAvailable() <= 0 ) {
        curToken.tkType = XTB_TK_EOP;
        return &curToken;
    }

    if ( pEOL() ) {
        pRead();
        curToken.tkType = XTB_TK_EOL;
        return &curToken;
    }

    if ( pArgSeparator() ) {
        pRead();
        curToken.tkType = XTB_TK_ARGSEP;
        return &curToken;
    }

    if ( pStringStart() ) {
        char* str = consumeString();
        if ( str == NULL ) {
            curToken.tkType = XTB_TK_ERR;
            curToken.iValue = XTB_ERR_STRING; // unclosed string
            curToken.addr = xtb_prgmSourceCursor; 
        } else {
            curToken.tkType = XTB_TK_STR;
            curToken.addr = xtb_strMemCursor;
            int next = xtbPushString(str);
            if ( next < 0 ) {
               curToken.tkType = XTB_TK_ERR;
               curToken.iValue = XTB_ERR_STRING_OVF; // too much strings
               curToken.addr = xtb_prgmSourceCursor; 
            }
        }
        return &curToken;
    }

    if ( pParenthesisStart() ) {
        pRead();
        curToken.tkType = XTB_TK_PARENTOP;
        return &curToken;
    }

    if ( pParenthesisStop() ) {
        pRead();
        curToken.tkType = XTB_TK_PARENTCL;
        return &curToken;
    }

    if ( pNumOperatorStart() ) {
        curToken.tkType = XTB_TK_OPR;
        curToken.iValue = (int)pPeek(); // get the char
        pRead();
        return &curToken;
    }

    if ( pAssignStart() ) { // "=" -or- "=="
        curToken.tkType = XTB_TK_ASSIGN;
        pRead();
        if ( pAssignStart() ) {
            curToken.tkType = XTB_TK_OPR_BOOL;
            curToken.iValue = BOP_EQ;
        }
        return &curToken;
    }

    if ( pBoolOperatorStart() ) {
        char cmplxOp[2+1]; memset(cmplxOp, 0x00, 2+1);
        curToken.tkType = XTB_TK_OPR_BOOL;
        cmplxOp[0] = pPeek(); // get the char
        pRead();
        if ( pBoolOperatorStart() ) {
            cmplxOp[1] = pPeek(); // get the char
            pRead();
        }

        curToken.iValue = getBoolOpertor(cmplxOp);

        return &curToken;
    }

    if ( pNumStart() ) {
        float value = consumeNumber();
        curToken.fValue = value;
        curToken.tkType = XTB_TK_NUM;
        return &curToken;
    }

    // now can be a letter or an error
    if ( pLetter() ) {
        char* name = consumeLetters();
        int keyWord = getKeyWord(name);
        int function = keyWord > -1 ? -1 : getFunction(name);

        if ( keyWord > -1 ) {
            curToken.tkType = XTB_TK_KEYW;
            curToken.iValue = keyWord;
            return &curToken;
        } else if ( function > -1 ) {
            curToken.tkType = XTB_TK_FCT;
            curToken.iValue = function;
            return &curToken;
        } else {
            // store name then use addr to store/retrieve location
            if ( strlen(name) > 4 ) {
                curToken.tkType = XTB_TK_ERR;
                curToken.iValue = XTB_ERR_VARNAME;
                curToken.addr = xtb_prgmSourceCursor; 
            } else {
                curToken.tkType = XTB_TK_VAR;
                Variable* var = readVar( name );
                if ( var == NULL ) {
                    error("Oups too much vars", 1);
                    return NULL;
                }
                curToken.iValue = var->index;
            }

            return &curToken;
        } 

    } 
    
    {
        println(" There's an ERROR");

        curToken.tkType = XTB_TK_ERR;
        curToken.iValue = XTB_ERR_SYNTAX; // syntax error
        curToken.addr = xtb_prgmSourceCursor;
        return &curToken;
    }

    return &curToken;
}





XTBToken** XTBaseApp::analyse() {
 println("Start");

 println("PRGM:");
 println(xtb_prgmSource);
 println("================");

// -------------------------------
    // FIXME : cleanVars() / cleanLabels()
    xtb_varCursor = 0;
    xtb_lblCursor = 0;
    xtb_jumpCursor = 0;

    for(int i=0; i < XTB_TOKENIZED_PRG_LEN; i++) {
      resetToken(  xtb_tokenized_prgm[i] );
    }
// -------------------------------

 Token* tk, *lastTk;
 int tkCursor = 0;
 
 while( true ) {
     if ( tkCursor >= XTB_TOKENIZED_PRG_LEN ) {
         tk->tkType = XTB_TK_ERR;
         tk->iValue = XTB_ERR_TOOBIG;
     } else {
        tk = parseToken();
     }

     lastTk = NULL;
     if ( tkCursor > 0 ) {
        lastTk = xtb_tokenized_prgm[ tkCursor-1 ];
     }
     copyToken( xtb_tokenized_prgm[ tkCursor++ ], tk );

     if ( tk->tkType == XTB_TK_ERR ) {
         println("There was an error !");
         print("Type : ");
         print( xtb_errors_msg[tk->iValue] );
         print(" @ ");
         print(tk->addr);
         println("");
         break;
     } else if ( tk->tkType == XTB_TK_EOP ) {
         print("Reached EndOfProgram @ ");
         println(tkCursor);
         break;
     } 

     if (!true) printToken(tk);
 }
 if ( tk->tkType == XTB_TK_ERR ) {
     return NULL;
 }

 println("Stop");
 return xtb_tokenized_prgm;
}