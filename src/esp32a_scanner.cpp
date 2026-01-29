#include <WiFi.h>
#include <esp_now.h>
#include <BLEDevice.h>
#include <BLEAdvertisedDevice.h>
#include <BLEScan.h>
#include <Wire.h>
#include <RTClib.h>

uint8_t broadcastAddress[] = {0x80, 0xF3, 0xDA, 0x5E, 0xF6, 0x50}; 


const uint8_t TARGET_UUID[16] = {
  0x01, 0x12, 0x23, 0x34, 0x45, 0x56, 0x67, 0x78,
  0x89, 0x9a, 0xab, 0xbc, 0xcd, 0xde, 0xef, 0xf0
};


const unsigned long ENTER_DELAY_MS = 3000;  // 3s to Enter
const unsigned long EXIT_DELAY_MS  = 5000;  // 5s to Exit

// --- END CONFIGURATION ---

typedef struct struct_message {
  char mac_address[18];
  int major_id;
  int minor_id;
  char event_type[10];
  char timestamp_iso[25];
} struct_message;

struct_message espNowPayload;
RTC_DS3231 rtc;
BLEScan* pBLEScan;

struct StudentState {
  int minor_id;           
  bool isPresent;         
  unsigned long firstSeenTime; 
  unsigned long lastSeenTime;  
  bool pendingEntry;      
};

StudentState student5 = {5, false, 0, 0, false};

void getIsoTime(char *buffer) {
  DateTime now = rtc.now();
  sprintf(buffer, "%04d-%02d-%02dT%02d:%02d:%02dZ", 
          now.year(), now.month(), now.day(), 
          now.hour(), now.minute(), now.second());
}

void sendEvent(int minor, const char* type) {
    espNowPayload.major_id = 101; 
    espNowPayload.minor_id = minor;
    strcpy(espNowPayload.event_type, type);
    getIsoTime(espNowPayload.timestamp_iso);
    strcpy(espNowPayload.mac_address, "80:F3:DA:5E:05:84"); // Scanner MAC

    Serial.printf(">>> SENDING EVENT: Student %d -> %s\n", minor, type);

    esp_now_send(broadcastAddress, (uint8_t *) &espNowPayload, sizeof(espNowPayload));
}

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      if (advertisedDevice.haveManufacturerData()) {
        String manuData = advertisedDevice.getManufacturerData();
        if (manuData.length() < 25) return;
        const uint8_t* payload = (const uint8_t*)manuData.c_str();

        for (int i = 0; i < manuData.length() - 16; i++) {
            if (memcmp(payload + i, TARGET_UUID, 16) == 0) {
                uint16_t minor = (payload[i+18] << 8) | payload[i+19];

                if (minor == student5.minor_id) {
                    unsigned long now = millis();
                    student5.lastSeenTime = now;

                    if (!student5.isPresent) {
                        if (!student5.pendingEntry) {
                            student5.pendingEntry = true;
                            student5.firstSeenTime = now;
                            Serial.println("Student detected... verifying...");
                        } else {
                            if (now - student5.firstSeenTime > ENTER_DELAY_MS) {
                                student5.isPresent = true;
                                student5.pendingEntry = false;
                                sendEvent(student5.minor_id, "entered");
                            }
                        }
                    }
                }
                break; 
            }
        }
      }
    }
};

void setup() {
  Serial.begin(115200);

  if (!rtc.begin()) { Serial.println("Couldn't find RTC!"); }
  if (rtc.lostPower()) { rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); }

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  if (esp_now_init() != ESP_OK) { Serial.println("Error initializing ESP-NOW"); return; }
  
  esp_now_peer_info_t peerInfo;
  memset(&peerInfo, 0, sizeof(peerInfo));
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK){ Serial.println("Failed to add peer"); return; }

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); 
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99); 
  Serial.println("Scanner Ready.");
}

void loop() {
  pBLEScan->start(1, false);
  pBLEScan->clearResults(); 

  unsigned long now = millis();

  if (student5.isPresent) {
      if (now - student5.lastSeenTime > EXIT_DELAY_MS) {
          student5.isPresent = false;
          student5.pendingEntry = false; 
          sendEvent(student5.minor_id, "exited");
      }
  } else {
      if (student5.pendingEntry && (now - student5.lastSeenTime > ENTER_DELAY_MS)) {
          student5.pendingEntry = false;
      }
  }
  delay(100);
}