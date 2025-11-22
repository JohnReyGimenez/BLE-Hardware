/******************************************************************
 * BLE Attendance Tag (Beacon) Firmware - nRF52840
 * * Role: Continuously broadcasts a passive iBeacon frame.
 * * Hardware: nRF52840 Nano V2 Devboard
 * * Library: Adafruit Bluefruit nRF52 Libraries
 ******************************************************************/

#include <bluefruit.h>

// --- CONFIGURATION ---

// 1. UNIQUE PROJECT UUID (16 bytes)
uint8_t BEACON_UUID[16] = {
  0x01, 0x12, 0x23, 0x34, 0x45, 0x56, 0x67, 0x78,
  0x89, 0x9a, 0xab, 0xbc, 0xcd, 0xde, 0xef, 0xf0
};

// 2. TAG IDENTIFIERS
// MAJOR: Identifies the Scanner/Zone 
// MINOR: Identifies the specific Student 
#define TAG_MAJOR_ID 101 
#define TAG_MINOR_ID 5  // <-- CHANGE THIS for every new student tag (5, 6, 7...)

// 3. MANUFACTURER ID
#define MANUFACTURER_ID 0x0059

// 4. SIGNAL STRENGTH
// RSSI at 1 meter. Used for distance estimation.
#define SIGNAL_CALIBRATION -54 

// --- END CONFIGURATION ---

// Define the iBeacon structure
BLEBeacon beacon(BEACON_UUID, TAG_MAJOR_ID, TAG_MINOR_ID, SIGNAL_CALIBRATION);

void startAdv(void)
{  
  // Set the beacon payload
  Bluefruit.Advertising.setBeacon(beacon);
  Bluefruit.Advertising.setType(BLE_GAP_ADV_TYPE_NONCONNECTABLE_SCANNABLE_UNDIRECTED);
  
  // Set Interval: 160 * 0.625ms = 100 ms
  Bluefruit.Advertising.setInterval(160, 160); 
  
  // Start Advertising forever (0 = no timeout)
  Bluefruit.Advertising.start(0);                
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE Attendance Tag...");
  
  Bluefruit.begin();

  // Power Saving Settings
  Bluefruit.autoConnLed(false); // Turn off the blue LED to save battery
  Bluefruit.setTxPower(0);   

  // Set Manufacturer ID
  beacon.setManufacturer(MANUFACTURER_ID);

  // Start Broadcasting
  startAdv();

  Serial.printf("Broadcasting iBeacon - Major: %d, Minor: %d\n", TAG_MAJOR_ID, TAG_MINOR_ID);

  // Suspend the loop() to save max power. The radio runs in the background.
  suspendLoop();
}

void loop() {
  // Loop is suspended.
}