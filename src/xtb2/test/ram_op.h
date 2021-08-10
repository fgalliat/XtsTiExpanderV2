bool setDataValue(addr varAddr, float value) {
  addr start = varAddr;
  printf("vm: setDataValue(@%d, %g)\n", varAddr, value);

  dataType type = (dataType)mem[varAddr];

  varAddr++; // type -- FIXME check type ?
  varAddr++; // lenM
  varAddr++; // lenL

  if ( type == T_FLOAT ) {
    copyFloatToBytes(mem, varAddr, value);
  } else if ( type == T_BYTE ) {
    int v = (int)value;
    mem[ varAddr ] = v % 256;
  }

  // dump(start, start+8);

  return true;   
}

bool setDataValue(addr varAddr, uint8_t bte) {
  printf("vm: setDataValue(@%d, %.2X)\n", varAddr, bte);
  varAddr++; // type
  varAddr++; // lenM
  varAddr++; // lenL -- FIXME check size Vs value size
  mem[varAddr] = bte;
  return true;   
}

bool setDataValue(addr varAddr, const char* value) {
  printf("vm: setDataValue(@%d, %s)\n", varAddr, value);
  varAddr++; // type
  varAddr++; // lenM
  varAddr++; // lenL -- FIXME check size Vs value size
  int len = strlen( value );
  memcpy(&mem[varAddr], &value[0], len);
  // if ()
  // mem[varAddr+len] = 0x00;
  return true;   
}

bool setDataValue(addr varAddr, addr otherData) {
  printf("vm: setDataValue(@%d, @%d)\n", varAddr, otherData);
  varAddr++; // type
  varAddr++; // lenM
  varAddr++; // lenL -- FIXME check size Vs value size
  int len = mem[otherData+1] * mem[otherData+2];
  // FIXME check type cast
  memcpy(&mem[varAddr], &mem[otherData+3], len);
  return true;   
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

void setRegValue(Register* reg, float value) {
    copyFloatToBytes(reg->data, 0, value);
}

float getRegValue(Register* reg) {
    return getFloatFromBytes(reg->data, 0);
}

// ============================================

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

