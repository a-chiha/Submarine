#include "Measurements.h"
#include <Arduino.h>
#include "serialDebugger.h"



// Implementering af putData-funktion
int Measurements::putData(const char* buffer, int index) {
  int hashedIndex = index - hashval_;
  if (hashedIndex < size_) {
    // de første 4 bytes i bufferen er en float (temperatur)
    memcpy(&data_[hashedIndex].temp, buffer, sizeof(float)); // Konverter de første 4 bytes til en float for temperatur
    if (serialDebug) Serial.printf("[ temp:%f, ", data_[hashedIndex].temp);

    // de næste 4 bytes i bufferen er en float (dybde)
    memcpy(&data_[hashedIndex].dept, buffer + 4, sizeof(float)); // Konverter de næste 4 bytes til en float for dybde
    if (serialDebug) Serial.printf("dept:%f,  ", data_[hashedIndex].dept);

    // de næste 4 bytes i bufferen er en float (oxygen)
    memcpy(&data_[hashedIndex].oxyg, buffer + 8, sizeof(float)); // Konverter de næste 4 bytes til en float for oxygen
    if (serialDebug) Serial.printf("oxyg:%f ] ", data_[hashedIndex].oxyg);
    if (serialDebug) Serial.printf(" saved at index %d\r\n.", hashedIndex);
    return 0;
  } else {
    if (serialDebug) Serial.println("Out of bounds.");
    return -1;
  }
}

String Measurements::toJson()
{
    // Opret en JsonDocument
    DynamicJsonDocument doc(1024);  // laver et json document

    // Opretter et array i JSON-dokumentet
    JsonArray array = doc.to<JsonArray>();

    // Gennemgå alle SensorData i vectoren og tilføj dem til JSON-arrayet
    for (const auto& data : data_) {
      JsonObject sensor = array.createNestedObject();
      sensor["temperatur"] = data.temp;
      sensor["dybde"] = data.dept;
      sensor["oxygen"] = data.oxyg;
    }

    // Serialiser JSON-dokumentet til en String og returner det
    String output;
    serializeJson(doc, output);
    return output;
}

int Measurements::getSize(){
  return size_;
}

Measurements::Measurements() : size_(0){}

void Measurements::setSize(int size)
{
    size_ = size;
    data_.resize(size);  // Reserver plads
    if (serialDebug) Serial.println("Size updated.");
}

void Measurements::setHashval(int hashval)
{
  hashval_ = hashval;
}
