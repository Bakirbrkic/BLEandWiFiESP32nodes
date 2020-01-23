#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <WiFi.h>
#include <WebSocketsServer.h>

// network credentials
const char* ssid     = "CS510";
const char* password = "123456789";

//appData
String appCommand;
String ledState;

// socket socket
WebSocketsServer webSocket = WebSocketsServer(80);

//tempBradcast delay
int period = 5000;
unsigned long time_now = 0;

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define led_CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define temp_CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a9"

BLEServer *pServer;
BLEService *pService;
BLECharacteristic *ledChar;
BLECharacteristic *tempChar;

// Called when receiving any WebSocket message
void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length);

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE!");

  WiFi.softAP(ssid, password);

  BLEDevice::init("CS510");
  pServer = BLEDevice::createServer();
  pService = pServer->createService(SERVICE_UUID);
  ledChar = pService->createCharacteristic(led_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ);
  tempChar = pService->createCharacteristic(temp_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_WRITE);

  ledChar->setValue("0");
  pService->start();
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  BLEDevice::startAdvertising();
  Serial.println("Characteristics defined!");

  // Start WebSocket server and assign callback
  webSocket.begin();
  webSocket.onEvent(onWebSocketEvent);
}

void loop() {
  if(millis() >= (time_now + period)){
    time_now += period;
    webSocket.broadcastTXT(tempChar->getValue().c_str());
  }
  

  // Look for and handle WebSocket data
  webSocket.loop();  
}


void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length){
  // Figure out the type of WebSocket event
  switch(type) {

    // Client has disconnected
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;

    // New client has connected
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connection from ", num);
        Serial.println(ip.toString());
      }
      break;

    // Echo text message back to client
    case WStype_TEXT:
      Serial.printf("[%u] Text: %s\n", num, payload);
      //reinterpret_cast<char*>(payload);
      appCommand = "" + String(reinterpret_cast<char*>(payload));
      ledChar->setValue(appCommand.c_str());
      webSocket.sendTXT(num, "Characteristic Changed to" + appCommand);
      break;

    // For everything else: do nothing
    case WStype_BIN:
    case WStype_ERROR:
    case WStype_FRAGMENT_TEXT_START:
    case WStype_FRAGMENT_BIN_START:
    case WStype_FRAGMENT:
    case WStype_FRAGMENT_FIN:
    default:
      break;
  }
}