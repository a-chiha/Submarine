#include "WifiManager.h"
#include "serialDebugger.h"

const char *thingSpeakURL = "http://192.168.0.1:8080"; // ThingSpeak API URL

// Forbind til hotspot
int WifiManager::connect(int tries){
  // print info til USBUART
  if (serialDebug) Serial.println();
  if (serialDebug) Serial.print("[WiFi] Connecting to ");
  if (serialDebug) Serial.println(SSID_);

  // Opretter forbindelse
  WiFi.begin(SSID_, PWD_);

  int tryDelay = 500; //ms der skal ventes mellem hvert forsøg
  int numberOfTries = tries; // Antal forsøg

  // Test of fejlbeskeder
  while (true) {
    switch (WiFi.status()) {
      case WL_NO_SSID_AVAIL: if (serialDebug) Serial.println("[WiFi] SSID not found"); break;
      case WL_CONNECT_FAILED:
        if (serialDebug) Serial.print("[WiFi] Failed - WiFi not connected! Reason: ");
        return -1;
        break;
      case WL_CONNECTION_LOST: if (serialDebug) Serial.println("[WiFi] Connection was lost"); break;
      case WL_SCAN_COMPLETED:  if (serialDebug) Serial.println("[WiFi] Scan is completed"); break;
      case WL_DISCONNECTED:    if (serialDebug) Serial.println("[WiFi] WiFi is disconnected"); break;
      case WL_CONNECTED:
        if (serialDebug) Serial.println("[WiFi] WiFi is connected!");
        if (serialDebug) Serial.print("[WiFi] IP address: ");
        if (serialDebug) Serial.println(WiFi.localIP());
        return 0;
        break;
      default:
        if (serialDebug) Serial.print("[WiFi] WiFi Status: ");
        if (serialDebug) Serial.println(WiFi.status());
        break;
    }
    delay(tryDelay);

    // Tjek om forsøg er opbrugt
    if (numberOfTries <= 0) {
      if (serialDebug) Serial.print("[WiFi] Failed to connect to WiFi!");
      WiFi.disconnect();
      return -1;
    } else {
      numberOfTries--; // dekrementer forsøgsantal
    }
  }
}

int WifiManager::sendMessage(const char* message)
{
  connect(100);

  HTTPClient http;
  String endpoint = String(URL_) + "/" + endpoints[0]; // Fuld endpoint URL for beskeder

  http.begin(endpoint); // Initialiser HTTP forbindelse
  http.addHeader("Content-Type", "text/plain"); // Sæt header

  // Opret en streng, der indeholder beskedtypen og selve beskeden
  String payload = String(message);

  // Send POST-anmodning
  int httpResponseCode = http.POST(payload);

  // Håndter HTTP svar
  if (httpResponseCode > 0) {
      if (serialDebug) Serial.print("[WiFi] Message sent, HTTP Response code: ");
      if (serialDebug) Serial.println(httpResponseCode);
  } else {
      if (serialDebug) Serial.print("[WiFi] Error sending message, HTTP Response code: ");
      if (serialDebug) Serial.println(httpResponseCode);
      http.end(); // Afslut forbindelse
      return -1; // Fejl
  }

  http.end(); // Afslut forbindelse
  return 0; // Succes
}



int WifiManager::getMessage(char* &message, char& type)
{
  connect(100);

  HTTPClient http;
  String endpoint = String(URL_) + "/" + endpoints[1]; // Fuld endpoint URL for beskeder

  http.begin(endpoint); // Initialiser HTTP forbindelse

  // Send GET-anmodning
  int httpResponseCode = http.GET();

  if (httpResponseCode == 200) {
      String response = http.getString(); // Læs svar fra server

      if (response.length() > 0) {
          type = response.charAt(0); // Første tegn som type
          message = strdup(response.c_str() + 1); // Resten som besked
          http.end();
          return 0; // Succes
      }
  } else {
      if (serialDebug) Serial.print("[WiFi] Error receiving message, HTTP Response code: ");
      if (serialDebug) Serial.println(httpResponseCode);
      http.end(); // Afslut forbindelse
      return -1; // Fejl
  }

  http.end(); // Afslut forbindelse
  return 0; // Succes
}


int WifiManager::sendData(const String& json)
{
  connect(100);
  HTTPClient http;
  String endpoint = String(URL_) + "/" + endpoints[2]; // Fuld endpoint URL for data

  http.begin(endpoint); // Initialiser HTTP forbindelse
  http.addHeader("Content-Type", "application/json"); // Sæt header

  // Send POST-anmodning
  int httpResponseCode = http.POST(json);

  if (httpResponseCode > 0) {
      if (serialDebug) Serial.print("[WiFi] Data sent, HTTP Response code: ");
      if (serialDebug) Serial.println(httpResponseCode);
  } else {
      if (serialDebug) Serial.print("[WiFi] Error sending data, HTTP Response code: ");
      if (serialDebug) Serial.println(httpResponseCode);
      http.end(); // Afslut forbindelse
      return -1; // Fejl
  }

  http.end(); // Afslut forbindelse
  return 1; // Succes
}

