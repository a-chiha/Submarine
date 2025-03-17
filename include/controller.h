#pragma once

#include "SerialCommunication.h"
#include "Measurements.h"
#include "WifiManager.h"
#include "Blink.h"


class ForetagMaalinger {
  private:
    SerialCommunication& psoc_;  // Reference til SerialCommunication instans
    Measurements msrm_;
    WifiManager server_;
    Blink blinker_;
    int handlePacket(const char* message, int id, char type);
    void waitForAck(int packetId);
  public:
    // Constructor, som bruger getInstance for at få instansen af SerialCommunication
    ForetagMaalinger(int rx, int tx, const char* ssid, const char* password);
    void begin();
};