# XtsTiExpander v2
## XTase - fgalliat @Jul 2021

 - Batt. > ESP32 > VoltageLvl converter > Ti wires

![TTGO T_DISPLAY screen dump of TiVoyage 200](./pictures/TTGO_displayTI.jpg)
(My TTGO T_Display is now able to make a Ti ScreenDump w/ ONLY Battery 3.7v as PWR !)


 - [x] connect & send/receive bytes to Ti Voyage 200

 - [~] SerialTerm to V200
   - [~] Dummy mode
   - [~] CBL Keyb mode
   - [x] Raw
   
 - [ ] install minimal TI protocol
   - [x] requestScreen (try 1st w/ ISR mode -> seems to work)
   - [x] send KeyStrokes
   - [ ] sendVar -> Ti
   - [~] readAsCBL <- Ti
   - [x] receiveVar <- Ti
   
 - [ ] install hiLevel protocol
   - [ ] actionVar receive ..

   /!\\ (recvNb == 0) -> no more : (recvNb == nbSent)