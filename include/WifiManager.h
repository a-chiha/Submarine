#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

class WifiManager {
  // Credentials
  const char* SSID_;
  const char* PWD_;

  // Server info
  const char* URL_ = "http://192.168.0.1:8080";
  const char* endpoints[3] = {"api/postHardwareTestResult", "api/getMeasurementDepth", "api/postMeasurements"};

public:
  // Constructor
  WifiManager(const char* ssid, const char* password)
    : SSID_(ssid), PWD_(password) {}

  // Send besked
  int sendMessage(const char* message);

  // læs besked
  int getMessage(char* &message, char&);

  // send data
  int sendData(const String& json);

  // Forbind til hotspot
  int connect(int);
};

#endif //WIFIMANAGER_H