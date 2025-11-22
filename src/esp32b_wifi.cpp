/******************************************************************
 * ESP32-B Wi-Fi Node Firmware
 * * Role: Receives ESP-NOW data from Scanner (A) and POSTs to Rails API.
 * * Hardware: ESP32 Dev Board
 ******************************************************************/

#include <WiFi.h>
#include <esp_now.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// --- CONFIGURATION ---
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Replace with computer's IP address 
const char* serverUrl = "http://RAILS_IP:3000/api/v1/attendances";

// --- END CONFIGURATION ---
typedef struct struct_message {
  char mac_address[18];
  int major_id;
  int minor_id;
  char event_type[10];
  char timestamp_iso[25];
} struct_message;

struct_message incomingReadings;

// Callback when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
  
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.print("Event: ");
  Serial.println(incomingReadings.event_type);
  Serial.print("Time: ");
  Serial.println(incomingReadings.timestamp_iso);

  sendToAPI();
}

void sendToAPI() {
  if(WiFi.status() == WL_CONNECTED){
    HTTPClient http;
    
    // Start connection
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json");
    
    // Create JSON Payload
    StaticJsonDocument<200> doc;
    
    // Note: Rails API expects 'mac_address', 'event_type', 'timestamp'
    // For this demo, pass the data we received.
    doc["mac_address"] = incomingReadings.mac_address; 
    doc["event_type"] = incomingReadings.event_type;
    doc["timestamp"] = incomingReadings.timestamp_iso;
    
    // Optional: send major/minor if your API accepts them
    // doc["major"] = incomingReadings.major_id;
    // doc["minor"] = incomingReadings.minor_id;

    String requestBody;
    serializeJson(doc, requestBody);
    
    Serial.println("Sending JSON: " + requestBody);

    // POST Request
    int httpResponseCode = http.POST(requestBody);
    
    if(httpResponseCode > 0){
      String response = http.getString();
      Serial.print("Response Code: "); 
      Serial.println(httpResponseCode);
      Serial.println(response);
    }
    else {
      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCode);
    }
    
    http.end();
  }
  else {
    Serial.println("WiFi Disconnected");
  }
}

void setup() {
  Serial.begin(115200);
  
  // 1. Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected! IP: " + WiFi.localIP().toString());
  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress()); // <--- COPY THIS MAC ADDRESS FOR ESP32-A

  // 2. Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // 3. Register Receive Callback
  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
  delay(2000);
}