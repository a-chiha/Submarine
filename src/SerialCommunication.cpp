#include <Arduino.h>  // til Serial.begin()
#include "SerialCommunication.h"
#include "serialDebugger.h"

volatile int SerialCommunication::ID_ = 0;

/* --------- HJÆLPERFUNKTIONER ---------- */

// Funktion til at oversætte char værdier til deres navne
const char* translateCharToName(char value) {
    switch (value) {
        case ACK:
            return "ACK (Acknowledge)";
        case CONNECTION:
            return "CONNECTION (Connection type)";
        case PROGRAM:
            return "PROGRAM (Program type)";
        case HASH:
            return "HASH (Hash value)";
        case TRIG:
            return "TRIGGER (Trigger event)";
        case DATA:
            return "DATA (Data packet)";
        case SIZE:
            return "SIZE (Size of data)";
        case RESULT:
            return "RESULT (Result data)";
        case STX:
            return "STX (Start of text)";
        case ETX:
            return "ETX (End of text)";
        case FS:
            return "FS (File separator)";
        case NAK:
            return "NAK (Negative Acknowledge)";
        default:
            return "UNKNOWN";
    }
}

int charArrayToInt(const char *arr) { //Laver char array om til en int
    int result = 0;
    
    // Loop gennem arrayet
    while (*arr != '\0') {
        // Checker om indexet er et tal
        if (*arr >= '0' && *arr <= '9') {
            result = result * 10 + (*arr - '0');  // Konvertér char til int og summér
        }
        arr++;
    }
    return result;
}

char* getUntilChar(const char *buffer, char character, int startIndex, int& charIndex) {
    int i = startIndex;

    // Find hvor mange tegn der skal kopieres
    while (buffer[i] != '\0' && buffer[i] != character) {
        i++;
    }
    charIndex = i;
    // Allokér hukommelse til output-strengen (+1 for nul-terminering)
    int length = i - startIndex;
    char *output = (char *)malloc(i + 1);

    // Kopiér tegn fra buffer til output
    if (output != NULL) {
        strncpy(output, buffer, length);
        output[i] = '\0';  // Nulterminér output-strengen
    }

    return output;  // Returnér output-arrayet
}

int SerialCommunication::readByte() {
  while (!PSOCSerial.available()); // vent på byte
  if (PSOCSerial.available() > 0) {
    return PSOCSerial.read();  // Læs én byte og returner den
  } else {
    return -1;  // Hvis ingen data er tilgængelige, returner -1 som fejlkode
  }
}

int SerialCommunication::waitFor(char character) {
  char c;
  while (PSOCSerial.available() > 0){
    c = PSOCSerial.read();
    if (c == character) {
      return character; // Success
    }
  }

  return -1; // No data received
}

bool SerialCommunication::dataAvailable() {
  return PSOCSerial.available() > 0;  // Tjek om der er data tilgængelig
}


/* --------- KLASSE IMPLEMENTATIONER ---------- */

// Constructor-implementering
SerialCommunication::SerialCommunication(const int rx, const int tx)
  : PSOCSerial(2)
{
  PSOCSerial.begin(9600, SERIAL_8N1, rx, tx);  // PSOC UART
}

// Statisk metode til at få adgang til den eneste instans
SerialCommunication& SerialCommunication::getInstance(const int rx, const int tx) {
  static SerialCommunication instance(rx, tx);  // Opretter kun én instans
  return instance;
}

// Funktion til at sende pakker
int SerialCommunication::sendPacket(const char* message, char type) {
 if (serialDebug) Serial.printf("\nSending message: %s (type: %s). ", message, translateCharToName(type));
  memset(TXBuffer, 0, BUFSIZE); // Nulstiller alle elementer i bufferen til 0

  if (message == NULL) {
   if (serialDebug) Serial.println("Error: message is null");
    return -1;
  }

  ++ID_;
  char idString[10];  // Buffer til at holde ID som en streng
  sprintf(idString, "%d", ID_); // dette gøres for at kunne beregne størrelsen af ID
  int count = strlen(message) + strlen(idString)  + 5;  // +5 for FS(1), type(1), checksum(2), ETX(1)

  // Format message with error checking
  int formatted_len = sprintf(TXBuffer, "%c%c%s%c%c%s",
                      STX,
                      count,
                      idString,
                      FS,
                      type,
                      message);

  if (formatted_len < 0) {
   if (serialDebug) Serial.println("Error formatting message");
    return -1;
  }

  // Beregn checksum over hele beskeden fra count indtil nu
  uint16_t checksum = 0;
  for (int i = 2; i < formatted_len; ++i) {
    checksum += (uint8_t)TXBuffer[i];
  }
  checksum = checksum % 65536; // Sørg for, at checksummen er 16-bit
  if (serialDebug) Serial.printf("TX checksum: %d... ", checksum);

  // Tilføj checksum til meddelelsen (2 bytes i hexadecimal format)
  TXBuffer[formatted_len++] = (checksum >> 8) & 0xFF; // Høj byte
  TXBuffer[formatted_len++] = checksum & 0xFF;        // Lav byte

  // Tilføj ETX (end of transmission)
  TXBuffer[formatted_len++] = ETX;

  // Send message via UART
  PSOCSerial.write(TXBuffer, formatted_len);

  // Udskriv det sendte pakket indhold for debugging
 if (serialDebug) Serial.printf("Sent with ID: %d\r\n", ID_);

  return ID_;
}

// Funktion til at læse pakker
int SerialCommunication::readPacket(char* &buffer, int& ID, char& type) {
 if (serialDebug) Serial.print("\nReading packet. ");
  
  while (waitFor(STX) != STX); // venter på STX
 if (serialDebug) Serial.print("STX found. ");
  
  int count = readByte(); // Læs count  
  if (count < 5) return -1; // returnér fejl hvis der ikke er noget.
  memset(RXBuffer, 0, BUFSIZE); // Nulstiller alle elementer i bufferen til 0
  int i;
  for (i = 0; i < count; ++i) {
    RXBuffer[i] = readByte();
  }
  
  i -= 1; // sæt i 1 tilbage, da den øges på vej ud af loopet
 if (serialDebug) Serial.print("Bytes read.");
  
  // Kontroller om ETX er på sidste plads
  if (RXBuffer[i] != ETX) {
   if (serialDebug) Serial.println("Error. ETX not at last byte.");
   if (serialDebug) Serial.print("Read: ");
   if (serialDebug) Serial.println((int)RXBuffer[i]);
    memset(RXBuffer, 0, BUFSIZE); // Nulstiller alle elementer i bufferen til 0
    return -1;
  }
  
 if (serialDebug) Serial.print("ETX at last byte. ");
  
  // Udtræk den modtagne checksum (de sidste to bytes før ETX)
  uint16_t received_checksum = ((uint8_t)RXBuffer[i - 2] << 8) | (uint8_t)RXBuffer[i - 1];
  
  // Beregn checksum over hele beskeden (fra STX til før checksummen)
  uint16_t calculated_checksum = 0;
  for (int j = 0; j < (i - 2); ++j) { // Undgår de to sidste bytes (som er checksummen)
    calculated_checksum += (uint8_t)RXBuffer[j];
  }
  calculated_checksum = calculated_checksum % 65536; // Sørg for, at checksummen er 16-bit
  
  // Sammenlign modtaget og beregnet checksum
  if (received_checksum != calculated_checksum) {
   if (serialDebug) Serial.println("Error: checksum mismatch.");
   if (serialDebug) Serial.print("Received checksum: ");
   if (serialDebug) Serial.println(received_checksum, HEX);
   if (serialDebug) Serial.print("Calculated checksum: ");
   if (serialDebug) Serial.println(calculated_checksum, HEX);
    return -1; // Checksum fejler
  }
  
 if (serialDebug) Serial.print("Checksum OK. ");
  
  // Saml ID til int
  int idx; // int til index for type
  const char* stringId = getUntilChar(RXBuffer, FS, 0, idx);
  ID = charArrayToInt(stringId);
  type = RXBuffer[++idx];
  idx++;
  int msgSize = i - idx - 3 + 1; // -3 for checksum og ETX (+1 for 0 index)
 if (serialDebug) Serial.printf("Message was: %d bytes. ", msgSize);  // +1 for 0 index

  // Allokér buffer for at modtage beskeden
  buffer = new char[msgSize + 1]; // +1 for nulterminering
  if (buffer == nullptr) {
     if (serialDebug) Serial.println("Memory allocation failed!");
      return -1;
  }

  // Kopier data til buffer
  for (int j = 0; j < msgSize; j++) { // -3 for at ekskludere checksum og ETX
    buffer[j] = RXBuffer[j + idx]; // +idx for at starte efter ID og type
  }
  buffer[msgSize] = '\0'; // Afslutter strengen med 0-terminering
 if (serialDebug) Serial.printf("Message: %s, Type: %c, ID: %d\r\n", buffer, type, ID);
  return 0;
}


int SerialCommunication::resendPacket()
{
  PSOCSerial.print(TXBuffer);
  return ID_;
}
