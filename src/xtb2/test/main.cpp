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

// ============================================
enum dataType : uint8_t { T_NONE=0x00, T_FLOAT, T_STRING };

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

bool addData(dataType type, int lenM=1, int lenL=-1) {
  if ( lenL < 0 ) {
      if  ( type == T_FLOAT ) {
          lenL = FLOAT_SIZE;
      } else
      return false;
  }
  
  // FIXME : handle overflow
  addr varAddr = getDataAddr( userDataCounter );

//   printf("%d\n", varAddr);

  mem[varAddr++] = type;
  mem[varAddr++] = lenM;
  mem[varAddr++] = lenL;
  // reserve ( lenM * lenL ) bytes for data

  userDataCounter++;
  return true;
}

bool setDataValue(int varNum, float value) {
  addr varAddr = getDataAddr( varNum );
  varAddr++; // type -- FIXME check type ?
  varAddr++; // lenM
  varAddr++; // lenL
  copyFloatToBytes(mem, varAddr, value);
  return true;   
}

bool setDataValue(int varNum, const char* value) {
  addr varAddr = getDataAddr( varNum );
  varAddr++; // type
  varAddr++; // lenM
  varAddr++; // lenL -- FIXME check size Vs value size
  int len = strlen( value );
  memcpy(&mem[varAddr], &value[0], len);
  return true;   
}

// ============================================

// start of User Funct // less are System Funct
addr userFuncSpaceStart = 1024; // FIXME

const addr FUNCT_DISP = 0x0001;

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

// bundle args
bool call(addr funct, int argc, Arg** args) {
    if ( funct < userFuncSpaceStart ) {
        // System Funct
        if ( funct == FUNCT_DISP ) {
            doDisp( argc, args );
        }
    } else {
        // User Funct
    }
    // freeArg(arg0);
    return true;
}

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

addr userCodeSpaceStart = 2048; // FIXME
addr curCodePosition = userCodeSpaceStart;

enum instr : uint8_t { 
    INSTR_NOOP=0x00, 
    INSTR_CALL, 
    INSTR_SETREG,
    INSTR_SETDATA };

void addCallStatement(addr functionAddr, int argc, Arg** argv, bool autoDelete) {
    mem[ curCodePosition++ ] = INSTR_CALL;
    mem[ curCodePosition++ ] = functionAddr >> 8;
    mem[ curCodePosition++ ] = functionAddr % 256;
    mem[ curCodePosition++ ] = (uint8_t)argc;
    for(int i=0; i < argc; i++) {
        writeArgToMem( curCodePosition, argv[i] );
        if (autoDelete) freeArg( argv[i] );
    }
}


// ============================================

void run() {
    addr curAddr = userCodeSpaceStart;

    if ( mem[curAddr] == INSTR_NOOP ) {
        return;
    } else if ( mem[curAddr] == INSTR_CALL ) {
        curAddr++;
        addr fct = (mem[curAddr++] << 8) + mem[curAddr++];
        int argc = mem[curAddr++];
        Arg** args = readArgsFromMem(argc, curAddr);
        call(fct, argc, args);
        free(args);
    }
}

// ============================================

void br() { printf("\n"); }

int main(int argc, char** argv) {
    addData(T_FLOAT);
    addData(T_STRING, 1, 25);

    setDataValue(0, 3.14);
    setDataValue(1, "Hello world");
 
    dump(userDataSpaceStart, 64);
    // call(FUNCT_DISP, buildArg( getDataAddr(0)));
    // call(FUNCT_DISP, buildArg( getDataAddr(1)));

    // call(FUNCT_DISP, buildArg((float)6.46));

    setRegValue(A, 65);
    // call(FUNCT_DISP, buildArg(A));

    // ======================================
    br();
    Arg* args[] = { buildArg( getDataAddr(1)), buildArg(A), buildArg( getDataAddr(0)) };
    addCallStatement( FUNCT_DISP, 3, args, true );
    dump(userCodeSpaceStart, userCodeSpaceStart+64);

    run();

    return 0;
}