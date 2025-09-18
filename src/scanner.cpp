/*
   Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleScan.cpp
   Ported to Arduino ESP32 by Evandro Copercini
*/

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

#define GREEN_PIN 27   
#define RED_PIN   25   

const char* TARGET_MAC = "80:f3:da:5e:f6:52"; 

const int RSSI_MIN = 60;
const int RSSI_MAX = 80;
const unsigned long ABSENCE_BUFFER_MS = 3000; // how long without seeing the beacon before exit
const unsigned long LED_BLINK_MS = 2000;     // green/red blink duration

volatile unsigned long lastSeen = 0;
volatile int lastRSSI = -127;

enum State { ABSENT, PRESENT };
State state = ABSENT;

BLEScan* pBLEScan = nullptr;

// Simple moving average for RSSI smoothing
const int RSSI_HISTORY = 3;
int rssiBuffer[RSSI_HISTORY] = {0};
int rssiIndex = 0;
int rssiCount = 0;

String toLowercase(const std::string &s) {
  String out(s.c_str());
  out.toLowerCase();
  return out;
}

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) override {
    String mac = advertisedDevice.getAddress().toString().c_str();
    int rssi = advertisedDevice.getRSSI();

    if (strlen(TARGET_MAC) == 0) {
      Serial.printf("Found: %s  RSSI=%d  Name=%s\n", mac.c_str(), rssi,
                    advertisedDevice.haveName() ? advertisedDevice.getName().c_str() : "n/a");
    } else {
      String targetLower = String(TARGET_MAC);
      targetLower.toLowerCase();
      mac.toLowerCase();
      if (mac.equalsIgnoreCase(targetLower)) {
        lastSeen = millis();

        // Add new RSSI to buffer and calculate average
        rssiBuffer[rssiIndex] = rssi;
        rssiIndex = (rssiIndex + 1) % RSSI_HISTORY;
        if (rssiCount < RSSI_HISTORY) rssiCount++;

        int sum = 0;
        for (int i = 0; i < rssiCount; i++) sum += rssiBuffer[i];
        lastRSSI = sum / rssiCount;

        Serial.printf("Seen target %s  RSSI=%d (avg=%d)\n", mac.c_str(), rssi, lastRSSI);
      }
    }
  }
};

void blinkLED(int pin, unsigned long ms) {
  digitalWrite(pin, HIGH);
  delay(ms);
  digitalWrite(pin, LOW);
}

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("Scanner starting...");

  pinMode(GREEN_PIN, OUTPUT);
  pinMode(RED_PIN, OUTPUT);
  digitalWrite(GREEN_PIN, LOW);
  digitalWrite(RED_PIN, LOW);

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);

  Serial.println("BLE scan ready.");
  if (strlen(TARGET_MAC) == 0) {
    Serial.println("TARGET_MAC is empty: scanning all devices. Copy the MAC you want.");
  } else {
    Serial.printf("Looking for MAC: %s (RSSI range: %d-%d)\n", TARGET_MAC, RSSI_MIN, RSSI_MAX);
  }
}

void loop() {
  // Short blocking scan (~1 second)
  pBLEScan->start(1, false);

  unsigned long now = millis();
  bool seenRecently = (lastSeen > 0) && (now - lastSeen <= ABSENCE_BUFFER_MS);
  bool isClose = (lastRSSI >= RSSI_MIN && lastRSSI <= RSSI_MAX);

  if (seenRecently && isClose) {
    if (state == ABSENT) {
      state = PRESENT;
      Serial.println("ENTRY detected: beacon present (green blink)");
      blinkLED(GREEN_PIN, LED_BLINK_MS);
    }
  } else {
    if (state == PRESENT && (now - lastSeen > ABSENCE_BUFFER_MS)) {
      state = ABSENT;
      Serial.println("EXIT detected: beacon lost (red blink)");
      blinkLED(RED_PIN, LED_BLINK_MS);
    }
  }
}