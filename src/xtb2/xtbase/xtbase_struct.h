/**
 * XTBase (Xtase May2020)
 * 
 * structures
 * 
 */ 

#include "../globals.h"
#include "../xtbase.h"

typedef struct Label XTBLabel;
struct Label
{
    char name[8+1];
    int index;
    int addr;
};


typedef struct Variable XTBVariable;
struct Variable
{
    char name[4+1];
    float fValue;
    int index;
};

typedef struct Token XTBToken;
struct Token
{
    int tkType;
    int iValue;
    float fValue;
    int addr;
    bool toBeDestroyed;
};

typedef struct Statement XTBStatement;
struct Statement
{
    int type;
    int addr; // statement adress
    int dest; // destination address (for jumps)
    Token** tokens;
    int length;
};

typedef struct Jump XTBJump;
struct Jump {
    int type;
    int from;
    int to;
};