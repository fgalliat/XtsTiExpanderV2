# XTBase Xtase language parser

Xtase - fgalliat @May 2020



- **Link (Serial / Serial BT)**
  - Send PRGM
  - Recv PRGM
  - Recv binary (for SD-updater / M5 only)
- **functionalities**
  - Keyb Input, Screen Output (TXT & GFX) + Serial(BT) I/O
  - WiFi connect, get/post REST client
  - compiler, interpreter
    - Floats, String constants
    - (conditional) Jumps & Labels
    - Loops
    - If / Else
  - SD card I/O
  - I2C : DHT12 / CarKB
  - Gc
  - PRGM Manager(new, edit, delete, rename, send, recv)
- **functions**
  - abs(x), cos(x), sin(x), int(x), frac(x), sqrt(x)
  - **beep( [duration] )**
  - **serBauds(115200)**
  - **serPrint( "Hello", "World", 3.14 )**
  - battLvl(), battAC()
  - menu("title", "opt1", "opt2", ...), confirm("Are you Sure"), alert("Its warming !")
  - getKey(), waitKey()
  - pinMode(x,y), digRead(x), digWrite(x,y), anaRead(x), pwmWrite(x,y)
- **Editor**
  - maybe use some DoubleLinkedList for LineOfText s
  - LineOfText
    - LineOfText* prev
    - LineOfText* next
    - char text[128+1] (on a 320 pixel width screen it makes 53chars * 2lines -> 106chars)
    - int lineNum
- **Fs** : must be able to work w/ SDCard & SPIFFS
  - bool selectFs(bool sdcard) -> return false if can't mount
    - selectFs(true) -> retries even if SD failed @ boot-time
  - char* readTextFile(char* filename, int &len) -> /!\\ uses malloc (can free later)
  - int writeTextFile(char* filename, char* content, bool overwrite=false, int maxLen=-1) -> return bytes wrote / -1 if could not write file
  - int writeLineOfText(char* filename, LineOfText* firstLine, bool overwrite=false) -> return nbOfLinesWrote
  - LineOfText* readLineOfText(char* filename, int &nbOfLines) -> return NULL if could not read
  - LineOfText* textToLineOfText( char* content )
  - -or directly-
  - LineOfText* readLinesOfText(char* filename, int &nbOfLines) -> nbOfLines = 0 : empty file, <0 : error
  - int writeLinesOfText(char* filename, LineOfText* text) -> return nbOfLines wrote (-1 for error)