/**
 * XTBase (Xtase May2020)
 * 
 * Enums
 * 
 */ 

#include "../globals.h"
#include "../xtbase.h"


#define XTB_JPTYPE_GOSUB 0

#define XTB_ERR_NONE       0x00
#define XTB_ERR_SYNTAX     0x01
#define XTB_ERR_STRING     0x02
#define XTB_ERR_STRING_OVF 0x03
#define XTB_ERR_TOOBIG     0x04
#define XTB_ERR_VARNAME    0x05

const char* xtb_errors_msg[] = {
    "None",
    "Syntax error",
    "Unclosed string",
    "Too much strings",
    "Prgm too big",
    "Var name error"
};

#define XTB_TK_EOP     0x00
#define XTB_TK_EOL     0x01
#define XTB_TK_STR     0x02
#define XTB_TK_ARGSEP  0x03
#define XTB_TK_NUM     0x04
#define XTB_TK_PARENTOP 0x05
#define XTB_TK_OPR     0x06
#define XTB_TK_KEYW    0x07
#define XTB_TK_FCT     0x08
#define XTB_TK_VAR     0x09
#define XTB_TK_ERR     0x0A
#define XTB_TK_PARENTCL 0x0B
#define XTB_TK_OPR_BOOL 0x0C
#define XTB_TK_ASSIGN   0x0D
#define XTB_TK_UNKNOWN 0x0E

const char* xtb_token_type[] = {
    "-EOP-",
    ":",
    "<String>",
    ",",
    "<num>",
    "(",
    "<+-*/%>",
    "<KeyW>",
    "<Fct>",
    "<Var>",
    "<Err>",
    ")",
    "<=!<>>",
    "=",
    "[??]"
};

// ============ Condition ============
#define XTB_NB_BCOND 10
const char* xtb_boolConds[XTB_NB_BCOND] = {
    "==",         // 0
    "!=",
    "<>",
    "<",
    ">",
    "<=",       // 5
    ">=",
    "&&",       // 7
    "||",
    "!"         // 9
};
int xtb_boolCond_lens[XTB_NB_BCOND];
#define BOP_EQ 0
#define BOP_NEQ 1
#define BOP_NEQ2 2
#define BOP_LT 3
#define BOP_GT 4
#define BOP_LTE 5
#define BOP_GTE 6
#define BOP_AND 7
#define BOP_OR 8
#define BOP_NOT 9

// Disp, ClrHome, Input, ...
#define XTB_STMT_REGULAR     0
// If
#define XTB_STMT_CONDITIONAL 1
// For
#define XTB_STMT_LOOP        2
// Lbl
#define XTB_STMT_LABEL       3
// Gosub (w/ return)
#define XTB_STMT_JUMP        4

const char* xtb_stmtTypes[5] = {
    "regular",
    "conditional",
    "loop",
    "label",
    "jump"
};



int getBoolOpertor(char* identifier) {
    for(int i=0; i < XTB_NB_BCOND; i++) {
        if ( strncmp(xtb_boolConds[i], identifier, xtb_boolCond_lens[i]) == 0 ) {
            return i;
        }
    }
    return -1;
}
