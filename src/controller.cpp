#include "controller.h"
#include "serialDebugger.h"
#include <Arduino.h>  // til Serial

/* --------- HJÆLPERFUNKTIONER -----------------*/

void ForetagMaalinger::waitForAck(int packetId)
{
  if (serialDebug) Serial.printf("\r\nWaiting for ID %d ACK", packetId);
  char* psoc_Buffer;
  char psoc_Type;
  int psoc_Id;
  while(1){
    psoc_.readPacket(psoc_Buffer, psoc_Id, psoc_Type);
    if (psoc_Type == ACK && charArrayToInt(psoc_Buffer) == packetId)
      break;
    psoc_.resendPacket();
    if (serialDebug) Serial.println("Resending packet...");
    delay(1000);
  }
  if (serialDebug) if (serialDebug) Serial.println("ACK received");
}

/* --------- KLASSE IMPLEMENTATIONER ---------- */

int ForetagMaalinger::handlePacket(const char *message, int id, char type)
{
  blinker_.turnOn();
  char idString[10];  // Buffer til at holde ID som en streng
  sprintf(idString, "%d", id);
  psoc_.sendPacket(idString, ACK); // sender ACK med modtaget ID
  if (type == TRIG){
    if (serialDebug) Serial.println("Handling trigger.");
    if (strcmp(message, "done") == 0){
      server_.sendData(msrm_.toJson());
      return 200;
    } 
    else {
      if (serialDebug) Serial.print("Unknown trigger: ");
      if (serialDebug) Serial.println(message);
      blinker_.fastBlink(50);
      return -1;
    }
  }
  else if (type == CONNECTION){
    if (serialDebug) Serial.println("Handling connection type");
    if (strcmp(message, "connect") == 0){
      blinker_.pulse();
      while (server_.connect(5)) blinker_.pulse();
      delay(500);
      blinker_.fastBlink(5);
      int packetId = psoc_.sendPacket("connected", CONNECTION);
      if (packetId < 0){
        if (serialDebug) Serial.print("Error occured while sending packet to psoc_.");
        blinker_.fastBlink(30);
      }
      waitForAck(packetId);
    } 
    else {
      if (serialDebug) Serial.print("Unknown connection message: ");
      if (serialDebug) Serial.println(message);
      blinker_.fastBlink(50);
      return -1;
    }
  }
  else if (type == SIZE){
    if (serialDebug) Serial.println("Resizing measurements vector...");
    msrm_.setSize(charArrayToInt(message));
  }
  else if (type == HASH){
    if (serialDebug) Serial.println("Setting hash value for measurements vector...");
    msrm_.setHashval(charArrayToInt(message));
  }
  else if (type == DATA){
    if (serialDebug) Serial.println("Saving data...");
    if (msrm_.putData(message, id)){ // Hvis der er overflow, udskriv størrelse
        if (serialDebug) Serial.print("Buffer overflow. Vector size: ");
        if (serialDebug) Serial.println(msrm_.getSize());
        blinker_.fastBlink(50);
        return -2;
    }
  }
  else if (type == RESULT){
    if (server_.sendMessage(message)){
      if (serialDebug) Serial.print("Error occured while sending message to server_.");
      blinker_.fastBlink(50);
      return -3;
    }
    char* server_Buffer;
    char server_Type;
    while (server_.getMessage(server_Buffer, server_Type))
      delay(3000);
    int packetId = psoc_.sendPacket(server_Buffer, server_Type);
    if (packetId < 0){
      if (serialDebug) Serial.print("Error occured while sending packet to psoc_.");
      blinker_.fastBlink(50);
      return -5;
    }
    waitForAck(packetId);
  }
  else {
    if (serialDebug) Serial.print("Unknown type (ASCII): ");
    if (serialDebug) Serial.print((int)type);
    blinker_.fastBlink(50);
    return -1;
  }
  blinker_.turnOff();
  return 0;
}

ForetagMaalinger::ForetagMaalinger(int rx, int tx, const char *ssid, const char *password)
    : psoc_(SerialCommunication::getInstance(rx, tx)), server_(ssid, password){}

void ForetagMaalinger::begin()
{
  while (true){
    char* buffer;  // Pointer til starten af bufferen
    int ID;
    char TYPE;
    int res = psoc_.readPacket(buffer, ID, TYPE);
    if (res){
      blinker_.fastBlink(30);
      if (serialDebug) Serial.print("Error: ");
      if (serialDebug) Serial.println(res);
      // Hvis buffer er allokeret, så frigør den
      if (buffer) {
        delete[] buffer;
      }
      return;
    }
    if (handlePacket(buffer, ID, TYPE) == 200) return;
    delete[] buffer;
  }
}