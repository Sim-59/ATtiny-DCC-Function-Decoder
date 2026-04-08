//  ACK-Rückmeldung für CV-Programmierung (PT) hinzugefügt                   MiHo 2026-04-06
//  https://simandit.de/simwiki/doku.php?id=modellbahn:umbauten:rekowagen
//  Funktion F2 entfällt dafür
//  Die Funktionstasten F0, F1, F3 steuern die Ausgänge PB0, PB1, PB4 
//   F0 und F3 schalten richtungsabhängig (Richtung konfigurierbar mit CV29) 
//   F1 schaltet fest
//
//  CV1 default 3   DCC Adresse   
//  CV8             Schreiben von 0 führt einen Decoder Reset aus (default DCC Adresse 3)
//  CV21            deaktiviert die Funktionen F0 and F3 (Bit0 bzw. Bit3 = 1, Bit7 = 1 schaltet die Richtungserkennung ab)
//  CV29            Bit0 = 1 kann für die Richtungsumkehr genutzt werden
//  Erst-Initialisierung bei unprogrammiertem EEPROM
//  -----------------
//
//  Anpassung an Arduino Digispark ATtiny85                                  JoFri 2024-08-12
//  https://jo-fri.github.io/Eisenbahn/DCC_ATtiny85/
//  Die Funktionstasten F0, F1 - F3 und F7 - F9 steuern die Ausgänge PB0 - PB4 
//   F0 und F3 schalten richtungsabhängig 
//   F1, F2, schalten fest
//   F7, F8, F9 flackern
//
//  Board Auswahl: ATtiny85 (Micronucleus / Digispark)
//
//     Dieses Sketch von Arduino Nano an ATtiny85 angepasst
//     Pin5 deaktiv, wenn an Pin5 was angesteckt wird, erfolgt ein Reset 
//     Fahrstufen =  FS128
//
//  benötigt wird die NmraDcc-master.zip  <NmraDcc.h> 
//  Diese Bibliothek ist auf neuerem Stand unter:  https://github.com/mrrwa/NmraDcc
//  hier gibst auch ältere Versionen https://reference.arduino.cc/reference/en/libraries/nmradcc/  
//  es wird die <NmraDcc.h> in der Version 2.0.17 benötigt.
//
// Ursprüngliche Quelle:
// Production 17 Function DCC Decoder   Dec_17LED_1Ftn.ino
// Version 6.01  Geoff Bunza 2014,2015,2016,2017,2018
//
/*
   Arduino ATtiny85                                 
              +--------------------+                
              |                    |   Pin             
         |-----                [ ] |    5  PB5  F4  (Reset ?)  
         | [-]    ATtiny85     [ ] |    4  PB4  F3  
    USB  | [ ]                 [ ] |    3  PB3  DccAckPin 
         | [ ]                 [ ] |    2  PB2  DCC Signal Eingang 
         | [+]                 [ ] |    1  PB1  F1  
         |-----                [ ] |    0  PB0  F0  
              |  [ ] [ ] [ ]    [+]|                
              +--------------------+                
                +5V GND  Vin                        
*/

#include <NmraDcc.h>

const int DCCPin = 2;            // PB2
const int DccAckPin = 3 ;        // PB3        ACK Erzeugung23
const int FunctionPin0 = 0;      // PB0 F0
const int FunctionPin1 = 1;      // PB1 F1     interne LED
//const int FunctionPin2 = 3;    // PB3 F2      
const int FunctionPin3 = 4;      // PB4 F3
//const int FunctionPin4 = 5;    // PB5 F4     je nach Tiny85 Model ist dieser Ausgang ein Reset 

#define CV_DECODER_FUNCTION_MODE  21

boolean richtung = false;        // Fahrtrichtungsauswertung
uint8_t cv29_byte, cv_function_byte;

NmraDcc Dcc;
DCC_MSG Packet;

struct CVPair {
  uint16_t CV;
  uint8_t Value;
};

CVPair FactoryDefaultCVs[] = {

	// The CV Below defines the Short DCC Address
  {CV_MULTIFUNCTION_PRIMARY_ADDRESS, DEFAULT_MULTIFUNCTION_DECODER_ADDRESS},

  // These two CVs define the Long DCC Address
  {CV_MULTIFUNCTION_EXTENDED_ADDRESS_MSB, CALC_MULTIFUNCTION_EXTENDED_ADDRESS_MSB(DEFAULT_MULTIFUNCTION_DECODER_ADDRESS)},
  {CV_MULTIFUNCTION_EXTENDED_ADDRESS_LSB, CALC_MULTIFUNCTION_EXTENDED_ADDRESS_LSB(DEFAULT_MULTIFUNCTION_DECODER_ADDRESS)},
  {CV_DECODER_FUNCTION_MODE, 0},

// ONLY uncomment 1 CV_29_CONFIG line below as approprate
//  {CV_29_CONFIG,                                      0}, // Short Address 14 Speed Steps
  {CV_29_CONFIG,                       CV29_F0_LOCATION}, // Short Address 28/128 Speed Steps
//  {CV_29_CONFIG, CV29_EXT_ADDRESSING | CV29_F0_LOCATION}, // Long  Address 28/128 Speed Steps  
};

//uint8_t FactoryDefaultCVIndex = sizeof(FactoryDefaultCVs) / sizeof(CVPair);
// auf 0 setzen, damit keine Reset-Initialisierung in der Loop erfolgt
uint8_t FactoryDefaultCVIndex = 0;

void notifyCVResetFactoryDefault() {
  // Make FactoryDefaultCVIndex non-zero and equal to num CV's to be reset
  // to flag to the loop() function that a reset to Factory Defaults needs to be done
  FactoryDefaultCVIndex = sizeof(FactoryDefaultCVs) / sizeof(CVPair);
};

void setup() {

// initialize the digital pins as an outputs
  pinMode( DccAckPin, OUTPUT );    // ACK-SIgnal
  digitalWrite(DccAckPin, LOW);
  pinMode(FunctionPin0, OUTPUT);
  digitalWrite(FunctionPin0, LOW); 
  pinMode(FunctionPin1, OUTPUT);
  digitalWrite(FunctionPin1, LOW); 
  pinMode(FunctionPin3, OUTPUT);    
  digitalWrite(FunctionPin3, LOW); 

  pinMode(DCCPin, INPUT);          // DCC Eingang
 
  if (Dcc.getCV(CV_MULTIFUNCTION_PRIMARY_ADDRESS) == 255) {
    FactoryDefaultCVIndex = sizeof(FactoryDefaultCVs) / sizeof(CVPair);
  }
  cv29_byte = Dcc.getCV(CV_29_CONFIG);
  cv_function_byte = Dcc.getCV(CV_DECODER_FUNCTION_MODE);
  // Setup which External Interrupt, the Pin it's associated with that we're using and enable the Pull-Up
  Dcc.pin(0, DCCPin, 0);
  // Call the main DCC Init function to enable the DCC Receiver
  Dcc.init(MAN_ID_DIY, 10, FLAGS_MY_ADDRESS_ONLY, 0);  // mit FLAGS... werden nur eigene Adressen angenommen
}

void loop() {
  // You MUST call the NmraDcc.process() method frequently from the Arduino loop() function for correct library operation
  Dcc.process();
  
  if (FactoryDefaultCVIndex && Dcc.isSetCVReady()) {
    FactoryDefaultCVIndex--;  // Decrement first as initially it is the size of the array
    Dcc.setCV(FactoryDefaultCVs[FactoryDefaultCVIndex].CV, FactoryDefaultCVs[FactoryDefaultCVIndex].Value);
  }
}

// This function is called by the NmraDcc library when a DCC ACK needs to be sent
// Calling this function should cause an increased 60ma current drain on the power supply for 6ms to ACK a CV Read 
void notifyCVAck(void)
{
  digitalWrite( DccAckPin, HIGH );
  delay( 8 );  
  digitalWrite( DccAckPin, LOW );
}

// Richtungserkennung
void notifyDccSpeed(uint16_t Addr, DCC_ADDR_TYPE AddrType, uint8_t speed, DCC_DIRECTION dir, DCC_SPEED_STEPS speedSteps) {
    richtung = dir; 
    if ((cv29_byte & 0x01) != 0) richtung = !richtung;                            // CV29-Bit0 = 1 invertiert die Richtungserkennung
}

// Funktionstasten
void notifyDccFunc(uint16_t Addr, DCC_ADDR_TYPE AddrType, FN_GROUP FuncGrp, uint8_t FuncState) {
    switch (FuncGrp) {
      case FN_0_4:
        if ((cv_function_byte & 0x80) == 0) {                                     // CV21-Bit7 = 1 schaltet die Richtungserkennung aus
          if ((cv_function_byte & 0x01) == 0) digitalWrite(FunctionPin0, richtung && (FuncState & FN_BIT_00) >> 4);   // F0  nur in Fahrrichtung vorwärts, wenn nicht deaktiviert
        } else {
          digitalWrite(FunctionPin0, (FuncState & FN_BIT_00) >> 4);               // F0  in beiden Fahrtrichtungen
        }
        digitalWrite(FunctionPin1, (FuncState & FN_BIT_01));                      // F1
//      digitalWrite(FunctionPin2, (FuncState & FN_BIT_02) >> 1);                 // F2 entfällt wegen ACK an PB2
        if ((cv_function_byte & 0x80) == 0) {                                     // CV21-Bit7 = 1 schaltet die Richtungserkennung aus
          if ((cv_function_byte & 0x08) == 0) digitalWrite(FunctionPin3, !richtung && (FuncState & FN_BIT_03) >> 2);  // F3  nur in Fahrrichtung rückwärts, wenn nicht deaktiviert
        } else {
          digitalWrite(FunctionPin3, (FuncState & FN_BIT_03) >> 2);               // F3  in beiden Fahrtrichtungen
        }
//      digitalWrite(FunctionPin4, (FuncState & FN_BIT_04) >> 3);                 // F4  nicht verwendet wegen RESET an PB4
        break;

      case FN_5_8:
  //      digitalWrite(FunctionPin5, (FuncState & FN_BIT_05));                  // F5
  //      digitalWrite(FunctionPin6, (FuncState & FN_BIT_06) >> 1);             // F6
  //      digitalWrite(FunctionPin1, (FuncState & FN_BIT_07) >> 2);             // F7
  //      digitalWrite(FunctionPin2, (FuncState & FN_BIT_08) >> 3);             // F8
        break;

      case FN_9_12:
  //      digitalWrite(FunctionPin3, (FuncState & FN_BIT_09));                  // F9
  //      digitalWrite(FunctionPin10, (FuncState & FN_BIT_10) >> 1);            // F10
  //      digitalWrite(FunctionPin11, (FuncState & FN_BIT_11) >> 2);            // F11
  //      digitalWrite(FunctionPin12, (FuncState & FN_BIT_12) >> 3);            // F12
        break;

      case FN_13_20:
  //      digitalWrite(FunctionPin13, !((FuncState & FN_BIT_13)));              // F13     mit !( xxx ) invertiert  LOW = aktiv 
  //      digitalWrite(FunctionPin14, !((FuncState & FN_BIT_14) >> 1));         // F14     mit !( xxx ) invertiert  LOW = aktiv
  //      digitalWrite(FunctionPin15, (FuncState & FN_BIT_15) >> 2);            // F15     HIGH = aktiv
  //      digitalWrite(FunctionPin16, (FuncState & FN_BIT_16) >> 3);            // F16     HIGH = aktiv
        break;

      case FN_21_28:

        break;
    }
}
