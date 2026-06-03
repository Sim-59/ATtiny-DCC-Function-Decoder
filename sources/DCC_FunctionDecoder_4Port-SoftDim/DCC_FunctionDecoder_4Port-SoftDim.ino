//  Ergänzungen - basierend auf unten veröffentlichtem Sketch                           MiHo 2026-06-03
//  https://simandit.de/simwiki/doku.php?id=modellbahn:umbauten:dcc-dekoder
//  ACK-Rückmeldung für CV-Lesen am Programmiergleis (PT) mit Zusatz-Schaltung an PB3 (AUX2) möglich
//	zuschaltbare zusätzliche Last von 60mA (z.B. Lastwiderstand) notwendig
//  Die Funktionstasten steuern die Ausgänge LV, LR, AUX1, AUX2
//  F0 schaltet richtungsabhängig LV und LR z.B. Schlusslichter (Richtung konfigurierbar mit CV29) 
//  Funktionsmapping von LR auf F1 ... F4
//  Funktionsmapping vom AUX1 und AUX2 auf F0 ... F12
//  Ausgänge LV, LR, AUX1 AUX2 können gedimmt werden
//
//  CV1 = Dekoder-Adresse, default 3
//  CV8 -> Schreiben erzeugt Dekoder-Reset
//  CV17/CV18 lange Dekoderadresse, kann mit CV29-Bit5 = 1 (64) aktiviert werden
//  CV29 = Configurations-Byte, default 2, 
//    Bit0 = 1 (1) dreht die Fahrtrichtung um
//    Bit5 = 1 (64) aktiviert erweiterte Adressierung mit CV17/CV18
//  CV33-CV46 Funktionsmapping für LR, AUX1, AUX2
//      CV33 default 1, F0-Vorwärts steuert LV bzw. LF, z.B. Schlusslicht Vorwärtsfahrt
//      CV34 default 2, F0-Rückwärts steuert LR, z.B. Schlusslicht Rückwärtsfahrt
//      CV35 default 4, F1 steuert AUX1, z.B. Innenbeleuchtung
//      CV36 default 8, F2 steuert AUX2
//  CV49, CV50 für Steuerung von LV, LR
//      Bit7=1 immer an
//      Bit6=1 aus, wenn vorwärts
//      Bit5=1 aus, wenn rückwärts
//  CV51, CV52, CV53, CV 54 Dimmwert für LV, LR, AUX1, AUX2 default 15, PB3 (AUX2) kann kein PWM -> Soft-Dim in Loop
//      Bit0 ... Bit4  (0 ... 31) 1=dunkel bis 31=volle Helligkeit, 0 wird auf 1 gesetzt
//
//  Erst-Initialisierung bei unprogrammiertem EEPROM
//  --------------------------------------------------------------------------------------------
//
//  Ausgangs-Idee - Arduino Digispark ATtiny85                                  JoFri 2024-08-12
//  https://jo-fri.github.io/Eisenbahn/DCC_ATtiny85/
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
         |-----                [ ] |    5  PB5  Reset - notwendig für ISP-Programmierung 
         | [-]    ATtiny85     [ ] |    4  PB4
    USB  | [ ]                 [ ] |    3  PB3
         | [ ]                 [ ] |    2  PB2 
         | [+]                 [ ] |    1  PB1
         |-----                [ ] |    0  PB0
              |  [ ] [ ] [ ]    [+]|                
              +--------------------+                
                +5V GND  Vin                        
*/

#include <NmraDcc.h>

//define the destination board
//#define UNO
//#define PCB_10
#define PCB_11

#ifdef UNO
  #define DCC_PIN         2     // DCC-Signal
  #define DCC_ACK_PIN     12    // ACK for CV reading
  #define LV_PIN          6     // PWM possible
  #define LR_PIN          5     // PWM possible
  #define LCAB_PIN        3     // PWM possible
  #define AUX2_PIN        4     // no PWM, Soft-Dim
  #define DEBUG

#elif defined PCB_10            // for ATtiny85 with obsolate PCB version MuFu4P 1.0
  #define DCC_PIN         2     // DCC signal
  #define DCC_ACK_PIN     3     // 60 mA-circuit can be temporarily connected to pin 4 for CV reading
  #define LV_PIN          0     // ws - LV
  #define LR_PIN          1     // ge - LR
  #define AUX2_PIN        3     // gn - AUX2 (optional ACK)
  #define LCAB_PIN        4     // vi - AUX1

#elif defined PCB_11            // for ATtiny85 PCB MuFu4P 1.1 and  MuFuDec 1.0
  #define DCC_PIN         2     // DCC signal
  #define DCC_ACK_PIN     3     // 60 mA-circuit can be temporarily connected to pin 4 for CV reading
  #define LV_PIN          0     // ws - LV
  #define LR_PIN          4     // ge - LR
  #define LCAB_PIN        1     // gn - AUX1
  #define AUX2_PIN        3     // vi - AUX2 (optional ACK)
#endif

// NMRA s-9.2.2_2012_10
// FL (f) in CV #33, FL (r) in CV#34, F1 in CV #35 ...
// CVs 33-37 control outputs 1-8
// CVs 38-42 control outputs 4-11
// CVs 43-46 control outputs 7-14
//
// Beispiel Kuehn-125 (6 outputs)
// CVs 33-37, Bit0 (1), Bit1 (2), Bit2 (4)      steuern Ausgänge 1-3 mit F0-F4
// CVs 38-42, Bit5 (32), Bit6 (64), Bit7 (128)  steuern Ausgänge 1-3 mit F5-F8
// CVs 43-46, Bit2 (4), Bit3 (8), Bit4 (16)     steuern Ausgänge 1-3 mit F9-F12
//
// CVs 49-52 für Lichteffekte Ausgänge 1-4: 
//    Bit7 (128) Funktion schaltet immer
//    Bit6 (64) aus, wenn vorwärts
//    Bit5 (32) aus, wenn rückwärts

#define CV_LV_MODE   49   // Effekte LV (Licht Vorwärtsfahrt)
#define CV_LR_MODE   50   // Effekte LR (Licht Rückwärtsfahrt)
#define CV_LV_DIM    51   // Dimmwert LV (Licht Vorwärtsfahrt)
#define CV_LR_DIM    52   // Dimmwert LR (Licht Rückwärtsfahrt)
#define CV_AUX1_DIM  53   // Dimmwert AUX1 (Innenbeleuchtung)
#define CV_AUX2_DIM  54   // Dimmwert AUX2 - Soft-Dim

#define CV_F0_F_MAP  33
#define CV_F0_R_MAP  34
#define CV_F1_MAP    35
#define CV_F2_MAP    36
#define CV_F3_MAP    37
#define CV_F4_MAP    38
#define CV_F5_MAP    39
#define CV_F6_MAP    40
#define CV_F7_MAP    41
#define CV_F8_MAP    42
#define CV_F9_MAP    43
#define CV_F10_MAP   44
#define CV_F11_MAP   45
#define CV_F12_MAP   46

#define F0_F_MAP_DEFAULT 1
#define F0_R_MAP_DEFAULT 2
#define F1_MAP_DEFAULT   4
#define F2_MAP_DEFAULT   8

boolean richtung = false;        // Fahrtrichtungsauswertung
uint8_t cv29_byte, cv_lv_cfg, cv_lr_cfg;
uint8_t cv33_f0f_map, cv34_f0r_map, cv35_f1map, cv36_f2map, cv37_f3map, cv38_f4map, cv39_f5map, cv40_f6map;
uint8_t cv41_f7map, cv42_f8map, cv43_f9map, cv44_f10map, cv45_f11map, cv46_f12map;
uint8_t cv_lv_dim, cv_lr_dim, cv_aux1_dim, cv_aux2_dim;
uint8_t aux2_dim_pos = 0; 
uint8_t aux2_enable = 0; 

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
  {CV_F0_F_MAP, F0_F_MAP_DEFAULT},
  {CV_F0_R_MAP, F0_R_MAP_DEFAULT},
  {CV_F1_MAP, F1_MAP_DEFAULT},
  {CV_F2_MAP, F2_MAP_DEFAULT},
  {CV_F3_MAP, 0},
  {CV_F4_MAP, 0},
  {CV_F5_MAP, 0},
  {CV_F6_MAP, 0},
  {CV_F7_MAP, 0},
  {CV_F8_MAP, 0},
  {CV_F9_MAP, 0},
  {CV_F10_MAP, 0},
  {CV_F11_MAP, 0},
  {CV_F12_MAP, 0},
  {CV_LV_MODE, 0},
  {CV_LR_MODE, 0},
  {CV_LV_DIM, 15},
  {CV_LR_DIM, 15},
  {CV_AUX1_DIM, 15},
  {CV_AUX2_DIM, 15},

// ONLY uncomment 1 CV_29_CONFIG line below as approprate
//  {CV_29_CONFIG,                                      0},   // Short Address 14 Speed Steps
  {CV_29_CONFIG,                       CV29_F0_LOCATION},     // Short Address 28/128 Speed Steps
//  {CV_29_CONFIG, CV29_EXT_ADDRESSING | CV29_F0_LOCATION},   // Long  Address 28/128 Speed Steps  
};

// uint8_t FactoryDefaultCVIndex = sizeof(FactoryDefaultCVs) / sizeof(CVPair);
// auf 0 setzen, damit keine Reset-Initialisierung in der Loop erfolgt
uint8_t FactoryDefaultCVIndex = 0;

void notifyCVResetFactoryDefault() {
  // Make FactoryDefaultCVIndex non-zero and equal to num CV's to be reset
  // to flag to the loop() function that a reset to Factory Defaults needs to be done
  FactoryDefaultCVIndex = sizeof(FactoryDefaultCVs) / sizeof(CVPair);
};

void setup() {

  #if defined DEBUG  
    Serial.begin(115200);
    Serial.println("DCC Function Decoder");
    Serial.println();
  #endif

// initialize the digital pins as an outputs
  pinMode(DCC_ACK_PIN, OUTPUT );    // ACK-SIgnal
  digitalWrite(DCC_ACK_PIN, LOW);
  pinMode(LV_PIN, OUTPUT);
  digitalWrite(LV_PIN, LOW); 
  pinMode(LCAB_PIN, OUTPUT);
  digitalWrite(LCAB_PIN, LOW); 
  pinMode(LR_PIN, OUTPUT);    
  digitalWrite(LR_PIN, LOW); 

  pinMode(DCC_PIN, INPUT);          // DCC Eingang
 
  if (Dcc.getCV(CV_MULTIFUNCTION_PRIMARY_ADDRESS) == 255) {
    FactoryDefaultCVIndex = sizeof(FactoryDefaultCVs) / sizeof(CVPair);
  }
  cv29_byte = Dcc.getCV(CV_29_CONFIG);

  cv_lv_cfg = Dcc.getCV(CV_LV_MODE);
  cv_lr_cfg = Dcc.getCV(CV_LR_MODE);

  cv33_f0f_map = Dcc.getCV(CV_F0_F_MAP);
  cv34_f0r_map = Dcc.getCV(CV_F0_R_MAP);
  cv35_f1map = Dcc.getCV(CV_F1_MAP);
  cv36_f2map = Dcc.getCV(CV_F2_MAP);
  cv37_f3map = Dcc.getCV(CV_F3_MAP);
  cv38_f4map = Dcc.getCV(CV_F4_MAP);
  cv39_f5map = Dcc.getCV(CV_F5_MAP);
  cv40_f6map = Dcc.getCV(CV_F6_MAP);
  cv41_f7map = Dcc.getCV(CV_F7_MAP);
  cv42_f8map = Dcc.getCV(CV_F8_MAP);
  cv43_f9map = Dcc.getCV(CV_F9_MAP);
  cv44_f10map = Dcc.getCV(CV_F10_MAP);
  cv45_f11map = Dcc.getCV(CV_F11_MAP);
  cv46_f12map = Dcc.getCV(CV_F12_MAP);

  cv_lv_dim = Dcc.getCV(CV_LV_DIM);
  if (cv_lv_dim < 1) cv_lv_dim = 1;
  if (cv_lv_dim > 31) cv_lv_dim = 31;
  cv_lv_dim = cv_lv_dim << 3;

  cv_lr_dim = Dcc.getCV(CV_LR_DIM);
  if (cv_lr_dim < 1) cv_lr_dim = 1;
  if (cv_lr_dim > 31) cv_lr_dim = 31;
  cv_lr_dim = cv_lr_dim << 3;

  cv_aux1_dim = Dcc.getCV(CV_AUX1_DIM);
  if (cv_aux1_dim < 1) cv_aux1_dim = 1;
  if (cv_aux1_dim > 31) cv_aux1_dim = 31;
  cv_aux1_dim = cv_aux1_dim << 3;
 
  // Soft-Dim
  cv_aux2_dim = Dcc.getCV(CV_AUX2_DIM);
  if (cv_aux2_dim < 1) cv_aux2_dim = 1;
  if (cv_aux2_dim > 31) cv_aux2_dim = 31;

  // Setup which External Interrupt, the Pin it's associated with that we're using and enable the Pull-Up
  Dcc.pin(0, DCC_PIN, 0);
  // Call the main DCC Init function to enable the DCC Receiver
  Dcc.init(MAN_ID_DIY, 30, FLAGS_MY_ADDRESS_ONLY, 0);  // mit FLAGS... werden nur eigene Adressen angenommen
  
//  delay(200);  // uncomment this if Micronucleus Bootloader is used.

  #if defined DEBUG  
    Serial.println("Decoder initialized");
  #endif
}

void loop() {
  // You MUST call the NmraDcc.process() method frequently from the Arduino loop() function for correct library operation
  Dcc.process();
  
  if (FactoryDefaultCVIndex && Dcc.isSetCVReady()) {
    FactoryDefaultCVIndex--;  // Decrement first as initially it is the size of the array
    Dcc.setCV(FactoryDefaultCVs[FactoryDefaultCVIndex].CV, FactoryDefaultCVs[FactoryDefaultCVIndex].Value);
    #if defined DEBUG  
      Serial.println("RESET");
    #endif
  }

// Soft-Dim AUX2
  aux2_dim_pos+=1;
  if (aux2_dim_pos > 31) aux2_dim_pos = 1;
  if (aux2_enable && (aux2_dim_pos <= cv_aux2_dim)) digitalWrite(AUX2_PIN, 1); else digitalWrite(AUX2_PIN, 0);
}

// This function is called by the NmraDcc library when a DCC ACK needs to be sent
// Calling this function should cause an increased 60ma current drain on the power supply for 6ms to ACK a CV Read 
void notifyCVAck(void)
{
  digitalWrite( DCC_ACK_PIN, HIGH );
  delay( 8 );  
  digitalWrite( DCC_ACK_PIN, LOW );
  #if defined DEBUG  
    Serial.println("ACK");
  #endif
}

// Richtungserkennung
void notifyDccSpeed(uint16_t Addr, DCC_ADDR_TYPE AddrType, uint8_t speed, DCC_DIRECTION dir, DCC_SPEED_STEPS speedSteps) {
    richtung = dir; 
    if ((cv29_byte & 0x01) != 0) richtung = !richtung;                              // CV29-Bit0 = 1 invertiert die Richtungserkennung
}

// Funktionstasten
void notifyDccFunc(uint16_t Addr, DCC_ADDR_TYPE AddrType, FN_GROUP FuncGrp, uint8_t FuncState) {
    switch (FuncGrp) {
      case FN_0_4:
        if ((cv_lv_cfg & 0x80) == 0) {                                                                               // CV49-Bit7 = 1 schaltet für PB0 die Richtungserkennung aus
          if ((((cv29_byte & 0x01) == 0) && ((cv_lv_cfg & 0x40) == 0)) || (((cv29_byte & 0x01) == 1) && ((cv_lv_cfg & 0x20) == 0))) {   // Bei Richtungs-Invertierung auch die Richtungs-Bits in CV49 wechseln
           analogWrite(LV_PIN, (richtung && ((FuncState & FN_BIT_00) >> 4)) * cv_lv_dim);                             // LV bei F0 in Fahrrichtung vorwärts, wenn nicht deaktiviert in CV49
          }
        } else {
          analogWrite(LV_PIN, ((FuncState & FN_BIT_00) >> 4) * cv_lv_dim);                                           // LV bei F0 in beiden Fahrtrichtungen
        }
        if ((cv_lr_cfg & 0x80) == 0) {                                                                // CV50-Bit7 = 1 schaltet für LR die Richtungserkennung aus
          if ((((cv29_byte & 0x01) == 0) && ((cv_lr_cfg & 0x20) == 0)) || (((cv29_byte & 0x01) == 1) && ((cv_lr_cfg & 0x40) == 0))) {   // Bei Richtungs-Invertierung auch die Richtungs-Bits in CV50 wechseln
            if (cv33_f0f_map & 0x02) analogWrite(LR_PIN, (!richtung && ((FuncState & FN_BIT_00) >> 4)) * cv_lr_dim);   // LR bei F0-F in Fahrrichtung rückwärts, wenn nicht deaktiviert in CV50
            if (cv34_f0r_map & 0x02) analogWrite(LR_PIN, (!richtung && ((FuncState & FN_BIT_00) >> 4)) * cv_lr_dim);   // LR bei F0-R in Fahrrichtung rückwärts, wenn nicht deaktiviert in CV50
            if (cv35_f1map & 0x02) analogWrite(LR_PIN, (!richtung && (FuncState & FN_BIT_01)) * cv_lr_dim);            // LR bei F1 in Fahrrichtung rückwärts, wenn nicht deaktiviert in CV50
            if (cv36_f2map & 0x02) analogWrite(LR_PIN, (!richtung && ((FuncState & FN_BIT_02) >> 1)) * cv_lr_dim);     // LR bei F2 in Fahrrichtung rückwärts, wenn nicht deaktiviert in CV50
            if (cv37_f3map & 0x02) analogWrite(LR_PIN, (!richtung && ((FuncState & FN_BIT_03) >> 2)) * cv_lr_dim);     // LR bei F3 in Fahrrichtung rückwärts, wenn nicht deaktiviert in CV50
            if (cv38_f4map & 0x40) analogWrite(LR_PIN, (!richtung && ((FuncState & FN_BIT_04) >> 3)) * cv_lr_dim);     // LR bei F4 in Fahrrichtung rückwärts, wenn nicht deaktiviert in CV50
          }
        } else {
          if (cv33_f0f_map & 0x02) analogWrite(LR_PIN, ((FuncState & FN_BIT_00) >> 4) * cv_lr_dim);   // LR bei F0-F in beiden Fahrtrichtungen
          if (cv34_f0r_map & 0x02) analogWrite(LR_PIN, ((FuncState & FN_BIT_00) >> 4) * cv_lr_dim);   // LR bei F0-R in beiden Fahrtrichtungen
          if (cv35_f1map & 0x02) analogWrite(LR_PIN, ((FuncState & FN_BIT_01)) * cv_lr_dim);          // LR bei F1 in beiden Fahrtrichtungen
          if (cv36_f2map & 0x02) analogWrite(LR_PIN, ((FuncState & FN_BIT_02) >> 1) * cv_lr_dim);     // LR bei F2 in beiden Fahrtrichtungen
          if (cv37_f3map & 0x02) analogWrite(LR_PIN, ((FuncState & FN_BIT_03) >> 2) * cv_lr_dim);     // LR bei F3 in beiden Fahrtrichtungen
          if (cv38_f4map & 0x40) analogWrite(LR_PIN, ((FuncState & FN_BIT_04) >> 3) * cv_lr_dim);     // LR bei F4 in beiden Fahrtrichtungen
        }

        if (cv33_f0f_map & 0x04) analogWrite(LCAB_PIN, ((FuncState & FN_BIT_00) >> 4) * cv_aux1_dim);  // AUX1 bei F0-F
        if (cv34_f0r_map & 0x04) analogWrite(LCAB_PIN, ((FuncState & FN_BIT_00) >> 4) * cv_aux1_dim);  // AUX1 bei F0-R
        if (cv35_f1map & 0x04) analogWrite(LCAB_PIN, (FuncState & FN_BIT_01) * cv_aux1_dim);           // AUX1 bei F1
        if (cv36_f2map & 0x04) analogWrite(LCAB_PIN, ((FuncState & FN_BIT_02) >> 1) * cv_aux1_dim);    // AUX1 bei F2
        if (cv37_f3map & 0x04) analogWrite(LCAB_PIN, ((FuncState & FN_BIT_03) >> 2) * cv_aux1_dim);    // AUX1 bei F3
        if (cv38_f4map & 0x80) analogWrite(LCAB_PIN, ((FuncState & FN_BIT_04) >> 3) * cv_aux1_dim);    // AUX1 bei F4

        // PB3 kann kein PWM -> Soft-Dim in loop
        if (cv33_f0f_map & 0x08) aux2_enable = (FuncState & FN_BIT_00) >> 4;        // AUX2 bei F0-F
        if (cv34_f0r_map & 0x08) aux2_enable = (FuncState & FN_BIT_00) >> 4;        // AUX2 bei F0-R
        if (cv35_f1map & 0x08) aux2_enable = (FuncState & FN_BIT_01);               // AUX2 bei F1
        if (cv36_f2map & 0x08) aux2_enable = (FuncState & FN_BIT_02) >> 1;          // AUX2 bei F2
        if (cv37_f3map & 0x08) aux2_enable = (FuncState & FN_BIT_03) >> 2;          // AUX2 bei F3
        if (cv38_f4map & 0x01) aux2_enable = (FuncState & FN_BIT_04) >> 3;          // AUX2 bei F4
        break;

      case FN_5_8:
        if (cv39_f5map & 0x80) analogWrite(LCAB_PIN, ((FuncState & FN_BIT_05) * cv_aux1_dim));         // AUX1 bei F5
        if (cv40_f6map & 0x80) analogWrite(LCAB_PIN, ((FuncState & FN_BIT_06) >> 1) * cv_aux1_dim);    // AUX1 bei F6
        if (cv41_f7map & 0x80) analogWrite(LCAB_PIN, ((FuncState & FN_BIT_07) >> 2) * cv_aux1_dim);    // AUX1 bei F7
        if (cv42_f8map & 0x80) analogWrite(LCAB_PIN, ((FuncState & FN_BIT_08) >> 3) * cv_aux1_dim);    // AUX1 bei F8

        // PB3 kann kein PWM -> Soft-Dim in loop
        if (cv39_f5map & 0x01) aux2_enable = (FuncState & FN_BIT_05);         // AUX2 bei F5
        if (cv40_f6map & 0x01) aux2_enable = (FuncState & FN_BIT_06) >> 1;    // AUX2 bei F6
        if (cv41_f7map & 0x01) aux2_enable = (FuncState & FN_BIT_07) >> 2;    // AUX2 bei F7
        if (cv42_f8map & 0x01) aux2_enable = (FuncState & FN_BIT_08) >> 3;    // AUX2 bei F8
        break;

      case FN_9_12:
        if (cv43_f9map & 0x10) analogWrite(LCAB_PIN, ((FuncState & FN_BIT_09) * cv_aux1_dim));         // AUX1 bei F9
        if (cv44_f10map & 0x10) analogWrite(LCAB_PIN, ((FuncState & FN_BIT_10) >> 1) * cv_aux1_dim);   // AUX1 bei F10
        if (cv45_f11map & 0x10) analogWrite(LCAB_PIN, ((FuncState & FN_BIT_11) >> 2) * cv_aux1_dim);   // AUX1 bei F11
        if (cv46_f12map & 0x10) analogWrite(LCAB_PIN, ((FuncState & FN_BIT_12) >> 3) * cv_aux1_dim);   // AUX1 bei F12

        // PB3 kann kein PWM -> Soft-Dim in loop
        if (cv43_f9map & 0x20) aux2_enable = (FuncState & FN_BIT_09);         // AUX2 bei F9
        if (cv44_f10map & 0x20) aux2_enable = (FuncState & FN_BIT_10) >> 1;   // AUX2 bei F10
        if (cv45_f11map & 0x20) aux2_enable = (FuncState & FN_BIT_11) >> 2;   // AUX2 bei F11
        if (cv46_f12map & 0x20) aux2_enable = (FuncState & FN_BIT_12) >> 3;   // AUX2 bei F12
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
