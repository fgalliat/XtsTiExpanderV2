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
    mem[ curCodePosition++ ] = AT_REG;
    mem[ curCodePosition++ ] = varAddr >> 8;
    mem[ curCodePosition++ ] = varAddr % 256;
    mem[ curCodePosition++ ] = getRegNum( reg );
    return start;
}


addr addSetDataStatement(addr varAddr, Arg* arg) {
    addr start = curCodePosition;
    mem[ curCodePosition++ ] = INSTR_SETDATA;
    mem[ curCodePosition++ ] = arg->type;
    mem[ curCodePosition++ ] = varAddr >> 8;
    mem[ curCodePosition++ ] = varAddr % 256;

    if ( arg->type == AT_REG ) {
        mem[ curCodePosition++ ] = arg->data[0];
    } else if ( arg->type == AT_KST ) {

        // float v = getFloatFromBytes( arg->data, 0 );
        // if ( mem[varAddr] == F_BYTE ) {
        // }

        memcpy(&mem[curCodePosition], arg->data, FLOAT_SIZE);
        curCodePosition += FLOAT_SIZE;
    } else if ( arg->type == AT_VAR ) {
        mem[ curCodePosition++ ] = arg->data[0];
        mem[ curCodePosition++ ] = arg->data[1];
    }
    
    return start;
}