/**
 * A BLE client example that is rich in capabilities.
 * There is a lot new capabilities implemented.
 * author unknown
 * updated by chegewara
 */

#include <Arduino.h>
#include "BLEDevice.h"
#include "DHT.h"

//led pin
int ledPin = 32;

//temperature sensor
DHT dht;
int dhtPin = 23;
float humidity;
float temperature;
int period = 5000;
unsigned long time_now = 0;

// The remote service we wish to connect to.
static BLEUUID serviceUUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
// The characteristics of the remote service we are interested in.
static BLEUUID led_charUUID("beb5483e-36e1-4688-b7f5-ea07361b26a8");
static BLEUUID temp_charUUID("beb5483e-36e1-4688-b7f5-ea07361b26a9");

static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLERemoteCharacteristic* led_Characteristic;
static BLERemoteCharacteristic* temp_Characteristic;
static BLEAdvertisedDevice* myDevice;

static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
    Serial.print("Notify callback for characteristic ");
    Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
    Serial.print(" of data length ");
    Serial.println(length);
    Serial.print("data: ");
    Serial.println((char*)pData);
}

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
    connected = true;
    Serial.println("Connect");
  }

  void onDisconnect(BLEClient* pclient) {
    connected = false;
    Serial.println("onDisconnect");
  }
};

bool connectToServer() {
    Serial.print("Forming a connection to ");
    Serial.println(myDevice->getAddress().toString().c_str());
    
    BLEClient*  pClient  = BLEDevice::createClient();
    Serial.println(" - Created client");

    pClient->setClientCallbacks(new MyClientCallback());

    // Connect to the remove BLE Server.
    pClient->connect(myDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
    Serial.println(" - Connected to server");

    // Obtain a reference to the service we are after in the remote BLE server.
    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService == nullptr) {
      Serial.print("Failed to find our service UUID: ");
      Serial.println(serviceUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our service");


    // Obtain a reference to the characteristic in the service of the remote BLE server.
    led_Characteristic = pRemoteService->getCharacteristic(led_charUUID);
    if (led_Characteristic == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(led_charUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our LED characteristic");

    // Obtain a reference to the characteristic in the service of the remote BLE server.
    temp_Characteristic = pRemoteService->getCharacteristic(temp_charUUID);
    if (temp_Characteristic == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(led_charUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our TEMP characteristic");

    // Read the value of the characteristic.
    if(led_Characteristic->canRead()) {
      std::string value = led_Characteristic->readValue();
      Serial.print("The characteristic value was: ");
      Serial.println(value.c_str());
      if(value.c_str()[0] == '1'){
        Serial.print("walue evaluated as 1");
        digitalWrite(ledPin, 1);
      }
      if(value.c_str()[0] == '0'){
        Serial.print("walue evaluated as 0");
        digitalWrite(ledPin, 0);
      }
        
    }

    if(temp_Characteristic->canNotify())
      temp_Characteristic->registerForNotify(notifyCallback);

    connected = true;
}
/**
 * Scan for BLE servers and find the first one that advertises the service we are looking for.
 */
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
 /**
   * Called for each advertising BLE server.
   */
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print("BLE Advertised Device found: ");
    Serial.println(advertisedDevice.toString().c_str());

    // We have found a device, let us now see if it contains the service we are looking for.
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {

      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      doScan = true;

    } // Found our server
  } // onResult
}; // MyAdvertisedDeviceCallbacks


void setup() {
  Serial.begin(115200);
  Serial.println("Starting Arduino BLE Client application...");
  BLEDevice::init("");
  
  //for led
  pinMode(ledPin,OUTPUT);

  //for temperature
  dht.setup(dhtPin);

  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 5 seconds.
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);
} // End of setup.


// This is the Arduino main loop function.
void loop() {

  // If the flag "doConnect" is true then we have scanned for and found the desired
  // BLE Server with which we wish to connect.  Now we connect to it.  Once we are 
  // connected we set the connected flag to be true.
  if (doConnect == true) {
    if (connectToServer()) {
      Serial.println("We are now connected to the BLE Server.");
    } else {
      Serial.println("We have failed to connect to the server; there is nothin more we will do.");
    }
    doConnect = false;
  }

  // If we are connected to a peer BLE Server, update the characteristic each time we are reached
  // with the current time since boot.
  if (connected) {
    
    if(millis() >= (time_now + period)){
      //read sensor
      humidity = dht.getHumidity();
      temperature = dht.getTemperature();
      //construct message
      String newValue = String(temperature) + "!" + String(humidity);
      Serial.println("Setting new characteristic value to \"" + newValue + "\"");
      // Set the characteristic's value to be the array of bytes that is actually a string.
      temp_Characteristic->writeValue(newValue.c_str(), newValue.length());
      time_now+=period;
    }

    // Read the value of the characteristic.
    if(led_Characteristic->canRead()) {
      std::string value = led_Characteristic->readValue();
      Serial.print("The characteristic value was: ");
      Serial.println(value.c_str());
      if(value.c_str()[0] == '1'){
        Serial.print("walue evaluated as 1");
        digitalWrite(ledPin, 1);
      }
      if(value.c_str()[0] == '0'){
        Serial.print("walue evaluated as 0");
        digitalWrite(ledPin, 0);
      }
        
    }
  }else if(doScan){
    BLEDevice::getScan()->start(0);  // this is just eample to start scan after disconnect, most likely there is better way to do it in arduino
  }

  delay(1000); // Delay a second between loops.
} // End of loop