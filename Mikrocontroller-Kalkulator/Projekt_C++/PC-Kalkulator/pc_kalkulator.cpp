/************************************************************
 * Projekt :   PC-Kalkulator – Serielle Kommunikation (macOS)
 * Kurs    :   Programmierung mit C/C++ (DLBROEPRS01_D)
 * Autor   :   Maximilian H.
 ************************************************************/

#include <iostream>     								// Terminal-Ein-/Ausgabe
#include <fstream>      								// Ergebnisse in Datei schreiben
#include <string>       								// String-Verarbeitung
#include <fcntl.h>      								// open()
#include <termios.h>    								// serielle Schnittstelle (POSIX)
#include <unistd.h>     								// read(), write(), close(), usleep(), sleep()
#include <cstring>      								// strerror()
#include <cctype>       								// isspace(), isdigit()

/* ===================== Port & Protokoll ================== */

static const std::string PORT = "/dev/cu.usbmodem101"; 					// fester macOS-Port
static const speed_t     BAUD = B9600;                  				// 9600 Baud (8N1, Raw-Mode)
                                                        				// Protokoll: "ersteZahl Operator zweiteZahl\n"
											// Erlaubte Operatoren: + - * /
											// Beispiel-Eingabe: "25 * 18\n"

/* ===================== Port einstellen =================== */
											// setzt 9600 Baud, 8N1, Raw-Mode (keine automatische Konvertierung)
bool konfigurierePort(int fd, speed_t baud = BAUD) {
    termios t{};
    if (tcgetattr(fd, &t) != 0) return false;
    cfsetispeed(&t, baud); cfsetospeed(&t, baud);
    t.c_cflag = (t.c_cflag & ~CSIZE) | CS8;            					// 8 Datenbits
    t.c_cflag |= (CLOCAL | CREAD);                     					// Empfänger an, kein Modem
    t.c_cflag &= ~(PARENB | PARODD | CSTOPB | CRTSCTS);					// keine Parität/Handshake, 1 Stopbit
    t.c_iflag = 0; t.c_oflag = 0; t.c_lflag = 0;       					// Raw-Mode (keine Zeilenverarbeitung)
    t.c_cc[VMIN] = 0; t.c_cc[VTIME] = 20;              					// read-Timeout 2.0 s (1/10 s Einheiten)
    if (tcsetattr(fd, TCSANOW, &t) != 0) return false;
    tcflush(fd, TCIOFLUSH);                            					// Startpuffer leeren (Reset-Artefakte)
    return true;
}

/* ===================== Senden / Lesen ==================== */

bool sendeZeile(int fd, const std::string& zeile) {     				// sendet Text + '\n'
    std::string s = zeile;
    if (s.empty() || s.back() != '\n') s.push_back('\n');
    return write(fd, s.c_str(), s.size()) == (ssize_t)s.size();
}

void spuelEingang(int fd) {                             				// Eingangs-Puffer leeren
    char tmp[256]; usleep(1000);                        				// kurze Wartezeit nach Port-Open
    while (true) { ssize_t n = read(fd, tmp, sizeof(tmp)); if (n <= 0) break; }
}

std::string trim(const std::string& s) {                				// Leerzeichen, Tabs, Zeilenumbrüche entfernen
    size_t a = 0, b = s.size();
    while (a < b && std::isspace((unsigned char)s[a])) ++a;
    while (b > a && std::isspace((unsigned char)s[b-1])) --b;
    return s.substr(a, b - a);
}

bool istErgebniszeile(const std::string& z) {           				// Zahl (+/-, optional .) oder ERR/DIV0
    if (z == "ERR" || z == "DIV0") return true;
    if (z.empty()) return false;
    size_t i = 0; if (z[i] == '+' || z[i] == '-') ++i;
    bool mind1 = false;
    while (i < z.size() && std::isdigit((unsigned char)z[i])) { mind1 = true; ++i; }
    if (i < z.size() && z[i] == '.') {                  				// Dezimalpunkt für Terminal-Ausgabe
        ++i; while (i < z.size() && std::isdigit((unsigned char)z[i])) ++i;
    }
    return mind1 && i == z.size();
}

std::string liesErgebnis(int fd, int maxZeitMs = 5000) {				// liest bis gültige Ergebniszeile ankommt
    std::string zeile; int gewartet = 0; char c;
    while (gewartet < maxZeitMs) {
        ssize_t n = read(fd, &c, 1);
        if (n == 1) {
            if (c == '\n' || c == '\r') {
                std::string z = trim(zeile); zeile.clear();
                if (!z.empty() && istErgebniszeile(z)) return z; 			// gültige Ergebniszeile
                // sonst: andere Zeilen (READY/Debug) ignorieren
            } else {
                zeile.push_back(c);
            }
        } else {
            usleep(10000); gewartet += 10;              				// 10 ms warten (Timeout-Fenster)
        }
    }
    return "";                                          				// Timeout ohne gültige Antwort
}

/* ====================== Hauptprogramm ==================== */

int main() {
    int fd = open(PORT.c_str(), O_RDWR | O_NOCTTY | O_SYNC);				// Port öffnen (fester Port)
    if (fd < 0) { std::cerr << "Kann Port nicht öffnen (" << PORT << "): "
                            << strerror(errno) << "\n"; return 1; }

    if (!konfigurierePort(fd)) {
        std::cerr << "Port-Konfiguration fehlgeschlagen.\n";
        close(fd); return 1;
    }

    sleep(2);                                           				// Arduino-Reset nach Port-Open abwarten
    std::cout << "Verbunden mit: " << PORT << "\n";
    std::cout << "Mehrfach-Eingabe aktiv. 'exit' zum Beenden.\n";

    spuelEingang(fd);                                   				// evtl. Start-/Reset-Ausgaben vom Arduino löschen

    while (true) {                                      				// Hauptschleife: mehrere Rechnungen nacheinander
        std::cout << "Eingabe (z. B. 54 * 22): ";
        std::string expr; std::getline(std::cin, expr);
        if (expr == "exit" || expr == "quit") break;
        if (expr.empty()) continue;

        spuelEingang(fd);                               				// vor neuer Anfrage Puffer leeren
        if (!sendeZeile(fd, expr)) {
            std::cerr << "Senden fehlgeschlagen.\n";
            continue;
        }

        std::string antwort = liesErgebnis(fd);         				// wartet bis Ergebnis / ERR / DIV0
        if (antwort.empty()) {
            std::cout << "Keine Antwort (Timeout).\n";
            continue;
        }

        std::cout << expr << " = " << antwort << "\n";  				// Terminal-Ausgabe
        std::ofstream log("ergebnisse.txt", std::ios::app);
        log << expr << " = " << antwort << std::endl;   				// dauerhaft mitschreiben
    }

    close(fd);                                          				// Port schließen
    return 0;
}