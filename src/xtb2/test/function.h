float rndF() {
    return (float) (rand() % 1000) / (float)1000.0;
}

// in degs
float cos(int value) {
    return 0.5;
}

const addr FUNCT_DISP   = 0x0001;
const addr FUNCT_RAND   = 0x0003;
const addr FUNCT_COS    = 0x0004;
const addr FUNCT_STRCAT = 0x0010;
const addr FUNCT_STRUPPER = 0x0011;
