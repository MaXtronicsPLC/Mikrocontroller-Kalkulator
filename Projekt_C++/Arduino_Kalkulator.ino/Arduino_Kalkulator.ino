/************************************************************
 * Projekt :   Mikrocontroller-Kalkulator
 * Kurs    :   Programmierung mit C/C++ (DLBROEPRS01_D)
 * Autor   :   Maximilian H.
 ************************************************************/

#include <LiquidCrystal.h>                                      // LCD-Bibliothek
#include <Keypad.h>                                             // Keypad-Bibliothek

/* ===================== Hardware-Pins ===================== */

LiquidCrystal lcd(7, 6, 5, 4, 3, 2);                            // LCD (RS=7,E=6,D4..D7=5..2)

const byte p1=22, p2=23, p3=24, p4=25;                          // Flachband P1..P4
const byte p5=26, p6=27, p7=28, p8=29;                          // Flachband P5..P8

byte zeilenPins[4]  = { p8, p7, p6, p5 };                       // korrigierte Keypad-Zeilen
byte spaltenPins[4] = { p4, p3, p2, p1 };                       // korrigierte Keypad-Spalten

char tastenFeld[4][4] = {                                       // Standard-Layout
  {'1','2','3','A'},                                            // A = +
  {'4','5','6','B'},                                            // B = -
  {'7','8','9','C'},                                            // C = *
  {'*','0','#','D'}                                             // * = Clear, # = "=", D = /
};

Keypad tastenfeld = Keypad(makeKeymap(tastenFeld), zeilenPins, spaltenPins, 4, 4);

/* ===================== Zustand & Daten ==================== */

enum Zustand { ERSTE, OPERATOR, ZWEITE, FERTIG };                                   // Zustandsautomat
Zustand zustand = ERSTE;                                                            // Startzustand

String ersteZahl   = "";                                                            // erster Operand
String zweiteZahl  = "";                                                            // zweiter Operand
char   opZeichen   = 0;                                                             // Operator

String lcdOben     = "";                                                            // LCD-Zeile 1 Cache
String lcdUnten    = "";                                                            // LCD-Zeile 2 Cache

String letztesErgebnisSerial = "";                                                  // gemerktes Ergebnis für Weiterrechnen

/* ======================= Hilfsfunktionen ================== */

bool istZiffer(char c) { return c >= '0' && c <= '9'; }                             // prüft auf Ziffer

bool tasteZuOp(char k, char& op) {                                                  // Keypad → Operator
  if (k=='A') op='+'; else if (k=='B') op='-';
  else if (k=='C') op='*'; else if (k=='D') op='/';
  else return false;
  return true;
}

String trimNks(const String& s, char dez) {                                         // Nachkommastellen kürzen
  String t = s;
  while (t.endsWith("0")) t.remove(t.length()-1);
  if (t.endsWith(String(dez))) t.remove(t.length()-1);
  return t;
}

String formatiereSerial(float v) {                                                  // Ausgabe für Serial (Punkt)
  char buf[26]; dtostrf((double)v, 0, 3, buf);
  String s(buf); s.trim();
  return trimNks(s, '.');
}

String formatiereLCD(float v) {                                                     // Ausgabe für LCD (Komma)
  String s = formatiereSerial(v);
  s.replace('.', ',');
  return (s.length() <= 16) ? s : s.substring(0,16);
}

void lcdZeige(const String& oben, const String& unten="") {                         // LCD-Ausgabe (nur bei Änderung)
  String o = (oben.length()  <= 16) ? oben  : oben.substring(0,16);
  String u = (unten.length() <= 16) ? unten : unten.substring(0,16);
  if (o==lcdOben && u==lcdUnten) return;
  lcdOben=o; lcdUnten=u;
  lcd.clear();
  lcd.setCursor(0,0); lcd.print(o);
  lcd.setCursor(0,1); lcd.print(u);
}

void lcdStart() {                                                                   // Startbildschirm
  lcd.clear();                                                                      // Anzeige löschen
  lcd.setCursor(0, 0);  lcd.print("MC-Kalkulator");                                 // Zeile 1
  lcd.setCursor(0, 1);  lcd.print("Bereit...");                                     // Zeile 2
}

void lcdZeigeAusdruck() {                                                           // zeigt aktuellen Ausdruck
  String z = ersteZahl; if (opZeichen) z += opZeichen; z += zweiteZahl;
  lcdZeige(z, "");
}

/* ========================= Rechnen ======================== */

bool rechne(float& erg) {                                                           // Berechnung mit Fehlerkontrolle (true = OK, false = Fehler)
  if (!ersteZahl.length() || !zweiteZahl.length() || !opZeichen) return false;
  float a = ersteZahl.toFloat(), b = zweiteZahl.toFloat();
  switch (opZeichen) {
    case '+': erg = a + b; return true;                                             // Addition
    case '-': erg = a - b; return true;                                             // Subtraktion
    case '*': erg = a * b; return true;                                             // Multiplikation
    case '/': if (b==0.0f) return false; erg = a / b; return true;                  // Division
  }
  return false;
}

void zeigeErgebnisOderFehler(bool ok, float wert, const String& expr) {             // Ergebnis/Fehler auf LCD + Serial ausgeben
  if (!ok) {                                                                        // Fehlerfall
    if (opZeichen == '/' && zweiteZahl == "0") {                                    // DIV/0 erkannt
      lcdZeige(expr, "= DIV0"); 
      Serial.println("DIV0");
    } else {
      lcdZeige(expr, "= ERR"); 
      Serial.println("ERR");
    }
    letztesErgebnisSerial = "";                                                     // Weiterrechnen deaktivieren
    return;
  }
  String sLCD = formatiereLCD(wert);                                                // LCD-Ausgabe: Komma als Trennzeichen, auf 16 Zeichen begrenzen
  String sSER = formatiereSerial(wert);                                             // Serielle Ausgabe: Dezimalpunkt, überflüssige Nullen entfernen
  lcdZeige(expr, String("= ") + sLCD);                                              // Ausdruck + Ergebnis aufs LCD
  Serial.println(sSER);                                                             // Ergebnis an PC zurück
  letztesErgebnisSerial = sSER;                                                     // für Weiterrechnen merken
}

/* ========================== Reset ========================= */

void allesLoeschen() {                                                              // kompletter Reset
  ersteZahl=""; zweiteZahl=""; opZeichen=0;
  zustand = ERSTE;
  lcdStart();
}

/* ===================== Keypad-Eingabe ===================== */

void zifferHinzu(char k) {                                                          // Ziffer verarbeiten
  if (zustand==FERTIG) allesLoeschen();
  if (zustand==OPERATOR) zustand = ZWEITE;
  String& ziel = (zustand==ERSTE ? ersteZahl : zweiteZahl);
  if (ziel.length() < 10) { ziel += k; lcdZeigeAusdruck(); }
}

void verarbeiteTaste(char k) {                                                      // Tastenlogik
  if (!k) return;
  if (k=='*') { allesLoeschen(); return; }                                          // Clear
  if (k=='#') {                                                                     // "="
    if (zustand==ZWEITE && ersteZahl.length() && zweiteZahl.length() && opZeichen) {
      float r=0; String expr = ersteZahl + String(opZeichen) + zweiteZahl;
      bool ok = rechne(r);
      zeigeErgebnisOderFehler(ok, r, expr);
      zustand = FERTIG;                                                             // Ergebniszustand
    }
    return;
  }
  if (istZiffer(k)) { zifferHinzu(k); return; }                                     // Ziffer
  char op=0;
  if (tasteZuOp(k, op)) {                                                           // Operator
    if (zustand==FERTIG && letztesErgebnisSerial.length()) {                        // Weiterrechnen möglich
      ersteZahl = letztesErgebnisSerial;
      zweiteZahl = ""; opZeichen = op; zustand = OPERATOR;
      lcdZeigeAusdruck(); return;
    }
    if (zustand==ERSTE && ersteZahl.length()) {                                     // Operator nach 1. Zahl
      opZeichen = op; zustand = OPERATOR; lcdZeigeAusdruck(); return;
    }
    if (zustand==OPERATOR) {
      opZeichen = op; lcdZeigeAusdruck(); return;                                   // Umschalten vor 2. Zahl
    }
  }
}

/* ==================== Serielle Eingabe ==================== */

void verarbeiteSeriell(const String& zeile) {
  String s = zeile; s.trim(); if (!s.length()) return;
  String t=""; for (unsigned i=0;i<s.length();++i) { char c=s[i]; if (c!=' '&&c!='\t'&&c!='\r') t+=c; }
  int pos=-1; char op=0;
  for (unsigned i=0;i<t.length();++i) { char c=t[i]; if (c=='+'||c=='-'||c=='*'||c=='/'){ pos=i; op=c; break; } }
  if (pos<=0 || pos >= (int)t.length()-1) { Serial.println("ERR"); return; }
  String A=t.substring(0,pos), B=t.substring(pos+1);
  for (unsigned i=0;i<A.length();++i) if (!istZiffer(A[i])) { Serial.println("ERR"); return; }
  for (unsigned i=0;i<B.length();++i) if (!istZiffer(B[i])) { Serial.println("ERR"); return; }
  ersteZahl=A; zweiteZahl=B; opZeichen=op; zustand=ZWEITE;
  float r=0; bool ok = rechne(r);
  zeigeErgebnisOderFehler(ok, r, A+String(op)+B);
  zustand=FERTIG;
}

/* ======================= Setup / Loop ===================== */

void setup() {
  Serial.begin(9600);
  lcd.begin(16,2);                                                                 // LCD initialisieren
  lcdStart();                                                                      // Startbildschirm
}

void loop() {
  if (char t = tastenfeld.getKey()) verarbeiteTaste(t);                            // Keypad abfragen
  if (Serial.available()) {
    String zeile = Serial.readStringUntil('\n');
    verarbeiteSeriell(zeile);
  }
}
