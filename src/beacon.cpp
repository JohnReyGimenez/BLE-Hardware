#include <Arduino.h>
#include <NimBLEDevice.h>

#define GREEN_LED 2
#define RED_LED 4

String TARGET_UUID = "12345678-1234-1234-1234-123456789abc";
bool seenBefore = false;

class MyAdvertisedDeviceCallbacks : public NimBLEAdvertisedDeviceCallbacks {
  void onResult(NimBLEAdvertisedDevice* advertisedDevice) override {
    String uuid = advertisedDevice->getServiceUUID().toString().c_str();
    if (uuid.equalsIgnoreCase(TARGET_UUID)) {
      Serial.printf("Found target UUID %s | RSSI: %d\n", uuid.c_str(), advertisedDevice->getRSSI());
      if (seenBefore) {
        digitalWrite(RED_LED, HIGH);
        delay(1000);
        digitalWrite(RED_LED, LOW);
      } else {
        digitalWrite(GREEN_LED, HIGH);
        delay(1000);
        digitalWrite(GREEN_LED, LOW);
        seenBefore = true;
      }
    }
  }
};

void setup() {
  Serial.begin(115200);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);

  NimBLEDevice::init("BLE-Scanner");
  NimBLEScan* pBLEScan = NimBLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->start(0, nullptr, false); // 0 = continuous scanning
}

void loop() {
  // Nothing here, scanning runs in background
}
