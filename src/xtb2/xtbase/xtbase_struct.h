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
    uint8_t index;
    uint16_t addr;
};


typedef struct Variable XTBVariable;
struct Variable
{
    char name[4+1];
    float fValue;
    uint8_t index;
};

typedef struct Token XTBToken;
struct Token
{
    uint8_t tkType;
    int iValue;
    float fValue;
    uint16_t addr;
    bool toBeDestroyed;
};

typedef struct Statement XTBStatement;
struct Statement
{
    uint8_t type;
    uint16_t addr; // statement adress
    int16_t dest; // destination address (for jumps) // can be -1
    Token** tokens;
    uint8_t length;
};

typedef struct Jump XTBJump;
struct Jump {
    uint8_t type;
    uint16_t from;
    uint16_t to;
};