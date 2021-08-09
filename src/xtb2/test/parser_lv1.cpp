// ==============
// = Level 1 Parser
// ==============
#include "globals.h"

#include "xcomp.h"

void br() { printf("\n"); }
void disp(const char* str) { printf("%s\n", str); }

void handleLine(char* line) {
    int tlen = strlen(line);
    if ( tlen == 0 ) { return; }

    int startOfLine = 0;
    for( ; startOfLine < tlen; startOfLine++ ) {
        if ( !( line[startOfLine] == ' ' || line[startOfLine] == '\t' ) ) {
            break;
        }
    }
    line = &line[startOfLine];
    tlen = strlen(line);
    if ( tlen == 0 ) { return; }

    printf("> %s\n", line);
}

int main(int argc, char** argv) {
    FILE* f = fopen("sample.xtb", "r");
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    printf("File is %ld bytes long\n", size);

    fseek(f, 0, SEEK_SET);

    #define MAX_LINE_LEN 80
    char line[MAX_LINE_LEN+1];
    int lineCursor = 0;

    for(long i=0; i < size; i++) {
        char c[1];
        fread(c, 1, 1, f);

        if ( c[0] == '\r' || c[0] == '\n' ) {
           line[lineCursor] = 0x00;
           handleLine(line);
           lineCursor = 0; 
        } else {
            line[lineCursor++] = c[0];
        }

    }
    if ( lineCursor > 0 ) {
        handleLine(line);
        lineCursor = 0;
    }

    fclose( f );

return 0;
}




