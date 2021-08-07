/**
 * XTBase (Xtase May2020)
 * 
 * abstraction for tty use
 * 
 */ 

#include "../globals.h"
#include "../xtbase.h"

// ==================
float cos(float rad) {
    return 0.5;
}

float sin(float rad) {
    return 0.98;
}

float sqrt(float num) {
    return 0.3333;
}
// ==================

int battLevel() {
    return 100;
}

bool battAC() {
    return true;
}

bool XTBaseApp::confirm(char* title, char* msg) {
    //return _confirm( (const char*) msg, (const char*) title, false, -1);
    println("| Confirm ?");
    print("| "); println(title);
    print("| "); println(msg);
    // FIXME
    return false;
}

void XTBaseApp::alert(char* title, char* msg) {
    //return _alert( (const char*) msg, (const char*) title, false, -1);
    println("| Alert");
    print("| "); println(title);
    print("| "); println(msg);
    // FIXME
}

// ==================


bool XTBaseApp::checkBreak() {
    // 'cause Ctrl-C kills program
  return false;   
}

void  XTBaseApp::println(const char* line) {
    print(line); print("\n");
}

void  XTBaseApp::println(int num) {
    print(num); print("\n");
}

void  XTBaseApp::println(float num) {
    print(num); print("\n");
}

void  XTBaseApp::print(const char* text) {
    printf("%s",text);
}

void  XTBaseApp::print(int num) {
    printf("%d",num);
}

void  XTBaseApp::print(float num) {
    printf("%g",num);
}

void  XTBaseApp::cls() {
    system("clear");
}

void  XTBaseApp::pause() {
    print("Pause ");
    int MAX_INPUT = 20;
    char line[MAX_INPUT+1];
    fgets(line, MAX_INPUT, stdin);
}

float XTBaseApp::inputFloat(const char* prompt) {
    float value = 0.0;
    int MAX_INPUT = 80;
    char line[MAX_INPUT+1];
    bool error = false;

    do {
        memset(line, 0x00, MAX_INPUT+1);
        print( prompt );
        fgets(line, MAX_INPUT, stdin);
        int tlen = strlen( line );
        error = false;

        if ( tlen <= 1 ) { error = true; } // just '\n' or '\r'

        for(int i=0; i < tlen; i++) {
            char ch = line[i];
            if ( ch == '\n' || ch == '\r' ) {
                break;
            }

            if ( !( ch == '-' || ch == '.' || ( ch >= '0' && ch <= '9' ) ) ) {
                error = true;
                break;
            }
        }
        if ( !error ) {
            value = atof( line );
        }
    } while( error );
    return value;
}

// return a malloc'ed buffer that can be free'd
uint8_t* XTBaseApp::loadFile(char* filename, int &len) {
    FILE *fp;
    long file_size;
    uint8_t *buffer;
    
    fp = fopen(filename, "rb");
    if (fp == NULL) {
        /* Handle error */
        // _disp("fp NULL");
        return NULL;
    }
    
    if (fseek(fp, 0 , SEEK_END) != 0) {
        /* Handle error */
        // _disp("fseek ERR");
        return NULL;
    }
    
    file_size = ftell(fp);
    if (file_size == -1) {
        /* Handle error */
        // _disp("fsize ERR");
        return NULL;
    }

    fseek(fp, 0 , SEEK_SET);    // rewind stream
    
    // printf("fsize %ld\n", file_size);

    buffer = (uint8_t*)malloc(file_size+1);
    if (buffer == NULL) {
        /* Handle error */
        // _disp("buff NULL");
        return NULL;
    }

    memset(buffer, 0x00, file_size+1);

    int read = fread(buffer, 1, file_size, fp);
    len = read;

    // printf( "(%d) %s\n", read, buffer );

    fclose(fp);
    return buffer;
}