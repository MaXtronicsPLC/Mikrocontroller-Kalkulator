# Projekt: Mikrocontroller-Kalkulator

**Kurs:** Programmierung mit C/C++ (DLBROEPRS01_D)  
**Autor:** Maximilian H.  

---

**Systemumgebung:** Die beschriebenen Schritte beziehen sich auf macOS. Unter Windows oder Linux können Pfade und Befehle abweichen.

## Einleitung
Dieses Projekt entstand im Rahmen des Kurses *Programmierung mit C/C++ (DLBROEPRS01_D)*.  
Ziel war die Entwicklung eines Taschenrechners, der auf einem **Arduino Mega 2560** und parallel auf einem **PC** genutzt werden kann.  

- Der Arduino verarbeitet Eingaben über ein 4x4-Keypad und zeigt Ergebnisse auf einem LCD-Display an.  
- Zusätzlich nimmt er Rechenausdrücke vom PC über die serielle Schnittstelle entgegen.  
- Das PC-Programm in C++ ermöglicht Eingaben im Terminal und protokolliert alle Ergebnisse in einer Textdatei.  

---

## Hardware
- Arduino Mega 2560  
- LCD Display 16x2 (HD44780 kompatibel)  
- 10 kΩ Potentiometer (Kontrasteinstellung LCD)  
- 4x4 Membran-Keypad (Pins 22–29 am Mega)  
- USB-Kabel zur Verbindung mit dem PC  

---

## Ordnerstruktur
```
Mikrocontroller-Kalkulator/
 ├── README.md                         (Projektbeschreibung, Hinweise, Installation)
 ├── LICENSE                           (Lizenzdatei, MIT License)
 └── Projekt_Cpp/
      ├── PC-Kalkulator/
      │    ├── pc_kalkulator.cpp       (PC-Programm in C++)
      │    ├── ergebnisse.txt          (wird automatisch erstellt)
      │    └── pc_kalkulator           (ausführbare Datei nach dem Kompilieren)
      └── Arduino_Kalkulator/
           └── Arduino_Kalkulator.ino  (Arduino-Sketch)
```

---

## Installation & Nutzung

### 0) Vorbereitung: Download & Entpacken
1. Repository auf GitHub öffnen → **Code → Download ZIP**.  
2. ZIP-Datei auf dem **Desktop** speichern.  
3. Entpacken → es entsteht der Ordner:  
   `~/Desktop/Mikrocontroller-Kalkulator-main/`

---

### 1) Arduino-Programm
1. Arduino IDE öffnen  
2. Board: **Arduino Mega 2560** auswählen  
3. Datei öffnen:  
   `Projekt_C++/Arduino_Kalkulator/Arduino_Kalkulator.ino`  
4. Sketch hochladen  
5. Anschlüsse prüfen:  
   - LCD: RS=7, E=6, D4..D7=5..2  
   - Keypad: P1→22, P2→23, P3→24, P4→25, P5→26, P6→27, P7→28, P8→29  
6. Baudrate: **9600**  

---

### 2) PC-Programm (macOS/Linux)
1. Terminal öffnen und in den Unterordner wechseln:
   ```bash
   cd ~/Desktop/Mikrocontroller-Kalkulator-main/Projekt_C++/PC-Kalkulator
   ```
2. Kompilieren:
   ```bash
   g++ -std=c++17 -o pc_kalkulator pc_kalkulator.cpp
   ```
   Falls `g++` fehlt: `xcode-select --install` (macOS).  

3. Starten:
   ```bash
   ./pc_kalkulator
   ```

4. Bedienung:  
   - Eingaben: `34 * 72`  
   - Ergebnis erscheint im Terminal  
   - Alle Eingaben & Ergebnisse werden in **ergebnisse.txt** gespeichert  
   - Beenden mit `exit` oder `quit`

---

## Hinweise
- Serial Monitor der Arduino IDE muss geschlossen sein, wenn das PC-Programm läuft (Ports blockieren sonst).  
- Dezimaltrennzeichen: LCD = **Komma**, Terminal = **Punkt**  
- Division durch 0 wird mit **DIV0** angezeigt  
- Typische Fehlerquellen: falscher Port oder geöffneter Serial Monitor  

---

## Dokumentation
Die vollständige Dokumentation wird **über PebblePad eingereicht**.  
Dieses Repository enthält nur den **Quellcode und technische Hinweise**.  

---

## Lizenz
Dieses Projekt steht unter der **MIT-Lizenz** (siehe `LICENSE`).  
