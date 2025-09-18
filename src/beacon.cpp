/*
   Based on 31337Ghost's reference code from https://github.com/nkolban/esp32-snippets/issues/385#issuecomment-362535434
   which is based on pcbreflux's Arduino ESP32 port of Neil Kolban's example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleScan.cpp
*/

/*
   Create a BLE server that will send periodic iBeacon frames.
   The design of creating the BLE server is:
   1. Create a BLE Server
   2. Create advertising data
   3. Start advertising.
   4. wait
   5. Stop advertising.
*/

// Beacon.ino
// Minimal BLE advertiser for ESP32 - acts as a presence tag

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("Simple BLE Beacon starting...");

  BLEDevice::init("ESP32_Beacon");        // device name (optional)
  BLEServer* pServer = BLEDevice::createServer();

  BLEAdvertising* pAdvertising = pServer->getAdvertising();
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x06); // recommended settings
  pAdvertising->start();

  Serial.println("Advertising started.");
}

void loop() {
  // Nothing to do — just keep advertising
  delay(1000);
}
