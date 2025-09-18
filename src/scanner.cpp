/*
   Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleScan.cpp
   Ported to Arduino ESP32 by Evandro Copercini
*/

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

#define LED_PIN 27   // LED pin

const char* TARGET_MAC = "80:f3:da:5e:f6:52";

const int RSSI_MIN = -80;   // weaker signal
const int RSSI_MAX = -60;   // stronger signal

const unsigned long ABSENCE_BUFFER_MS = 3000;

volatile unsigned long lastSeen = 0;
volatile int lastRSSI = -127;

BLEScan* pBLEScan = nullptr;

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    if (advertisedDevice.getAddress().toString() == TARGET_MAC) {
      lastSeen = millis();
      lastRSSI = advertisedDevice.getRSSI();
      Serial.printf("Seen target %s RSSI=%d\n", TARGET_MAC, lastRSSI);
    }
  }
};

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);
}

void loop() {
  pBLEScan->start(1, false);
  pBLEScan->stop();

  unsigned long now = millis();
  bool seenRecently = (now - lastSeen) < ABSENCE_BUFFER_MS;

  bool isClose = false;
  if (seenRecently) {
    if (lastRSSI >= RSSI_MIN && lastRSSI <= RSSI_MAX) {
      isClose = true;
    }
  }

  digitalWrite(LED_PIN, isClose ? HIGH : LOW);

  // Debug output
  Serial.printf("now=%lu lastSeen=%lu lastRSSI=%d seenRecently=%d isClose=%d --> LED %s\n",
                now, lastSeen, lastRSSI, seenRecently, isClose, isClose ? "ON" : "OFF");

  delay(500);
}
