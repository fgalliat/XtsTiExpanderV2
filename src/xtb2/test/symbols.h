// 64KB -> 16b addr
#define MEM_SIZE 65535

uint8_t mem[MEM_SIZE];

typedef uint16_t addr;

#define FLOAT_SIZE 4
#define BYTE_SIZE 1

// ============================================
// T_BYTE -> allow : byte, char & bool
// FIXME : add short
enum dataType : uint8_t { T_NONE=0x00, T_FLOAT, T_STRING, T_BYTE };

struct Data {
    dataType type;
    uint8_t lenM;
    uint8_t lenL;
    uint8_t* data;
};

// ---------

// no regType -> always float value ?
struct Register {
    uint8_t* data;
};

// ---------

enum argType : uint8_t { AT_NONE=0x00, AT_VAR, AT_REG, AT_KST };

struct Arg {
    argType type;
    uint8_t* data;
};

// ---------

enum opComp : uint8_t {
    OPCOMP_NONE = 0x00,
    OPCOMP_GT,
    OPCOMP_LT,
    OPCOMP_GTE,
    OPCOMP_LTE,
    OPCOMP_EQ,
    OPCOMP_NEQ
};

enum opCalc : uint8_t {
    OPCALC_NONE = 0x00,
    OPCALC_PLUS,
    OPCALC_MINUS,
    OPCALC_MUL,
    OPCALC_DIV
};

// ---------

enum instr : uint8_t { 
    INSTR_NOOP=0x00, 
    INSTR_CALL,      // call a function (store in RegHL if needed)
    INSTR_COMP,      // COMPute then store in RegA
    INSTR_SETREG, 
    INSTR_SETDATA,   // take RegA then Store to a Variable/Data (ex after COMPute)
    INSTR_INCDATA,   // increment -or- decrement a Data
    INSTR_TEST,      // test a num condition then store into RegA
    INSTR_JMPAT,     // JuMP when reg A is True 
};

// ---------

// forward
void setRegValue(Register* reg, float value);
uint8_t* makeMemSeg(int howMany);

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

int getRegNum(Register* r) {
    if ( r == A ) { return 1; }
    return 2;
}