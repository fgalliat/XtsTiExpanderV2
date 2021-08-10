// ==============
// = Level 1 Parser
// ==============
#include "globals.h"

#include "xcomp.h"

#define INC_VM 1
#if INC_VM
 // = VM emulator
 #include "xvm.h"
#endif


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

int indexOf(char* str, char* toFind, int start=0) {
    // printf("%s -- %s\n", str, toFind);
    int idx = indexOf(str, toFind[0], start);
    if ( idx < 0 ) { return idx; }
    int tlen = strlen(toFind);
    if (tlen > 0) {
        int ii = 0;
        while( true ) {
            idx = indexOf(str, toFind[0], start);
            int id2 = idx;
            int i;
            for(i=1; tlen; i++) {
                id2 = indexOf(str, toFind[i], id2 );
                // printf("? '%c' %d -> %d (%d) [%d]\n", toFind[i], idx, id2, i, start);
                if ( id2 != idx+i ) {
                    ii=0;
                    start++;
                    break;
                } else {
                    ii=1;
                }
            }
            if ( start >= strlen( str ) ) {
                return -1;
            } else if (i == tlen) {
                return idx;
            }
        }
    }
    return idx;
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

char* str_split(char* stringToSplit, char delim, int tokenNum) {
    if ( tokenNum < 0 ) {
        return makeString(0);
    }

    int curToken = 0;
    int idx = 0, lastIdx = 0;

    while( true ) {
        lastIdx = idx;
        idx = indexOf(stringToSplit, delim, idx);
        if ( idx < 0 ) {
            // return NULL;
            // THERE WAS A BUG HERE !!!
            return substring(stringToSplit, lastIdx, strlen( stringToSplit ) );
        }
        if ( curToken == tokenNum ) {
            return substring(stringToSplit, lastIdx, idx);
        }
        idx++; // += delim.length
        curToken++;
    }

    return NULL;
}

int str_count(char* stringToSplit, char delim) {
    int curToken = 0;
    int idx = 0;//, lastIdx = 0;

    while( true ) {
        // lastIdx = idx;
        idx = indexOf(stringToSplit, delim, idx);
        if ( idx < 0 ) {
            break;
        }
        idx++; // += delim.length
        curToken++;
    }

    return curToken;
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
      int newLen = (right-left)+1;
      char tmp[newLen+1]; memset(tmp, 0x00, newLen+1);

      memcpy(tmp, &str[left], newLen);
      memset( str, 0x00, tlen );
      memcpy(&str[0], tmp, newLen);
  }

  return str;
}


// ========================
char* varsNames[99];
addr varsAddrs[99];
int varMax = 0;

void addNewVar(dataType type, char* name, int len=-1) {
    printf(" Add var : (%s)\n", name);
  // varsNames[varMax] = name;
  varsNames[varMax] = (char*)malloc( strlen(name)+1 );
  sprintf(varsNames[varMax], name, strlen(name));
  varsAddrs[varMax] = addData(type, 1, len);
  varMax++;
}

int findVar(char* name) {
    printf(" Var find [%s]\n", name);
    int tlen = strlen(name);
    for(int i=0; i < varMax; i++) {
        if ( strlen(varsNames[i]) == tlen && strncmp(varsNames[i], name, tlen) == 0 ) {
            printf(" Var found [%s]\n", name);
            return i;
        }
    }
    printf(" Var Not found !!!! [%s]\n", name);
    return -1;
}

char* lblsNames[99];
addr lblsAddrs[99];
int lblMax = 0;

void addLabel(char* name) {
    printf(" Add lbl : (%s)\n", name);
//   lblsNames[lblMax] = name;
lblsNames[lblMax] = (char*)malloc( strlen(name)+1 );
  sprintf(lblsNames[lblMax], name, strlen(name));
  lblsAddrs[lblMax] = curCodePosition; // from xcomp.h
  lblMax++;
}

int findLabel(char* name) {
    for(int i=0; i < lblMax; i++) {
        int tlen = strlen(name);
        if ( strlen(lblsNames[i]) == tlen && strncmp(lblsNames[i], name, tlen) == 0 ) {
            printf(" Label found [%s]\n", name);
            return i;
        }
    }
    printf(" Label Not found !!!! [%s]\n", name);
    return -1;
}

Arg* readArgFromString(char* str) {
    trim(str);
    if ( str[0] >= 'a' && str[0] <= 'z' ) {
        return buildArg( varsAddrs[ findVar(str) ] );
    } else if ( str[0] == '"' ) {
        int tlen = (strlen(str)-2);
        char str2[tlen+1]; memset(str2, 0x00, tlen+1);
        memcpy(str2, &str[1], tlen);
        addr data = addDataStringConstant( str2 );
        return buildArg( data );
    } else if ( strlen(str) > 2 && str[0] == '0' && str[1] == 'x' ) {
        // FIXME
        disp("FIXME 0x01");
        float f = 255.0;
        return buildArg( f );
    } else {
        float f = atof(str);
        return buildArg( f );
    }
    return NULL;
}

Arg** readArgsFromString(char* str, int &nbArgs) {
  int nbMaxArgs = str_count(str, ' ');
  //if ( nbMaxArgs == 0 ) { nbMaxArgs = 1; }
  nbMaxArgs++;

  Arg** coll = (Arg**)malloc( nbMaxArgs * sizeof( Arg* ) );

  int cpt = 0;
  for(int i=0; i < nbMaxArgs; i++) {
      char* tk = str_split(str, ' ', i);
      if ( strlen(tk) == 0 || tk[0] == ' ' ) {
          continue;
      }
      if ( tk[0] == '"' ) {
          char str2[128+1]; memset(str2, 0x00, 128+1);
          sprintf( str2, "%s", tk );
          while ( tk[strlen(tk)-1] != '"' ) {
              i++;
              tk = str_split(str, ' ', i);
              int curLen = strlen(str2);
              sprintf( &str2[curLen], " %s", tk );
          }
        //   disp(">>>>");
        //   disp(str2);
        //   disp(">>>>");
          addr ad = addDataStringConstant(str2);
          coll[cpt++] = buildArg( ad );
      } else if ( tk[0] >= 'a' && tk[0] <= 'z' ) {
          addr ad = varsAddrs[ findVar(tk) ];
          coll[cpt++] = buildArg( ad );
      } else if ( startsWith(tk, "0x") ) {
          // FIXME
          float v = 255.0;
          coll[cpt++] = buildArg( v );
      }
      else if ( tk[0] >= '0' && tk[0] <= '9' || tk[0] == '-' || tk[0] == '.' ) {
          float v = atof(tk);
// printf( "readArgsFromString() -> %g (%s)", v, tk );
          coll[cpt++] = buildArg( v );
      }
      // free(tk);
  }
  nbArgs = cpt;

  return coll;
}

// ========================

void handleLine(char* line, bool prevIsIfTrue=false) {
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

    if ( startsWith(line, "//") ) {
        return;
    } 
    // ======= Var Def ===============
    else if ( startsWith(line, "byte ") ) {
        char* descr = &line[5];
        int len = strlen(descr);
        if ( contains( descr, "=" ) ) {
            len = indexOf(descr, '=')+1;
            disp("ASSIGN NYI");
            return;
        }

        char* varName = trim( substring( descr, 0, len ) );
        addNewVar(T_BYTE, varName);

        // FIXME .. freemem
    } else if ( startsWith(line, "float ") ) {
        char* descr = &line[6];
        int len = strlen(descr);
        if ( contains( descr, "=" ) ) {
            len = indexOf(descr, '=')+1;
            disp("ASSIGN NYI");
            return;
        }

        char* varName = trim( substring( descr, 0, len ) );
        addNewVar(T_FLOAT, varName);

        // FIXME .. freemem
    } else if ( startsWith(line, "string[") ) {
        char* descr = &line[7];
        int dMax = indexOf(descr, ']')+1;
        char* dStr = trim( substring( descr, 0, dMax ) );
        int ln = atoi(dStr);
        // printf("DLen is %d \n", ln);
        descr = &descr[dMax];

        int len = strlen(descr);
        if ( contains( descr, "=" ) ) {
            len = indexOf(descr, '=')+1;
            disp("ASSIGN NYI");
            return;
        }

        char* varName = trim( substring( descr, 0, len ) );

        addNewVar(T_STRING, varName, ln);
        // FIXME ..
    }
    // condition
    else if ( startsWith(line, "if ") ) {
        int idd = indexOf(line, (char*)" then");
        if ( idd < 0 ) {
            disp("Wrong if statement");
            disp(line);
            return;
        }
        char* cond = substring(line, 3, idd);
        trim(cond);
        // FIXME : clean

        char* then = &line[ idd + 5 ];

        disp("cond:"); disp(cond);
        disp("then:"); disp(then);

        if ( contains(line, "==") ) {
            int idx = indexOf(cond, (char*)"==");
            Arg* arg0 = readArgFromString( substring( cond, 0, idx ) );
            Arg* arg1 = readArgFromString( substring( cond, idx+2, strlen(cond) ) );
            addTestDataStatement( arg0, OPCOMP_EQ, arg1 );
        } else if ( contains(line, "!=") ) {
            int idx = indexOf(cond, (char*)"!=");
            Arg* arg0 = readArgFromString( substring( cond, 0, idx ) );
            Arg* arg1 = readArgFromString( substring( cond, idx+2, strlen(cond) ) );
            addTestDataStatement( arg0, OPCOMP_NEQ, arg1 );
        } else if ( contains(line, ">=") ) {
            int idx = indexOf(cond, (char*)">=");
            Arg* arg0 = readArgFromString( substring( cond, 0, idx ) );
            Arg* arg1 = readArgFromString( substring( cond, idx+2, strlen(cond) ) );
            addTestDataStatement( arg0, OPCOMP_GTE, arg1 );
        } else if ( contains(line, "<=") ) {
            int idx = indexOf(cond, (char*)"<=");
            Arg* arg0 = readArgFromString( substring( cond, 0, idx ) );
            Arg* arg1 = readArgFromString( substring( cond, idx+2, strlen(cond) ) );
            addTestDataStatement( arg0, OPCOMP_LTE, arg1 );
        } else if ( contains(line, ">") ) {
            int idx = indexOf(cond, (char*)">");
            Arg* arg0 = readArgFromString( substring( cond, 0, idx ) );
            Arg* arg1 = readArgFromString( substring( cond, idx+2, strlen(cond) ) );
            addTestDataStatement( arg0, OPCOMP_GT, arg1 );
        } else if ( contains(line, "<") ) {
            int idx = indexOf(cond, (char*)"<");
            Arg* arg0 = readArgFromString( substring( cond, 0, idx ) );
            Arg* arg1 = readArgFromString( substring( cond, idx+2, strlen(cond) ) );
            addTestDataStatement( arg0, OPCOMP_LT, arg1 );
        } else {
            disp("Wrong if statement");
            disp(line);
            return;
        }

        handleLine(then, true);

    }
    else if ( startsWith(line, "inc ") ) {
        char* varName = &line[4];
        trim(varName);
        // FIXME check if extra arg
        addIncDataStatement( varsAddrs[findVar(varName)], 1.0 );
    }
    // calls
    else if ( startsWith(line, "disp ") ) {
        int nbArgs = 0;
        // disp("before crash");
        Arg** args = readArgsFromString(&line[5], nbArgs);
        // disp("after crash");
        printf("Found %d args\n", nbArgs);
        addCallStatement( FUNCT_DISP, nbArgs, args );
    }
    // labels
    else if ( startsWith(line, "lbl ") ) {
        char* lblName = &line[4];
        addLabel(trim(lblName));
    }
    else if ( startsWith(line, "goto ") ) {
        char* lblName = &line[5];
        trim(lblName);
        if ( !prevIsIfTrue ) {
            printf("NYI gotoAnyWay [%s]\n", lblName);
            return;
        }
        disp("before jump");
        disp(lblName);
        addJumpWhenAisTrue( lblsAddrs[findLabel(lblName)] );
        disp("before jump");
    }
    // assignation
    else if ( contains(line, "=") ) { // FIXME : contained in str ..
        if ( contains(line,"+") || contains(line,"-") || contains(line,"*")
              || contains(line,"/") || contains(line,"(") || contains(line,")") ) {
            disp("CALC NYI");
            return;
        }

        int idx = indexOf(line, '=');
        char* varName = substring(line, 0, idx);
        trim(varName);
        char* expr = substring(line, idx+1, strlen(line));
        trim(expr);

        // disp("+++");
        // disp((const char*)varName);
        // disp((const char*)expr);
        // disp("---");

        addr varAddr = varsAddrs[ findVar( varName ) ];
        int nbArgs = 0;
        Arg** args = readArgsFromString(expr, nbArgs);

// disp("-------->");
// printf( "var@ %d \n", varAddr );
// doDisp(args[0]);
// disp("-------->");
        addSetDataStatement( varAddr, args[0] );
    }

    else
    printf("(!!) > %s\n", line);
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

#if INC_VM
    disp("=====================");
    disp(" = Data Space =");
    dump(userDataSpaceStart, userDataSpaceStart+64);
    disp(" = Code Space =");
    dump(userCodeSpaceStart, userCodeSpaceStart+64);
    run();
#endif

return 0;
}




