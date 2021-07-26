# XtsTiExpander v2
## XTase - fgalliat @Jul 2021

 - Batt. > ESP32 > VoltageLvl converter > Ti wires
 
 - [x] connect & send/receive bytes to Ti Voyage 200
 - [~] SerialTerm to V200
   - [ ] Dummy mode
   - [~] CBL Keyb mode
   - [x] Raw
 - [ ] install minimal TI protocol
   - [x] requestScreen (try 1st w/ ISR mode -> seems to work)
   - [x] send KeyStrokes
   - [ ] sendVar -> Ti
   - [~] readAsCBL <- Ti
   - [~] receiveVar <- Ti

   /!\\ (recvNb == 0) -> no more : (recvNb == nbSent)