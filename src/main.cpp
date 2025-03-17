#include "controller.h"
#include "serialDebugger.h"

#define TEMPFIELD "&field1="
#define DEPTFIELD "&field2="
#define OXYGFIELD "&field3="


bool serialDebug = false;  // Styrer om der udskrives nogte til usb uart

const int RX_PIN = 3;
const int TX_PIN = 1;

// Credentials for connection
const char *ssid = "testing";
const char *password = "testtest";

int arraySize = 10;

//ForetagMaalinger controller(RX_PIN, TX_PIN, ssid, pa6ssword, thingSpeakURL, thingSpeakWriteAPIKey);
ForetagMaalinger ctrl(RX_PIN, TX_PIN, ssid, password);

// Setup function
void setup() {
  if (serialDebug) 
    Serial.begin(115200);  // USB UART
}

// Loop function
void loop() {
  ctrl.begin();
} 