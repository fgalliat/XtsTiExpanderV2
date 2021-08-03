# XtsTiExpander v2
## XTase - fgalliat @Jul 2021

 - Batt. > ESP32 > Voltage Lvl converter > Ti wires

   | ![TTGO T_DISPLAY screen dump of TiVoyage 200](./pictures/TTGO_displayTI.jpg) | ![screen dump in enclosure](./pictures/enclosure/running.jpg) |
   | ------------------------------------------------------------ | ------------------------------------------------------------ |

   (My TTGO T_Display is now able to make a Ti ScreenDump w/ ONLY Battery 3.7v as PWR !)

- **Main Features** :

 - [x] PWR on Batt
 - [x] Var Upload
 - [x] Var Download
 - [x] Screen dump
 - [X] SPIFFS
 - [x] Buzzer
 - [x] WiFi
   - [x] start, stop, conf
   - [x] telnet as Shell client (still a bug on close)
 - [x] 'tiaction' spe Var handle


- **Sub Features** :
 - [x] connect & send/receive bytes to Ti Voyage 200

 - [~] SerialTerm to V200
   - [~] Dummy mode
   - [~] CBL Keyb mode
   - [x] Raw
   
 - [ ] install minimal TI protocol
   - [x] requestScreen (try 1st w/ ISR mode -> seems to work)
   - [x] send KeyStrokes
   - [x] sendVar -> Ti
   - [~] readAsCBL <- Ti
   - [x] receiveVar <- Ti
   
 - [~] install hiLevel protocol
   - [~] actionVar receive (wip for longer than 64 data)
   - [x] play asBasica music String
   - [x] start WiFi ...

   /!\\ (recvNb == 0) -> no more : (recvNb == nbSent)

- **Shell Features** :
  - [x] shutdown esp32
  - [ ] ls (sorted + df) -> Stream
  - [ ] cat a var
  - [ ] cat a conf
  - [x] add WiFi conf
  - [x] start / stop WiFi
  - [x] screen dump
  - [x] builtin screen color scheme / swap

- **Commands set** : 

  | tiaction          | shell     | action                                      |
  | ----------------- | --------- | ------------------------------------------- |
  | wifi:start        | wifistart | start WiFi & telnet server                  |
  | wifi:stop         | wifistop  | close WiFi & telnet                         |
  |                   | wifipsk   | add WiFi config (interactive)               |
  |                   | wifidel   | delete all WiFi configs                     |
  | screen:swap       |           | swap OLED screen color scheme               |
  |                   | screen    | TiScreenDump                                |
  | get:<varName>     |           | sends the <varName> to Ti                   |
  | play:<tuneString> |           | plays the <tuneString> onto Expander BUZZER |
  |                   | quit      | exit from Shell                             |
  |                   | halt      | shutdown Expander MCU                       |
  | list:             | ls        | list files to ?screen?                      |