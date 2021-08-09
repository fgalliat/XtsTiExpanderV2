// ============================================
#include "globals.h"



// ============================================
int userDataCounter = 0;

// check retType
addr seekData(addr addr) {
    uint8_t type = mem[addr++];
    uint8_t lenM = mem[addr++];
    uint8_t lenL = mem[addr++];
    addr += lenM * lenL;
    return addr;
}

addr getDataAddr(int numVar) {
  int cpt = 0;
  addr curAddr = userDataSpaceStart;
  while( cpt < numVar ) {
      curAddr = seekData( curAddr );
      cpt++;
  }
  return curAddr;
}

addr addData(dataType type, int lenM=1, int lenL=-1) {
  if ( lenL < 0 ) {
      if  ( type == T_FLOAT ) {
          lenL = FLOAT_SIZE;
      } else if  ( type == T_BYTE ) {
          lenL = BYTE_SIZE;
      } else
      return false;
  }
  
  // FIXME : handle overflow
  addr varAddr = getDataAddr( userDataCounter );
  addr varStart = varAddr;

//   printf("%d\n", varAddr);

  mem[varAddr++] = type;
  mem[varAddr++] = lenM;
  mem[varAddr++] = lenL;
  // reserve ( lenM * lenL ) bytes for data

  userDataCounter++;
  return varStart;
}

addr addDataStringConstant(const char* str) {
    addr place = addData(T_STRING, 1, strlen(str));
    setDataValue(place, str);
    return place;
}



// ============================================

// do not use at runtime ... ?
Arg* buildArg(float kstVal) {
  Arg* arg = new Arg();
  arg->type = AT_KST; // need a SubType ?
  arg->data = makeMemSeg(FLOAT_SIZE);
  copyFloatToBytes(arg->data, 0, kstVal);
  return arg;
}

Arg* buildArg(addr varAddr) {
  Arg* arg = new Arg();
  arg->type = AT_VAR;
  arg->data = makeMemSeg(2);
  arg->data[0] = varAddr >> 8;
  arg->data[1] = varAddr % 256;
  return arg;
}

Arg* buildArg(Register* reg) {
  Arg* arg = new Arg();
  arg->type = AT_REG;
  arg->data = makeMemSeg(1);
  arg->data[0] = getRegNum(reg);
  return arg;
}

void freeArg(Arg* arg) {
    free(arg->data); // see if keep it later Cf prealloc
    free(arg);
}

void writeArgToMem( addr &curAddr, Arg* arg ) {
    mem[curAddr++] = arg->type;
    int size = 0;
    if ( arg->type == AT_KST ) { size = 4; }
    if ( arg->type == AT_VAR ) { size = 2; }
    if ( arg->type == AT_REG ) { size = 1; }
    for (int i=0; i < size; i++) {
        // curAddr++ is mandatory behavior
        mem[curAddr++] = arg->data[i];
    }
}



// ======================


void doDisp( Arg* arg ) {
    argType type = arg->type;
    if ( type == AT_VAR ) {
        addr argAddr = (arg->data[0] << 8) + arg->data[1];
        uint8_t varType = mem[argAddr];
        if ( varType == T_FLOAT ) {
            float v = getFloatFromBytes(mem, argAddr+3);
            printf("%g", v);
            return;
        } else if ( varType == T_STRING ) {
            int lenElem = mem[argAddr+2];
            char str[lenElem+1];
            memcpy(str, &mem[argAddr+3], lenElem);
            printf("%s", str);
            return;
        } else if ( varType == T_BYTE ) {
            int lenElem = mem[argAddr+2];
            uint8_t bte = mem[argAddr+3];
            printf("0x%s%.2X", (bte < 16 ? " " : ""), bte );
            return;
        }
    } else if ( type == AT_KST ) {
        float v = getFloatFromBytes( arg->data, 0 );
        printf("%g", v);
        return;
    } else if ( type == AT_REG ) {
        float v = getFloatFromBytes( getReg(arg->data[0])->data, 0 );
        printf("%g", v);
        return;
    }
    printf("Unknown disp op \n");
}

void doDisp(int argc, Arg** args) {
    for(int i=0; i < argc; i++) {
        doDisp(args[i]);
        if ( i < argc-1 ) printf(" ");
    }
    printf("\n");
}

void doStrCat(int argc, Arg** args) {
    if ( argc < 2 ) { return; }
    if ( args[0]->type != AT_VAR ) { return; }
    addr varDest = (args[0]->data[0] << 8) + args[0]->data[1];
    if ( mem[varDest] != T_STRING ) { return; }

    // FIXME : beware of overflow

    char* str = getStringDataValue( varDest );
    for(int i=1; i < argc; i++) {
        // BE Sure to start BEFORE next DataDescription
        // strlen can be bigger than destData end
        if ( args[i]->type == AT_VAR ) {
            addr srcAddr = (args[i]->data[0] << 8) + args[i]->data[1];
            if ( mem[srcAddr] != T_STRING ) {
                sprintf( str, "%s%g", str, getNumDataValue(srcAddr) );
            } else {
                sprintf( str, "%s%s", str, getStringDataValue(srcAddr) );
            }
        } else {
            sprintf( str, "%s%g", str, getNumValue(args[i]) );
        }
    }
}

void doStrUpper(int argc, Arg** args) {
    if ( argc < 1 ) { return; }
    if ( args[0]->type != AT_VAR ) { return; }
    addr varDest = (args[0]->data[0] << 8) + args[0]->data[1];
    if ( mem[varDest] != T_STRING ) { return; }

    // FIXME : beware of overflow

    char* str = getStringDataValue( varDest );
    int len = strlen(str); // beware of max Size (Cf non 0 terminated)

    for(int i=0; i < len; i++) {
        if ( str[i] >= 'a' && str[i] <= 'z' ) {
            str[i] = str[i] - 'a' + 'A';
        }
    }
}

// **************************

bool call(addr funct, int argc, Arg** args) {
    if ( funct < userFuncSpaceStart ) {
        // System Funct
        if ( funct == FUNCT_DISP ) {
            doDisp( argc, args );
        } else if ( funct == FUNCT_STRCAT ) {
            doStrCat( argc, args );
        } else if ( funct == FUNCT_STRUPPER ) {
            doStrUpper( argc, args );
        } else if ( funct == FUNCT_RAND ) {
            setRegValue(HL, rndF() );
        } else if ( funct == FUNCT_COS ) {
            if ( argc != 1 ) { return false; }
            setRegValue(HL, cos( (int)getNumValue(args[0]) ) );
        }
    } else {
        // User Funct
    }
    return true;
}

bool incData(addr dataAddr, float delta) {
  dataType dt = (dataType)mem[dataAddr];
  if ( dt == T_STRING || dt == T_NONE ) { return false; }
  if ( dt == T_FLOAT ) {
    float v = getFloatDataValue(dataAddr);
    v += delta;
    setDataValue(dataAddr, v);
  } else if ( dt == T_BYTE ) {
      uint8_t v = getByteDataValue(dataAddr);
      v += (int)delta;
      setDataValue(dataAddr, v);
  }
  return true;
}


// ------------------


bool _compare(Arg* arg1, opComp op, Arg* arg2) {
  float value1 = getNumValue( arg1 );
  float value2 = getNumValue( arg2 );

  if ( op == OPCOMP_EQ ) {
      return ( value1 == value2 );
  } else if ( op == OPCOMP_NEQ ) {
      return ( value1 != value2 );
  } else if ( op == OPCOMP_GT ) {
      return ( value1 > value2 );
  } else if ( op == OPCOMP_LT ) {
      return ( value1 < value2 );
  } else if ( op == OPCOMP_GTE ) {
      return ( value1 >= value2 );
  } else if ( op == OPCOMP_LTE ) {
      return ( value1 <= value2 );
  }
  return false;
}

bool compare(Arg* arg1, opComp op, Arg* arg2) {
    setRegValue(A, _compare( arg1, op, arg2 ) ? 1.0 : 0.0 );
    return true;
}

bool doCompute(Arg* arg1, opCalc op, Arg* arg2) {
    float result = 0.0;
    float value1 = getNumValue( arg1 );
    float value2 = getNumValue( arg2 );

    if ( op == OPCALC_PLUS ) {
        result = value1+value2;
    } else if ( op == OPCALC_MINUS ) {
        result = value1-value2;
    } else if ( op == OPCALC_MUL ) {
        result = value1*value2;
    } else if ( op == OPCALC_DIV ) {
        result = value1/value2;
    }

    setRegValue(A, result );
    return true;
}

// ============================================

addr curCodePosition = userCodeSpaceStart;

addr addCallStatement(addr functionAddr, int argc, Arg** argv, bool autoDelete=true) {
    addr start = curCodePosition;
    mem[ curCodePosition++ ] = INSTR_CALL;
    mem[ curCodePosition++ ] = functionAddr >> 8;
    mem[ curCodePosition++ ] = functionAddr % 256;
    mem[ curCodePosition++ ] = (uint8_t)argc;
    for(int i=0; i < argc; i++) {
        writeArgToMem( curCodePosition, argv[i] );
        if (autoDelete) freeArg( argv[i] );
    }
    return start;
}

addr addIncDataStatement(addr dataAddr, float delta=1.0) {
    addr start = curCodePosition;
    mem[ curCodePosition++ ] = INSTR_INCDATA;
    mem[ curCodePosition++ ] = dataAddr >> 8;
    mem[ curCodePosition++ ] = dataAddr % 256;
    copyFloatToBytes(mem, curCodePosition, delta);
    curCodePosition += FLOAT_SIZE;
    return start;
}

addr addTestDataStatement(Arg* argComp1, opComp oper, Arg* argComp2, bool autoDelete=true) {
    addr start = curCodePosition;
    mem[ curCodePosition++ ] = INSTR_TEST;
    writeArgToMem( curCodePosition, argComp1 );
    mem[ curCodePosition++ ] = oper;
    writeArgToMem( curCodePosition, argComp2 );
    if ( autoDelete ) {
        free(argComp1);
        free(argComp2);
    }
    return start;
}

addr addCalcStatement(Arg* argCalc1, opCalc oper, Arg* argCalc2, bool autoDelete=true) {
    addr start = curCodePosition;
    mem[ curCodePosition++ ] = INSTR_COMP;
    writeArgToMem( curCodePosition, argCalc1 );
    mem[ curCodePosition++ ] = oper;
    writeArgToMem( curCodePosition, argCalc2 );
    if ( autoDelete ) {
        free(argCalc1);
        free(argCalc2);
    }
    return start;
}

addr addJumpWhenAisTrue(addr codeAddr) {
    addr start = curCodePosition;
    mem[ curCodePosition++ ] = INSTR_JMPAT;
    mem[ curCodePosition++ ] = codeAddr >> 8;
    mem[ curCodePosition++ ] = codeAddr % 256;
    return start;
}

addr addSetDataStatement(addr varAddr, Register* reg) {
    addr start = curCodePosition;
    mem[ curCodePosition++ ] = INSTR_SETDATA;
    mem[ curCodePosition++ ] = varAddr >> 8;
    mem[ curCodePosition++ ] = varAddr % 256;
    mem[ curCodePosition++ ] = getRegNum( reg );
    return start;
}

// ============================================

void run() {
    addr curAddr = userCodeSpaceStart;

    while(true) {
        if ( mem[curAddr] == INSTR_NOOP ) {
            return;
        } else if ( mem[curAddr] == INSTR_CALL ) {
            curAddr++;
            addr fct = (mem[curAddr++] << 8) + mem[curAddr++];
            int argc = mem[curAddr++];
            Arg** args = readArgsFromMem(argc, curAddr);
            call(fct, argc, args);
            // --- clean ...
            for(int i=0; i < argc; i++) { free(args[i]); }
            free(args);
        } else if ( mem[curAddr] == INSTR_INCDATA ) {
            curAddr++;
            addr dataAddr = (mem[curAddr++] << 8) + mem[curAddr++];
            float delta = getFloatFromBytes(mem, curAddr);
            curAddr += FLOAT_SIZE;
            incData(dataAddr, delta);
        } else if ( mem[curAddr] == INSTR_TEST ) {
            curAddr++;
            Arg* arg1 = readArgFromMem(curAddr);
            opComp oper = (opComp)mem[curAddr++];
            Arg* arg2 = readArgFromMem(curAddr);
            compare(arg1, oper, arg2); // feeds register A
            // --- clean ...
            free(arg1);
            free(arg2);
        } else if ( mem[curAddr] == INSTR_JMPAT ) { // jump whan A true
            curAddr++;
            addr jump = (mem[curAddr++] << 8) + mem[curAddr++];
            if ( getRegValue( A ) >= 1.0 ) {
                curAddr = jump; // do jump
            }
        } else if ( mem[curAddr] == INSTR_COMP ) {
            curAddr++;
            Arg* arg1 = readArgFromMem(curAddr);
            opCalc oper = (opCalc)mem[curAddr++];
            Arg* arg2 = readArgFromMem(curAddr);
            doCompute(arg1, oper, arg2); // feeds register A
            // --- clean ...
            free(arg1);
            free(arg2);
        } else if ( mem[curAddr] == INSTR_SETDATA ) { // set a data from a reg
            curAddr++;
            addr varAddr = (mem[curAddr++] << 8) + mem[curAddr++];
            Register* reg = getReg(mem[curAddr++]);
            setDataValue( varAddr, getRegValue( reg ) );
        } 
    }
}

// ============================================

void br() { printf("\n"); }
void disp(const char* str) { printf("%s\n", str); }

int main(int argc, char** argv) {
    // C Desktop specific
    srand(time(NULL));

    addr var_i = addData(T_FLOAT);
    addr var_str = addData(T_STRING, 1, 25);
    addr var_b = addData(T_BYTE);

    addr kst_str = addDataStringConstant("ABC");

    setDataValue(var_i, (float)3.14);
    setDataValue(var_str, "Hello world");
    setDataValue(var_b, (uint8_t)0xFE);
 
    disp(" = UserData Space =");
    dump(userDataSpaceStart, 64);

    setRegValue(A, 65);

    // ======================================
    br();
    Arg* args[] = { buildArg( var_str), 
                    buildArg(A), 
                    buildArg( var_i), 
                    buildArg( var_b) };
    addCallStatement( FUNCT_DISP, 4, args, true );
    disp(" = Code Space =");
    dump(userCodeSpaceStart, userCodeSpaceStart+64);

    // 
    br();
    addIncDataStatement( var_i, 12.7 );
    addIncDataStatement( var_b, -128 );

    Arg* args2[] = { buildArg( var_i), 
                    buildArg( var_b) };
    addCallStatement( FUNCT_DISP, 2, args2, true );

    disp(" = Code Space =");
    dump(userCodeSpaceStart, userCodeSpaceStart+64);

    // ===========================
    addTestDataStatement( buildArg(var_b), OPCOMP_GTE, buildArg((float)0x80) );

    Arg* args3[] = { buildArg(var_b), buildArg( A ) };
    addr src = addCallStatement( FUNCT_DISP, 2, args3, true );

    addIncDataStatement( var_b );
    addTestDataStatement( buildArg(var_b), OPCOMP_LT, buildArg((float)0x81) );
    addJumpWhenAisTrue( src );

    Arg* args4[] = { buildArg(var_b), buildArg( A ) };
    addCallStatement( FUNCT_DISP, 2, args4, true );

    Arg* args45[] = { buildArg(var_str), buildArg( kst_str ), buildArg( (float)666 ) };
    addCallStatement( FUNCT_STRCAT, 3, args45, true );

    Arg* args47[] = { buildArg(var_str) };
    addCallStatement( FUNCT_STRUPPER, 1, args47, true );

    Arg* args5[] = { buildArg(var_str) };
    addCallStatement( FUNCT_DISP, 1, args5, true );

    // i = i + 2.0
    addCalcStatement( buildArg(var_i), OPCALC_PLUS, buildArg((float)2.0) );
    addSetDataStatement( var_i, A );

    addCallStatement( FUNCT_RAND, 0, NULL );
    Arg* args6[] = { buildArg(HL) };
    addCallStatement( FUNCT_DISP, 1, args6, true );

    Arg* args63[] = { buildArg((float)45) };
    addCallStatement( FUNCT_COS, 1, args63, true );
    Arg* args65[] = { buildArg(HL) };
    addCallStatement( FUNCT_DISP, 1, args65, true );

    Arg* args7[] = { buildArg(var_i) };
    addCallStatement( FUNCT_DISP, 1, args7, true );

    disp(" = Code Space =");
    dump(userCodeSpaceStart, userCodeSpaceStart+64);

    run();

    return 0;
}