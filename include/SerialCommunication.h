#ifndef SERIALCOMMUNICATION_H
#define SERIALCOMMUNICATION_H

#define CONNECTION  'C'
#define PROGRAM     'P'
#define HASH        'H'
#define TRIG        'T'
#define DATA        'D'
#define SIZE        'S'
#define RESULT      'R'
#define STX         02  // start of text
#define ETX         03  // end of text
#define FS          28  // file seperator
#define ACK         6   // Acknowledge

#include <HardwareSerial.h>

#define BUFSIZE 256

class SerialCommunication {
private:
  static volatile int ID_;
  char RXBuffer[BUFSIZE];  // Buffer til modtagelse af data
  char TXBuffer[BUFSIZE];  // Buffer til afsendelse af data
  HardwareSerial PSOCSerial;  // Brug UART2 til Serial2

  int readByte(); // Læs én byte. returnerer fejlkode -1, eller 0 for success.
  int waitFor(char character); // vent på bestemt tegn. returnerer fejlkode -1, eller modtaget tegn
  bool dataAvailable(); // Tjek om der er data tilgængelig

  // Privat constructor for at forhindre, at flere instanser oprettes
  SerialCommunication(const int rx, const int tx);
  // Privat copy constructor og assignment operator for at forhindre kopiering
  SerialCommunication(const SerialCommunication&) = delete;
  SerialCommunication& operator=(const SerialCommunication&) = delete;

public:
  // Statisk metode til at få adgang til den eneste instans
  static SerialCommunication& getInstance(const int rx, const int tx);
  int readPacket(char* &buffer, int& ID, char& type);
  int sendPacket(const char* message, char type);
  int resendPacket();
};

int charArrayToInt(const char *arr); // Hjælperfunktion

#endif
