#include <WiFi.h>
#include <esp_now.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid     = "Manganese";
const char* password = "loveloverey81";

const char* serverUrl = "http://10.23.42.253:3000/api/v1/attendances";

typedef struct struct_message {
  char mac_address[18];
  int major_id;
  int minor_id;
  char event_type[10];
  char timestamp_iso[25];
} struct_message;

struct_message espNowPayload;

void sendAttendanceToRails(const char* mac, int major, int minor, const char* event, const char* time_iso) {
  if(WiFi.status() == WL_CONNECTED){
    HTTPClient http;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json");

    JsonDocument doc;
    doc["mac_address"] = mac; 
    doc["major_id"]    = major;
    doc["minor_id"]    = minor;
    doc["event_type"]  = event;
    doc["timestamp"]   = time_iso;

    String jsonString;
    serializeJson(doc, jsonString);

    Serial.println("Sending JSON: " + jsonString);
    int httpResponseCode = http.POST(jsonString);

    if(httpResponseCode > 0){
      Serial.print("Response Code: "); 
      Serial.println(httpResponseCode);
    } else {
      Serial.print("Error sending POST: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  } else {
    Serial.println("Error: WiFi lost");
  }
}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&espNowPayload, incomingData, sizeof(espNowPayload));
  
  Serial.println("\n>>> DATA RECEIVED via ESP-NOW");
  Serial.printf("Student: %d, Event: %s\n", espNowPayload.minor_id, espNowPayload.event_type);

  sendAttendanceToRails("80:F3:DA:5E:05:84", // Pass Scanner MAC as source
                        espNowPayload.major_id, 
                        espNowPayload.minor_id, 
                        espNowPayload.event_type, 
                        espNowPayload.timestamp_iso);
}

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, password);
  
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to Manganese!");
  Serial.println(WiFi.localIP());

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
  Serial.println("Receiver Ready.");
}

void loop() {
  delay(1000);
}