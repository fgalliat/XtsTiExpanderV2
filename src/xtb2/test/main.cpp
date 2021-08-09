// ============================================
// = Compiler emulator
#include "globals.h"

#include "xcomp.h"

#define INC_VM 1
#if INC_VM
 // = VM emulator
 #include "xvm.h"
#endif



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

    #if INC_VM
        disp(" = Code Space =");
        dump(userCodeSpaceStart, userCodeSpaceStart+64);
        run();
    #endif

    return 0;
}