/**
 * XTBase (Xtase Jul2020)
 * 
 * Editor routines
 * 
 */ 

#include "../globals.h"
#include "../xtbase.h"

#define EDITOR_LINE_LEN 128

typedef struct XTBLineOfText XTBLineOfText;
struct XTBLineOfText {
        XTBLineOfText* prev;
        XTBLineOfText* next;
        int lineNum;
        char text[ EDITOR_LINE_LEN +1 ];
};

// XTBLineOfText* convertText(char* content) {
//     // FIXME 
// }