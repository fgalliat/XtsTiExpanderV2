/**
 * XTBase (Xtase May2020)
 * 
 * functions & keywords
 * 
 */ 

#include "../globals.h"
#include "../xtbase.h"

#define XTB_NB_KEYWORD 15
const char* xtb_KeyWords[XTB_NB_KEYWORD] = {
    "disp",         // 0
    "pause",
    "clrhome",
    "input",
    "prompt",
    "end",          // 5
    "for",
    "lbl",
    "goto",
    "gosub",
    "return", // 10
    "if",
    "then",
    "else",
    "rem"
};
int xtb_KeyWords_lens[XTB_NB_KEYWORD];

#define XTB_KW_END 5
#define XTB_KW_FOR 6
#define XTB_KW_LBL 7
#define XTB_KW_GOTO 8
#define XTB_KW_GOSUB 9
#define XTB_KW_RETURN 10
#define XTB_KW_IF 11
#define XTB_KW_THEN 12
#define XTB_KW_ELSE 13
#define XTB_KW_REM 14

// (!!) argv[0] is always the Instr 
void kwDisp(XTBaseApp* _this, int argc, Token** argv) {
    // _this->println( "---------->" );
    for(int i=1; i < argc; i++) {
        if ( xtbIsString( argv[i] ) ) {
            _this->print( xtbGetString( argv[i] ) );
        } else if ( xtbIsVar( argv[i] ) ) {
            _this->print(" ");
            _this->print( xtbGetVar( argv[i] )->fValue );
        } else {
            _this->print(" ");
            _this->print( argv[i]->fValue );
        }
    }
    _this->println( "" );
    // _this->println( "---------->" );
}

void kwPause(XTBaseApp* _this, int argc, Token** argv) {
    #if INTERACTIVE_MODE
    _this->pause();
    #else
    _this->println( "---------->" );
    _this->println( "Pause" );
    _this->println( "---------->" );
    #endif
}

void kwClrHome(XTBaseApp* _this, int argc, Token** argv) {
    // _this->println( "---------->" );
    // _this->println( "ClrHome" );
    // _this->println( "---------->" );

    _this->cls();
}

void kwInput(XTBaseApp* _this, int argc, Token** argv) {
    char* messageToWrite = NULL;
    Variable* varToAssign = NULL;
    if ( argc == 2 && xtbIsVar( argv[1] ) ) {
        varToAssign = xtbGetVar( argv[1] );
    } else if ( argc == 3 && xtbIsString( argv[1] ) && xtbIsVar( argv[2] ) ) {
        varToAssign = xtbGetVar( argv[2] );
        messageToWrite = xtbGetString( argv[1] );
    } else {
        _this->error("in Input [\"msg\",]<var>", 1);
        return;
    }

    if ( varToAssign == NULL ) {
        _this->error("VarErr in Input [\"msg\",]<var>", 2);
        return;
    }

    // _this->println( "---------->" );
    if ( messageToWrite != NULL ) {
        _this->print( messageToWrite ); // no LN
    }

#if INTERACTIVE_MODE
    float inputValue = _this->inputFloat("? ");
#else
    float inputValue = 3.14;
    _this->print("? ");
    _this->println(inputValue);
#endif

    varToAssign->fValue = inputValue;
    // _this->println( "---------->" );
}

void kwPrompt(XTBaseApp* _this, int argc, Token** argv) {
    if ( argc < 2 && ! xtbIsVar(argv[1]) ) {
        _this->error("in Prompt <var>[,<var>]", 1);
        return;
    }

    // _this->println( "---------->" );
    char prompt[64+1];
    for(int i=1; i < argc; i++) {
        if ( ! xtbIsVar(argv[i]) ) {
            _this->error("in Prompt <var>,<var>", 2);
            return;  
        }
        Variable* var = xtbGetVar( argv[i] );
        if ( var == NULL ) {
            _this->error("non existsing var", 3);
            return;  
        }
        char* vn = var->name;

#if INTERACTIVE_MODE
        memset(prompt, 0x00, 64+1);
        sprintf( prompt, "%s ? ", vn );
        float inputValue = _this->inputFloat(prompt);
#else
        float inputValue = 2.76;
        _this->print(vn);
        _this->print(" ? ");
        _this->println(inputValue);
#endif
        var->fValue = inputValue;
    }
    // _this->println( "---------->" );
}

void kwEnd(XTBaseApp* _this, int argc, Token** argv) {
}

void kwFor(XTBaseApp* _this, int argc, Token** argv) {
}

void kwLbl(XTBaseApp* _this, int argc, Token** argv) {
    if ( argc < 2 || ! xtbIsString(argv[1]) ) {
        _this->error("in Lbl \"<str>\" ", 1);
        return;
    }
    _this->println( "---------->" );
    _this->println( "Lbl " );
    _this->println( "---------->" );
}

void kwGoto(XTBaseApp* _this, int argc, Token** argv) {
    if ( argc < 2 || !xtbIsString(argv[1]) ) {
        _this->error("in Goto \"<str>\" ", 1);
        return;
    }
    _this->println( "---------->" );
    char* label = xtbGetString( argv[1] );
    _this->print("Goto -> \"");
    _this->print( (label == NULL ? "NULL" : label) );
    _this->println("\"");
    _this->println( "---------->" );
}

void kwGosub(XTBaseApp* _this, int argc, Token** argv) {
    if ( argc < 2 || !xtbIsString(argv[1]) ) {
        _this->error("in Gosub \"<str>\" ", 1);
        return;
    }

    char* destLabel = xtbGetString( argv[1] );
    if (destLabel != NULL) {
        Label* dest = xtbGetLabel( destLabel );
        if ( dest == NULL ) {
            _this->error("Can't find that Lbl", 2);
            return;
        }
        _this->jumpTo(dest);
    } else {
        _this->error("Can't find that String", 3);
        return;
    }

    _this->println( "---------->" );
    _this->print("Gosub -> \"");
    _this->print( (destLabel == NULL ? "NULL" : destLabel) );
    _this->println("\"");
    _this->println( "---------->" );
}

void kwReturn(XTBaseApp* _this, int argc, Token** argv) {
    _this->println( "---------->" );
    _this->println( "Return " );
    _this->println( "---------->" );
}

void kwIf(XTBaseApp* _this, int argc, Token** argv) {
    if ( argc < 2 || ! xtbIsNum( argv[1] ) ) { // in fact its a boolean ....
        _this->error("in If <condition> ", 1); // FIXME
        return;
    }

    _this->println( "---------->" );
    _this->print("If (");
    _this->print( ( argv[0]->fValue != 0.0 ? "true" : "false" ) );
    _this->println(")");
    _this->println( "---------->" );

}

void kwThen(XTBaseApp* _this, int argc, Token** argv) {
    _this->println( "---------->" );
    _this->println( "Then " );
    _this->println( "---------->" );
}

void kwElse(XTBaseApp* _this, int argc, Token** argv) {
    _this->println( "---------->" );
    _this->println( "Else " );
    _this->println( "---------->" );
}

void kwRem(XTBaseApp* _this, int argc, Token** argv) {
    // do nothing
}

void (*kw_fun_ptr[])(XTBaseApp*, int, Token**) = {
    kwDisp,
    kwPause,
    kwClrHome,
    kwInput,
    kwPrompt,
    kwEnd,
    kwFor,
    kwLbl,
    kwGoto,
    kwGosub,
    kwReturn,
    kwIf,
    kwThen,
    kwElse,
    kwRem
};

#define XTB_NB_FUNCTION 18
const char* xtb_functions[XTB_NB_FUNCTION] = {
    "cos",
    "sin",
    "abs",
    "sqrt",

    "int",      // int(3.14) -> 3
    "frac",     // frac(3.14) -> 0.14

    "battlvl",  // BattLevel %
    "battac",   // Batt AC (0-1)

    "pinmode",  // pinMode
    "digread",  // digitalRead
    "digwrite", // digitalWrite
    "anaread",  // analogRead
    "pwmwrite",

    "menu",
    "confirm",
    "alert",

    "getkey",
    "waitkey"
};
int xtb_functions_lens[XTB_NB_FUNCTION];


float fcCos(XTBaseApp* _this, int argc, Token** argv) {
    if ( argc != 2 || ! xtbIsNum( argv[1] ) ) {
        _this->error("Cos( <num> )", 1);
        return 0.0;
    }
    return cos( argv[1]->fValue );
    // return 0.5;
}

float fcSin(XTBaseApp* _this, int argc, Token** argv) {
    if ( argc != 2 || ! xtbIsNum( argv[1] ) ) {
        _this->error("Sin( <num> )", 1);
        return 0.0;
    }
    return sin( argv[1]->fValue );
    // return 0.98;
}

float fcAbs(XTBaseApp* _this, int argc, Token** argv) {
    if ( argc != 2 || ! xtbIsNum( argv[1] ) ) {
        _this->error("Abs( <num> )", 1);
        return 0.0;
    }
    return abs( argv[1]->fValue );
}

float fcSqrt(XTBaseApp* _this, int argc, Token** argv) {
    if ( argc != 2 || ! xtbIsNum( argv[1] ) ) {
        _this->error("Sqrt( <num> )", 1);
        return 0.0;
    }
    return sqrt( argv[1]->fValue );
}

float fcInt(XTBaseApp* _this, int argc, Token** argv) {
    if ( argc != 2 || ! xtbIsNum( argv[1] ) ) {
        _this->error("Int( <num> )", 1);
        return 0.0;
    }
    return (int)( argv[1]->fValue );
}

float fcFrac(XTBaseApp* _this, int argc, Token** argv) {
    if ( argc != 2 || ! xtbIsNum( argv[1] ) ) {
        _this->error("Frac( <num> )", 1);
        return 0.0;
    }
    float v = argv[1]->fValue;
    int i = (int)v;
    return ( v - (float)i );
}

float fcBattLvl(XTBaseApp* _this, int argc, Token** argv) {
    // int  batt     = M5.Power.getBatteryLevel();
    //   bool charging = M5.Power.isCharging();
    //   bool charged  = M5.Power.isChargeFull();
    return (float)battLevel();
}

float fcBattAC(XTBaseApp* _this, int argc, Token** argv) {
    return battAC() ? 1.0 : 0.0;
}

float fcPinMode(XTBaseApp* _this, int argc, Token** argv) {
    if ( argc != 3 || !( xtbIsNum(argv[1]) && xtbIsNum(argv[2]) ) ) {
        _this->error("pinMode( <num>,<num> )", 1);
        return 0.0;
    }

    _this->print("pinMode(");
    _this->print( (int)(argv[1]->fValue) );
    _this->print(", ");
    _this->print( (int)(argv[2]->fValue) );
    _this->println(")");

    // pinMode( argv[1]->fValue, argv[2]->fValue );
    return 1.0;
}

float fcDigRead(XTBaseApp* _this, int argc, Token** argv) {
    if ( argc != 2 || ! xtbIsNum( argv[1] ) ) {
        _this->error("digRead( <num> )", 1);
        return 0.0;
    }
    // return digRead( argv[1]->fValue );
    return 1.0;
}

float fcDigWrite(XTBaseApp* _this, int argc, Token** argv) {
    if ( argc != 3 || !( xtbIsNum(argv[1]) && xtbIsNum(argv[2]) ) ) {
        _this->error("digWrite( <num>,<num> )", 1);
        return 0.0;
    }
    // digWrite( argv[1]->fValue, argv[2]->fValue );
    return 1.0;
}

float fcAnaRead(XTBaseApp* _this, int argc, Token** argv) {
    if ( argc != 2 || ! xtbIsNum( argv[1] ) ) {
        _this->error("anaRead( <num> )", 1);
        return 0.0;
    }
    // return anaRead( argv[1]->fValue );
    return 1.0;
}

float fcPwmWrite(XTBaseApp* _this, int argc, Token** argv) {
    if ( argc != 3 || !( xtbIsNum(argv[1]) && xtbIsNum(argv[2]) ) ) {
        _this->error("pwmWrite( <num>,<num> )", 1);
        return 0.0;
    }
    // digWrite( argv[1]->fValue, argv[2]->fValue );
    return 1.0;
}

float fcMenu(XTBaseApp* _this, int argc, Token** argv) {
    if ( ! argc >= 3 || !( xtbIsString(argv[1]) && xtbIsString(argv[2]) ) ) {
        _this->error("Menu( \"<title>\",\"<Item1>\"[,\"<ItemN>\"] )", 1);
        return 0.0;
    }

    int nbItems = argc-2;
    char* items[ nbItems ];
    for(int i=0; i < argc-2; i++) {
        items[i] = xtbGetString( argv[ 2+1 ] );
    }

    return (float)_this->menu( nbItems, items, xtbGetString(argv[1]) );
}

float fcConfirm(XTBaseApp* _this, int argc, Token** argv) {
    if ( ! argc == 3 || !( xtbIsString(argv[1]) && xtbIsString(argv[2]) ) ) {
        // _this->error("Confirm( [\"<title>\",]\"<Text>\" )", 1);
        if ( ! argc == 2 || !( xtbIsString(argv[1]) ) ) {
            _this->error("Confirm( [\"<title>\",]\"<Text>\" )", 2);
            return 0.0;
        }
        // return 0.0;
    }

    char* title = argc == 2 ? NULL : xtbGetString( argv[1] );
    char* text  = argc == 2 ? xtbGetString( argv[1] ) : xtbGetString( argv[2] );

    return _this->confirm( title, text ) ? 1.0 : 0.0;
}

float fcAlert(XTBaseApp* _this, int argc, Token** argv) {
    if ( ! argc == 3 || !( xtbIsString(argv[1]) && xtbIsString(argv[2]) ) ) {
        // _this->error("Alert( [\"<title>\",]\"<Text>\" )", 1);
        if ( ! argc == 2 || !( xtbIsString(argv[1]) ) ) {
            _this->error("Alert( [\"<title>\",]\"<Text>\" )", 2);
            return 0.0;
        }
        // return 0.0;
    }

    char* title = argc == 2 ? NULL : xtbGetString( argv[1] );
    char* text  = argc == 2 ? xtbGetString( argv[1] ) : xtbGetString( argv[2] );

    _this->alert( title, text );

    return 1.0;
}

// FIXME : refacto ( __waitKey() is __getKey() )
extern char __waitKey();

float fcGetKey(XTBaseApp* _this, int argc, Token** argv) {
    // FIXME : abstract XTBaseApp::getKey()
    #if ( ABSTRACT_MODE == ABSTRACT_M5 )
    return (float)__waitKey();
    #else
    return 0.0;
    #endif
}

float fcWaitKey(XTBaseApp* _this, int argc, Token** argv) {
    // FIXME : abstract XTBaseApp::getKey()
    #if ( ABSTRACT_MODE == ABSTRACT_M5 )
    while( true ) {
        char key = __waitKey();

        if ( key == 0xFE ) {
            console.println();
            _this->doBreak();
            return 0.0;
        }

        if ( key == 0x00 ) {
            delay(50);
            yield();
        }
        return (float)key;
    }
    #endif
    return 0.0;
}

float (*fct_fun_ptr[])(XTBaseApp*, int, Token**) = {
    fcCos,
    fcSin,
    fcAbs,
    fcSqrt,

    fcInt,
    fcFrac,

    fcBattLvl,
    fcBattAC,

    fcPinMode,
    fcDigRead,
    fcDigWrite,
    fcAnaRead,
    fcPwmWrite,

    fcMenu,
    fcConfirm,
    fcAlert,

    fcGetKey,
    fcWaitKey
};

int getKeyWord(char* identifier) {
    for(int i=0; i < XTB_NB_KEYWORD; i++) {
        if ( strncmp(xtb_KeyWords[i], identifier, xtb_KeyWords_lens[i]) == 0 ) {
            return i;
        }
    }
    return -1;
}

int getFunction(char* identifier) {
    for(int i=0; i < XTB_NB_FUNCTION; i++) {
        if ( strncmp(xtb_functions[i], identifier, xtb_functions_lens[i]) == 0 ) {
            return i;
        }
    }
    return -1;
}