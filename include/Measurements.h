#ifndef MEASUREMENTS_H
#define MEASUREMENTS_H

#include <vector>
#include <cstring>
#include <ArduinoJson.h>

class Measurements{
  int size_;
  int hashval_ = 0;

  struct SensorData {
    float temp;
    float dept;
    float oxyg;
  };

  std::vector<SensorData> data_;  // vector til at holde SensorData

public:
  Measurements();

  void setSize(int size);

  void setHashval(int hashval);

  int getSize();
  // Tilføjer data til arrays
  int putData(const char* buffer, int index);

  String toJson();
};
#endif // MEASUREMENTS_H