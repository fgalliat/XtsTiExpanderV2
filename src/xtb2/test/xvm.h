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
        } else {
            printf("Unknown disp op [%d] VARType [%d] @%d->[%d] \n", type, varType, argAddr, mem[argAddr]);
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
    printf("Unknown disp op [%d] \n", type);
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
        } else if ( mem[curAddr] == INSTR_SETDATA ) { // set a data from an Arg
            curAddr++;
            argType type = (argType)mem[curAddr++];
            addr varAddr = (mem[curAddr++] << 8) + mem[curAddr++];

            if ( type == AT_KST ) {
                float f = getFloatFromBytes(mem, curAddr);
                curAddr+=FLOAT_SIZE;

printf("vm: SETDATA float %g\n", f);

                setDataValue( varAddr, f );
            } else if (type == AT_REG) {
                Register* reg = getReg(mem[curAddr++]);
                setDataValue( varAddr, getRegValue( reg ) );
            } else if (type == AT_VAR) {
                addr data = (mem[curAddr++] << 8) + mem[curAddr++];
                setDataValue( varAddr, data );
            }
            
        }
    }
}
