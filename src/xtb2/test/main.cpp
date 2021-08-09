// ============================================
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "stdint.h"

static int min(int a, int b) { return a < b ? a : b; }

// ===================

static int isBEndian = -1;

static bool isBigEndian(){
    if ( isBEndian > -1 ) { return isBEndian == 1; } 
    int number = 1;
    isBEndian = (*(char*)&number != 1) ? 1 : 0;
    return isBEndian == 1;
}

static float getFloatFromBytes(unsigned char* memSeg, int address) {
    char bytes[sizeof(float)];

    if( isBigEndian() ){
       for(int i=0;i<sizeof(float);i++) {
          bytes[sizeof(float)-i] = memSeg[address+i];
        }
    }
    else{
       for(int i=0;i<sizeof(float);i++) {
          bytes[i] = memSeg[address+i];
        }
    }
    float result;
    memcpy(&result, bytes, sizeof(float));
    return result;
}

static void copyFloatToBytes(unsigned char* memSeg, int address, float f) {
    int float_tlen = sizeof(float); // 4 bytes on Arduino
    char data[float_tlen];
    memcpy(data, &f, sizeof f);    // send data
    bool bigEndian = isBigEndian();
    for(int i=0; i < float_tlen; i++) {
        if ( bigEndian ) {
            //                                       v ne manque t'il pas un -1 ???
            memSeg[ address+i ] = data[float_tlen - i];
        }
        else {
            memSeg[ address+i ] = data[i];
        }
    }
}

// ============================================

// 64KB -> 16b addr
#define MEM_SIZE 65535

uint8_t mem[MEM_SIZE];

typedef uint16_t addr;

#define FLOAT_SIZE 4
#define BYTE_SIZE 1

// ============================================

void dump(addr start=0, addr stop=MEM_SIZE) {
  int cpt = 0;
  for(addr i=start; i < stop; i++) {
      uint8_t c = mem[i];
      char d = (c >=32 && c < 128) ? (char)c : '?';
      printf("%.2X%c ", c, d);
      cpt++;
      if ( cpt == 8 ) {
          printf(" ");
      }
      if ( cpt == 16 ) {
          cpt = 0;
          printf("\n");
      }
  }
  if ( cpt != 0 ) { printf("\n"); }
}

// ============================================
// T_BYTE -> allow : byte, char & bool
enum dataType : uint8_t { T_NONE=0x00, T_FLOAT, T_STRING, T_BYTE };

struct Data {
    dataType type;
    uint8_t lenM;
    uint8_t lenL;
    uint8_t* data;
};

// ============================================

// maybe rather @ end of mem ....
addr userDataSpaceStart = 0; // FIXME

int userDataCounter = 0;

// ---- temp def
#define varI 0
#define varF 1

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

bool setDataValue(addr varAddr, float value) {
  varAddr++; // type -- FIXME check type ?
  varAddr++; // lenM
  varAddr++; // lenL
  copyFloatToBytes(mem, varAddr, value);
  return true;   
}

bool setDataValue(addr varAddr, uint8_t bte) {
  varAddr++; // type
  varAddr++; // lenM
  varAddr++; // lenL -- FIXME check size Vs value size
  mem[varAddr] = bte;
  return true;   
}

bool setDataValue(addr varAddr, const char* value) {
  varAddr++; // type
  varAddr++; // lenM
  varAddr++; // lenL -- FIXME check size Vs value size
  int len = strlen( value );
  memcpy(&mem[varAddr], &value[0], len);
  return true;   
}

addr addDataStringConstant(const char* str) {
    addr place = addData(T_STRING, 1, strlen(str));
    setDataValue(place, str);
    return place;
}

float getFloatDataValue(addr varAddr) {
  varAddr++; // type -- FIXME check type ?
  varAddr++; // lenM
  varAddr++; // lenL
  return getFloatFromBytes(mem, varAddr);
}

char* getStringDataValue(addr varAddr) {
  varAddr++; // type -- FIXME check type ?
  varAddr++; // lenM
  varAddr++; // lenL
  return (char*)&mem[varAddr];
}

uint8_t getByteDataValue(addr varAddr) {
  varAddr++; // type -- FIXME check type ?
  varAddr++; // lenM
  varAddr++; // lenL
  return mem[varAddr++];
}

float getNumDataValue(addr dataAddr) {
  dataType dt = (dataType)mem[dataAddr];
  if ( dt == T_STRING || dt == T_NONE ) { return -1.0; }
  if ( dt == T_FLOAT ) {
    float v = getFloatDataValue(dataAddr);
    return v;
  } else if ( dt == T_BYTE ) {
      uint8_t v = getByteDataValue(dataAddr);
      return (float)v;
  }
  return -1.0;
}

// ============================================

// start of User Funct // less are System Funct
addr userFuncSpaceStart = 1024; // FIXME

const addr FUNCT_DISP   = 0x0001;
const addr FUNCT_STRCAT = 0x0002;

// ============================================

// beware w/ that 
uint8_t* makeMemSeg(int howMany) {
    return (uint8_t*) malloc( howMany );
}

// ============================================

// no regType -> always float value ?
struct Register {
    uint8_t* data;
};

void setRegValue(Register* reg, float value) {
    copyFloatToBytes(reg->data, 0, value);
}

float getRegValue(Register* reg) {
    return getFloatFromBytes(reg->data, 0);
}

Register* buildReg() {
    Register* reg = new Register();
    // later use specific memZone
    reg->data = makeMemSeg(FLOAT_SIZE);
    setRegValue(reg, 0.0);
    return reg;
}

Register* A = buildReg();
Register* HL = buildReg();

Register* getReg(int num) {
    if (num == 1) return A;
    if (num == 2) return HL;
    return NULL;
}

// ============================================

enum argType : uint8_t { AT_NONE=0x00, AT_VAR, AT_REG, AT_KST };

struct Arg {
    argType type;
    uint8_t* data;
};

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
  arg->data[0] = (reg == A ? 1:2); // FIXME Reg index
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

Arg* readArgFromMem(addr &curAddr) {
    Arg* arg = new Arg(); // FIXME ?
    arg->type = (argType)mem[curAddr++];
    int size = 0;
    if ( arg->type == AT_KST ) { size = 4; }
    if ( arg->type == AT_VAR ) { size = 2; }
    if ( arg->type == AT_REG ) { size = 1; }
    arg->data = &mem[curAddr]; // FIXME : sure w/ that ?
    curAddr+= size;
    return arg;
}

Arg** readArgsFromMem(int argc, addr &curAddr) {
    if ( argc == 0 ) { return NULL; }
    Arg** args = (Arg**)malloc(argc*sizeof( Arg* ));
    for(int i=0; i < argc; i++) {
        args[i] = readArgFromMem(curAddr);
    }
    return args;
}

float getNumValue(Arg* arg) {
    if ( arg->type == AT_KST ) { return getFloatFromBytes( arg->data, 0 ); }
    if ( arg->type == AT_VAR ) { return getNumDataValue( (arg->data[0]<<8) + arg->data[1] ); }
    if ( arg->type == AT_REG ) { return getRegValue( getReg( arg->data[0] ) ); } 
    return -1.0;
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

// **************************

bool call(addr funct, int argc, Arg** args) {
    if ( funct < userFuncSpaceStart ) {
        // System Funct
        if ( funct == FUNCT_DISP ) {
            doDisp( argc, args );
        } else if ( funct == FUNCT_STRCAT ) {
            doStrCat( argc, args );
        }
    } else {
        // User Funct
    }
    // freeArg(arg0);
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

enum opComp : uint8_t {
    OPCOMP_NONE = 0x00,
    OPCOMP_GT,
    OPCOMP_LT,
    OPCOMP_GTE,
    OPCOMP_LTE,
    OPCOMP_EQ,
    OPCOMP_NEQ
};

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

// ============================================

addr userCodeSpaceStart = 2048; // FIXME
addr curCodePosition = userCodeSpaceStart;

enum instr : uint8_t { 
    INSTR_NOOP=0x00, 
    INSTR_CALL, 
    INSTR_SETREG,
    INSTR_SETDATA,
    INSTR_INCDATA,
    INSTR_TEST,
    INSTR_JMPAT, // JuMP when reg A is True 
};

addr addCallStatement(addr functionAddr, int argc, Arg** argv, bool autoDelete) {
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

addr addJumpWhenAisTrue(addr codeAddr) {
    addr start = curCodePosition;
    mem[ curCodePosition++ ] = INSTR_JMPAT;
    mem[ curCodePosition++ ] = codeAddr >> 8;
    mem[ curCodePosition++ ] = codeAddr % 256;
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
        }
    }
}

// ============================================

void br() { printf("\n"); }
void disp(const char* str) { printf("%s\n", str); }

int main(int argc, char** argv) {
    addr var_i = addData(T_FLOAT);
    addr var_str = addData(T_STRING, 1, 25);
    addr var_b = addData(T_BYTE);

    addr kst_str = addDataStringConstant("ABC");

    setDataValue(var_i, (float)3.14);
    setDataValue(var_str, "Hello world");
    setDataValue(var_b, (uint8_t)0xFE);
 
    disp(" = UserData Space =");
    dump(userDataSpaceStart, 64);
    // call(FUNCT_DISP, buildArg( getDataAddr(0)));
    // call(FUNCT_DISP, buildArg( getDataAddr(1)));

    // call(FUNCT_DISP, buildArg((float)6.46));

    setRegValue(A, 65);
    // call(FUNCT_DISP, buildArg(A));

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

    Arg* args45[] = { buildArg(var_str), buildArg( kst_str ) };
    addCallStatement( FUNCT_STRCAT, 2, args45, true );

    Arg* args5[] = { buildArg(var_str) };
    addCallStatement( FUNCT_DISP, 1, args5, true );

    disp(" = Code Space =");
    dump(userCodeSpaceStart, userCodeSpaceStart+64);

    run();

    return 0;
}