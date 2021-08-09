// ==============
// = Level 1 Parser
// ==============
#include "globals.h"

#include "xcomp.h"

void br() { printf("\n"); }
void disp(const char* str) { printf("%s\n", str); }

bool startsWith(char* str1, const char* strToFind) {
    int tlen = strlen( strToFind );
    if ( strlen(str1) < tlen ) { return false; }
    return strncmp(str1, strToFind, tlen) == 0;
}

bool contains(char* str1, const char* strToFind) {
    int tlen = strlen( strToFind );
    if ( strlen(str1) < tlen ) { return false; }
    return strstr(str1, strToFind) != NULL;
}

int indexOf(char* str, char ch, int start=0) {
  if ( start < 0 ) {
      start = 0;
  }

  int len = strlen(str);
  if ( start >= len ) {
    //   start = -1;
    return -1;
  }

  for(int i=start; i < len; i++) {
      if ( str[i] == ch ) {
          return i;
      }
  }

return -1;
}

// always free-able
char* makeString(int len) {
    char* p = (char*)malloc(len+1);
    p[len] = (char)0x00;
  return p;
}

// makes a safe copy
char* substring(char* str, int start, int stop) {
    int len = strlen(str);
    if ( start >= len ) {
        return makeString(0);
    }
    if ( stop < 0 ) {
        stop = len;
    }
    if ( start < 0 ) {
        start = 0;
    }
    int rlen = stop-start;
    char* res = makeString( rlen );

    for(int i=0; i < rlen; i++) {
        res[i] = str[i+start];
    }

    return res;
}

// modify str
char* trim(char* str) {
  if ( str == NULL || strlen(str) < 1 ) { return str; }
  int tlen = strlen(str);
  int left = 0;
  int right = tlen-1;
  for(int i=0; i < tlen; i++) {
      if ( str[i] > 32 ) {
          left = i;
          break;
      }
  }
  for(int i=tlen-1; i >= 0; i--) {
      if ( str[i] > 32 ) {
          right = i;
          break;
      }
  }

  if ( left > 0 || right < tlen-1 ) {
    //   if (right < tlen-1) str[right] = 0x00; // before going to left
      if (left > 0      ) memmove(&str[0], &str[left], tlen-left);
      str[right] = 0x00;
  }

  return str;
}


// ========================
char* varsNames[99];
addr varsAddrs[99];
int varMax = 0;

void addNewVar(dataType type, char* name, int len=-1) {
    printf("Add var : (%s)\n", name);
  varsNames[varMax] = name;
  varsAddrs[varMax] = addData(type, 1, len);
  varMax++;
}

int findVar(char* name) {
    for(int i=0; i < varMax; i++) {
        int tlen = strlen(name);
        if ( strlen(varsNames[i]) == tlen && strncmp(varsNames[i], name, tlen) == 0 ) {
            return i;
        }
    }
    return -1;
}

char* lblsNames[99];
addr lblsAddrs[99];
int lblMax = 0;

void addLabel(char* name) {
    printf("Add lbl : (%s)\n", name);
  lblsNames[lblMax] = name;
  lblsAddrs[lblMax] = curCodePosition; // from xcomp.h
  lblMax++;
}

int findLabel(char* name) {
    for(int i=0; i < lblMax; i++) {
        int tlen = strlen(name);
        if ( strlen(lblsNames[i]) == tlen && strncmp(lblsNames[i], name, tlen) == 0 ) {
            return i;
        }
    }
    return -1;
}

// ========================

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

    if ( startsWith(line, "//") ) {
        return;
    } 
    // ======= Var Def ===============
    else if ( startsWith(line, "byte ") ) {
        char* descr = &line[5];
        int len = strlen(descr);
        if ( contains( descr, "=" ) ) {
            len = indexOf(descr, '=')+1;
        }

        char* varName = trim( substring( descr, 0, len ) );
        addNewVar(T_BYTE, varName);

        // FIXME .. freemem
    } else if ( startsWith(line, "float ") ) {
        char* descr = &line[6];
        int len = strlen(descr);
        if ( contains( descr, "=" ) ) {
            len = indexOf(descr, '=')+1;
        }

        char* varName = trim( substring( descr, 0, len ) );
        addNewVar(T_FLOAT, varName);

        // FIXME .. freemem
    } else if ( startsWith(line, "string[") ) {
        char* descr = &line[7];
        int dMax = indexOf(descr, ']')+1;
        char* dStr = trim( substring( descr, 0, dMax ) );
        int ln = atoi(dStr);
        printf("DLen is %d \n", ln);
        descr = &descr[dMax];

        int len = strlen(descr);
        if ( contains( descr, "=" ) ) {
            len = indexOf(descr, '=')+1;
        }

        char* varName = trim( substring( descr, 0, len ) );

        addNewVar(T_STRING, varName, ln);
        // FIXME ..
    }
    // condition
    else if ( startsWith(line, "if ") ) {
        if ( contains(line, "==") ) {
            
        } else if ( contains(line, "!=") ) {
            
        } else if ( contains(line, ">=") ) {
            
        } else if ( contains(line, "<=") ) {
            
        } else if ( contains(line, ">") ) {
            
        } else if ( contains(line, "<") ) {
            
        }
    }
    // assignation
    else if ( contains(line, "=") ) { // FIXME : contained in str ..
    }
    else if ( startsWith(line, "inc ") ) {
    }
    // calls
    else if ( startsWith(line, "disp ") ) {
    }
    // labels
    else if ( startsWith(line, "lbl ") ) {
        char* lblName = &line[4];
        addLabel(lblName);
    }

    else
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




