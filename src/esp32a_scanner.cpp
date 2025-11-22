/******************************************************************
 * ESP32-A Scanner Node Firmware
 * * Role: Scans for iBeacons, handles debounce/loitering logic,
 * timestamps with RTC, and sends to ESP32-B via ESP-NOW.
 * * Hardware: ESP32 Dev Board + DS3231 RTC
 ******************************************************************/

#include <WiFi.h>
#include <esp_now.h>
#include <BLEDevice.h>
#include <BLEAdvertisedDevice.h>
#include <BLEScan.h>
#include <Wire.h>
#include <RTClib.h>

// --- CONFIGURATION ---

// *** REPLACE THIS with the MAC Address of your Wi-Fi Node (ESP32-B) ***
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; 

const uint8_t TARGET_UUID[16] = {
  0x01, 0x12, 0x23, 0x34, 0x45, 0x56, 0x67, 0x78,
  0x89, 0x9a, 0xab, 0xbc, 0xcd, 0xde, 0xef, 0xf0
};

// TIMING SETTINGS (Anti-Loitering)
const unsigned long ENTER_DELAY_MS = 3000;  // Must be seen for 3s to "Enter"
const unsigned long EXIT_DELAY_MS  = 20000; // Must be gone for 20s to "Exit"

// --- END CONFIGURATION ---

// Data structure for ESP-NOW (Must match receiver)
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

// --- STUDENT TRACKING STRUCTURE ---
struct StudentState {
  int minor_id;           // ID
  bool isPresent;         // Is currently considered "Inside"
  unsigned long firstSeenTime; // When did we start seeing them?
  unsigned long lastSeenTime;  // When did we last see them?
  bool pendingEntry;      // Are we waiting to confirm entry?
};

// Tracking for Student ID 5 (expand this to an array for more students)
StudentState student5 = {5, false, 0, 0, false};

// --- HELPER FUNCTIONS ---

void getIsoTime(char *buffer) {
  DateTime now = rtc.now();
  sprintf(buffer, "%04d-%02d-%02dT%02d:%02d:%02dZ", 
          now.year(), now.month(), now.day(), 
          now.hour(), now.minute(), now.second());
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  // Optional: Print status to Serial Monitor for debugging
  // Serial.print("Packet Send Status: ");
  // Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

void sendEvent(int minor, const char* type) {
    // Prepare Payload
    espNowPayload.major_id = 101; 
    espNowPayload.minor_id = minor;
    strcpy(espNowPayload.event_type, type);
    getIsoTime(espNowPayload.timestamp_iso);
    strcpy(espNowPayload.mac_address, "XX:XX:XX:XX:XX:XX"); // Placeholder or real MAC

    Serial.printf(">>> SENDING EVENT: Student %d -> %s at %s\n", minor, type, espNowPayload.timestamp_iso);

    esp_now_send(broadcastAddress, (uint8_t *) &espNowPayload, sizeof(espNowPayload));
}

// --- BLE CALLBACKS ---

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      if (advertisedDevice.haveManufacturerData()) {
        String manuData = advertisedDevice.getManufacturerData();
        if (manuData.length() < 25) return;
        const uint8_t* payload = (const uint8_t*)manuData.c_str();

        // Scan for our Target UUID inside the payload
        bool uuidMatch = false;
        for (int i = 0; i < manuData.length() - 16; i++) {
            if (memcmp(payload + i, TARGET_UUID, 16) == 0) {
                
                // Extract Minor ID (Student ID)
                uint16_t minor = (payload[i+18] << 8) | payload[i+19];

                // --- LOGIC FOR STUDENT 5 ---
                if (minor == student5.minor_id) {
                    unsigned long now = millis();
                    
                    // Always update the "Last Seen" time immediately
                    student5.lastSeenTime = now;

                    // If NOT present yet: Check entry condition
                    if (!student5.isPresent) {
                        if (!student5.pendingEntry) {
                            // First detection! Start the timer.
                            student5.pendingEntry = true;
                            student5.firstSeenTime = now;
                            Serial.println("Student detected... verifying...");
                        } else {
                            // Already pending. Has it been long enough?
                            if (now - student5.firstSeenTime > ENTER_DELAY_MS) {
                                // YES! Confirmed Entry.
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

// --- SETUP ---

void setup() {
  Serial.begin(115200);
  
  // 1. Initialize RTC
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC! Check wiring.");
  }
  if (rtc.lostPower()) {
    Serial.println("RTC lost power, setting time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  // 2. Initialize Wi-Fi
  WiFi.mode(WIFI_STA);

  // 3. Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_send_cb(OnDataSent);

  // 4. Register Peer (ESP32-B)
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }

  // 5. Initialize BLE Scanner
  Serial.println("Starting BLE Scanner...");
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); 
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99); 
}

void loop() {
  // Scan for 1 second
  pBLEScan->start(1, false);
  pBLEScan->clearResults(); 

  // --- LOGIC: EXIT CHECK ---
  unsigned long now = millis();

  // Only check exit if they are currently marked "Present"
  if (student5.isPresent) {
      // Have they been gone longer than the EXIT_DELAY?
      if (now - student5.lastSeenTime > EXIT_DELAY_MS) {
          student5.isPresent = false;
          student5.pendingEntry = false; // Reset entry logic
          sendEvent(student5.minor_id, "exited");
      }
  } else {
      // If they are NOT present, but were pending entry and vanished
      if (student5.pendingEntry && (now - student5.lastSeenTime > ENTER_DELAY_MS)) {
          student5.pendingEntry = false;
          Serial.println("Student passed by (False Alarm)");
      }
  }
  
  delay(100);
}