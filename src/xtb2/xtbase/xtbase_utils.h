/**
 * XTBase (Xtase May2020)
 * 
 * Utils
 * 
 */ 

#include "../globals.h"
#include "../xtbase.h"

// === Types ===

bool xtbIsVar(XTBToken* t) {
    if ( t == NULL ) { return false; }
    return t->tkType == XTB_TK_VAR;
}

bool xtbIsString(XTBToken* t) {
    if ( t == NULL ) { return false; }
    return t->tkType == XTB_TK_STR;
}

bool xtbIsNum(XTBToken* t) {
    if ( t == NULL ) { return false; }
    return t->tkType == XTB_TK_NUM;
}

// === Values ===

XTBVariable* _xtbGetVar(int varNum) {
    if ( varNum < 0 || varNum >= XTB_NB_VAR_MAX ) { return NULL; }
    return xtb_vars[ varNum ];
}

XTBVariable* xtbGetVar(XTBToken* t) {
    if ( t == NULL ) { return NULL; }
    return _xtbGetVar( t->iValue );
}

Variable* _xtbAddVar(char* name, float value) {
    sprintf(xtb_vars[xtb_varCursor]->name, "%s", name);
    xtb_vars[xtb_varCursor]->fValue = value;
    return xtb_vars[xtb_varCursor++];
}

Variable* _xtbFindVar(char* name) {
    int tlen = strlen(name);
    for(int i=0; i < xtb_varCursor; i++) {
        if ( strncmp(xtb_vars[i]->name, name, tlen) == 0 ) {
            return xtb_vars[i];
        }
    }
    return NULL;
}

// get or create a var
Variable* readVar(char* name) {
    Variable* found = _xtbFindVar(name);
    if ( found == NULL ) {
        found = _xtbAddVar(name, 0.0);
    }
    return found;
}



// ========= Labels ===========

XTBLabel* xtbGetLabel(char* lblName) {
    int tlen = strlen(lblName);
    for(int i=0; i < xtb_lblCursor; i++) {
        if ( strncmp(xtb_lbls[i]->name, lblName, tlen) == 0 ) {
            return xtb_lbls[i];
        }
    }
    return NULL;
}


XTBStatement* xtbGetStatement(int stNum) {
    if ( stNum < 0 || stNum >= xtb_nbStatements ) { return NULL; }
    return xtb_statements[stNum];
}

// ========== Strings ========
char xtb_curString[ XTB_STRING_MAX_LEN ];
char* _xtbGetString(int num) {
    if (num < 0 || num >= XTB_NB_STRINGS_MAX) { return NULL; }
    memcpy(xtb_curString, &xtb_strMemSpace[ num * XTB_STRING_MAX_LEN ], XTB_STRING_MAX_LEN);
    return xtb_curString;
}

int _xtbSetString(int num, char* str) {
    if (num < 0 || num >= XTB_NB_STRINGS_MAX) { return -1; }
    if (str == NULL) { return -1; }
    int tlen = strlen( str );
    memset( &xtb_strMemSpace[ num * XTB_STRING_MAX_LEN ], 0x00, XTB_STRING_MAX_LEN );
    memcpy( &xtb_strMemSpace[ num * XTB_STRING_MAX_LEN ], str, min( XTB_STRING_MAX_LEN, tlen ) );
    return num;
}

char* xtbGetString(XTBToken* t) {
    if ( t == NULL ) { return NULL; }
    return _xtbGetString( t->addr ); // !!!
}

int xtbPushString(char* str) {
    if (xtb_strMemCursor >= XTB_NB_STRINGS_MAX) { return -1; }
    if (str == NULL) { return -1; }
    _xtbSetString( xtb_strMemCursor, str );
    return xtb_strMemCursor++;
}