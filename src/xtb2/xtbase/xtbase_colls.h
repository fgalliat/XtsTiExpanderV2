/**
 * XTBase (Xtase May2020)
 * 
 * Collections
 * 
 */ 

#include "../globals.h"
#include "../xtbase.h"

#define XTB_NB_LBL_MAX 26
XTBLabel* xtb_lbls[ XTB_NB_LBL_MAX ];
int xtb_lblCursor = 0;

#define XTB_NB_VAR_MAX 26
XTBVariable* xtb_vars[ XTB_NB_VAR_MAX ];
int xtb_varCursor = 0;

#define XTB_TOKENIZED_PRG_LEN 384
XTBToken* xtb_tokenized_prgm[XTB_TOKENIZED_PRG_LEN];

#define XTB_MAX_STATEMENTS 999
XTBStatement* xtb_statements[XTB_MAX_STATEMENTS];
int xtb_nbStatements = 0;

#define XTB_MAX_JUMPS 16
XTBJump* xtb_jumps[XTB_MAX_JUMPS];
int xtb_jumpCursor = 0;


#define XTB_STRING_MAX_LEN 32
#define XTB_NB_STRINGS_MAX 26
char xtb_strMemSpace[ XTB_NB_STRINGS_MAX * XTB_STRING_MAX_LEN ];
int xtb_strMemCursor = 0;

#define XTB_SOURCE_MAX_LEN 512
char xtb_prgmSource[XTB_SOURCE_MAX_LEN];
int xtb_prgmSourceCursor = 0;
int xtb_prgmSourceLen = 0;

