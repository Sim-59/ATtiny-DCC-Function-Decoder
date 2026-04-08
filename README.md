# Einfacher DCC Funktionsdekoder mit ATtiny85 
## Ursprungs-Version
Die Dekoder-Software für die Arduino wurde abgeleitet von einem Sketch, der für ein kleines Digispark-Board mit ATtiny85 entworfen wurde.\
https://jo-fri.github.io/Eisenbahn/DCC_ATtiny85/index.html

Der Sketch verwendet die NmraDcc-Bibliothek von http://mrrwa.org/, die über die Arduino-Bibliotheksverwaltung eingebunden werden kann.

## Modifizierte Version
Die Hardware wurde so modifiziert, so dass der Dekoder auf dem Programmiergleis programmiert und auch gelesen werden kann. Es wird an einem Ausgang dafür das ACK-Signal erzeugt. Damit sind 3 Ausgänge nutzbar.
Weiterhin wurden einige Zusatzkonfigurationen eingefügt.

Folgende Funktionen sind möglich:
- F0 schaltet richtungsabhängig Rücklichter der einen Seite
- F3 schaltet richtungsabhängig Rücklichter der anderen Seite
- F1 schaltet die Innenraumbeleuchtung

Es sind Konfigurationsvariablen vorhanden:
- CV1 - DCC-Adresse, default 3
- CV8 - Schreiben darauf erzeugt ein Reset auf Default-Einstellungen
- CV21 - Lichtkonfiguration, default 0
  - Bit7 (Wert 128) unterdrückt die Richtungsinformation, aktivierte Funktion immer an
  - Bit3 (Wert 8) unterdrückt F3-Funktion, LEDs an dieser Funktion immer aus
  - Bit0 (Wert 1) unterdrückt F0-Funktion, LEDs an dieser Funktion immer aus
- CV29 - Konfigurationsbyte, default 2 - wie bei Loks mit 28/128 Fahrstufen
  - Bit0 - dreht die Richtungsinformation des Wagens um

Einsatzbeispiel in einer Lichtleiste als Ersatz für eine Beleuchtung in einem Modellbahnwagen:\
https://simandit.de/simwiki/doku.php?id=modellbahn:umbauten:rekowagen
