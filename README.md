# Einfacher DCC-Funktionsdekoder mit ATtiny85 
## Ursprungs-Version
Die Dekoder-Software für die Arduino wurde abgeleitet von einem Sketch, der für ein kleines Digispark-Board mit ATtiny85 entworfen wurde.\
https://jo-fri.github.io/Eisenbahn/DCC_ATtiny85/index.html

Der Sketch verwendet die [NmraDcc-Bibliothek](https://github.com/mrrwa/NmraDcc) von [MRRWA](http://mrrwa.org/), die über die Arduino-Bibliotheksverwaltung eingebunden werden kann.

## Modifizierte Version
Für den Einsatz in einer Lichtleiste als Ersatz für bisherige Beleuchtung in einem [Modellbahnwagen](https://simandit.de/simwiki/doku.php?id=modellbahn:umbauten:rekowagen) habe ich die Hardware so modifiziert, so dass der Dekoder auf dem Programmiergleis programmiert und auch gelesen werden kann. Es wird an einem Ausgang dafür das ACK-Signal erzeugt. Damit sind jedoch nur drei der in der Ursprungsversion verfügbaren vier Ausgänge nutzbar.  
Auf der Beleuchtungsplatine ist ein Steckverbinder für einen ISP-Programmer vorhanden, damit kann auf den Bootloader wie beim Digispark-Board verzichtet werden.  

Folgende Funktionen sind möglich:
- F0 schaltet richtungsabhängig Beleuchtung der beider Seiten (z.B. Schlusslichter)
- F1 schaltet die Innenraumbeleuchtung

Weiterhin wurden Zusatzkonfigurationen mit den folgenden CVs ergänzt:  
 
- CV1  DCC-Adresse, default 3   
- CV8  Schreiben führt einen Decoder Reset mit Default-Werten aus
- CV29 Konfigurationsbyte, default 2
  - Bit0=1 Richtungsumkehr
- CV33-CV46 Funktionsmapping für Innenbeleuchtung F0-F12, default F1 (CV35=4)
  - CV33 default 1, F0-Vorwärts steuert PB0 (Schlusslicht Vorwärtsfahrt)
  - CV34 default 2, F0-Rückwärts steuert PB4 (Schlusslicht Rückwärtsfahrt), Mapping auf F1 ... F4 möglich
  - CV35 default 4, F1 steuert PB1 (Innenbeleuchtung), Mapping auf F0 ... F12 möglich
- CV49, CV50 für Steuerung der Schlussbeleuchtung
  - Bit7=1 immer an
  - Bit6=1 aus, wenn vorwärts
  - Bit5=1 aus, wenn rückwärts



