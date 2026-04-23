# DCC-Funktionsdekoder mit ATtiny85 
## Ursprungs-Version
Die Dekoder-Software für die Arduino wurde abgeleitet von einem Sketch, der für ein kleines Digispark-Board mit ATtiny85 entworfen wurde.\
https://jo-fri.github.io/Eisenbahn/DCC_ATtiny85/index.html

Der Sketch verwendet die [NmraDcc-Bibliothek](https://github.com/mrrwa/NmraDcc) von [MRRWA](http://mrrwa.org/), die über die Arduino-Bibliotheksverwaltung eingebunden werden kann.

Zum Digispark-Board wurde von mir eine kleine [Zusatzplatine](http://simandit.de/simwiki/doku.php?id=modellbahn:umbauten:dcc-dekoder#funktions-dekoder_mit_digispark-board) entwickelt, mit der die Anschaltung an das Gleissignal erfolgt. Dies enthält weiterhin 3 Transistoren für die Funktionsausgänge, um z.B. die gleichgerichtete Gleisspannung zu schalten, und ein 4. Transistor zur Erzeugung des ACK-Signals. Damit sind jedoch nur noch drei der in der Ursprungsversion verfügbaren vier Ausgänge nutzbar. 

## Modifizierte Version
Für den Einsatz in einer LED-Lichtleiste als Ersatz für die vorhandene Beleuchtung mit zwei Birnchen in vierachsigen [PIKO-DR-Rekowagen](https://simandit.de/simwiki/doku.php?id=modellbahn:umbauten:rekowagen) habe ich die Dekoder-Hardware mit auf die Beleuchtungsplatine gebracht. Die Wagen sind jetzt auf dem Programmiergleis konfigurierbar.   
Auf der Beleuchtungsplatine ist ein Steckverbinder für einen ISP-Programmer vorhanden, damit kann auf den Bootloader wie beim Digispark-Board verzichtet werden, was Speicherplatz und Bootzeit spart.

Einen Dekoder mit dem Digispark-Board und der Zusatzplatine habe ich in einem [Packwagen](https://simandit.de/simwiki/doku.php?id=modellbahn:umbauten:pwg-daa) eingebaut.

Folgende Funktionen sind möglich:
- F0 schaltet richtungsabhängig PB0 (LV) und PB4 (LR), z.B. Schlusslichter beider Seiten
  - PB4 (LR) kann auf eine andere Funktionstaste gemappt und damit einzeln geschaltet werden
- F1 schaltet PB1 (AUX1), z.B. Innenraumbeleuchtung

Jetzt sind folgenden Konfigurationsvariablen (CVs) sind vorhanden:  
- CV1 DCC-Adresse, default 3 
- CV7 Versionsnummer
- CV8 Hersteller-ID
  - Schreiben auf CV8 führt einen Decoder Reset mit Default-Werten aus
- CV17/CV18 erweiterte DCC-Adresse (zusammen mit CV29-Bit5)
- CV29 Konfigurationsbyte
  - Bit0=1 Richtungsumkehr
  - Bit1=1 28/128 Fahrstufen (default
  - Bit5=1 erweiterte Adresse nutzen
- CV33-CV46 Funktionsmapping für PB0 (LV), PB4 (LR), PB1 (AUX1)
  - CV33 default 1, F0-Vorwärts steuert PB0 (LV, Ausgang Vorwärtsfahrt)
  - CV34 default 2, F0-Rückwärts steuert PB4 (LR, Ausgang Rückwärtsfahrt), Mapping auf F1 ... F4 möglich
  - CV35 default 4, F1 steuert PB1 (AUX1, z.B. Innenbeleuchtung), Mapping auf F0 ... F12 möglich
- CV49, CV50 für Steuerung der Ausgänge PB0 und PB4 (Schlussbeleuchtung)
  - Bit7=1 immer an
  - Bit6=1 aus, wenn vorwärts
   - Bit5=1 aus, wenn rückwärts
- CV51, CV52, CV53 für Dimmung der Ausgänge PB0, PB1, PB4
  - Bit0 ... Bit4 (0 ... 31), 0 wird auf 1 gesetzt

## Option 4 Ausgänge
Wenn auf das ACK-Signal zum Lesen der Konfigurationsvariablen verzichtet oder dieses zeitweilig mit einem Jumper/Schalter aktiviert werden kann, dann kann auch der Port PB3 als AUX2 genutzt werden. Die dafür gewählte Schaltung liegt im Ordner "hardware". 

Dieser 4. Port ist aber nicht dimmbar, PWM unterstützt der ATtiny85 an PB3 nicht.  
Alle anderen Funktionen sind weiterhin verfügbar, auch ein Funktionsmapping für AUX2 ist mit dem trotz vorhandenen Bootloader verbliebenen Speicherplatz geradeso möglich geworden.  
- F2 schaltet AUX2 (default), Funktionsmapping mit CV36 (default 8)

Ich nutze aber nur die Variante ohne Bootloader mit ISP-Programmierung.

