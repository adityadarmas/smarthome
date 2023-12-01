#include <WiFi.h>
#include <esp_now.h>

// Replace with your WiFi credentials
const char* ssid = "UnknownDevice";
const char* password = "12345678";

typedef struct struct_message {
  int device_id;
  int device_val;
} struct_message;

struct_message message;

bool WiFi_connected = false;
bool ESP_init = false;

unsigned long wifiTimeout = 5000;  // 5 seconds
unsigned long ESPTimeout = 10000;  // 10 seconds
unsigned long previousMillis = 0;

esp_now_peer_info_t peerInfo;

void setup() {
  Serial.begin(115200);

  connectToWiFi();
}

void loop() {

  if (!WiFi_connected) {
    if (!ESP_init) {
      initESPNow();
    } else {
      //Serial.println("ESP-NOW mode.");
      //unsigned long currentMillis = millis();
      // while (millis() - currentMillis < ESPTimeout) {
      // break
      // }
      if (WiFi.status() == WL_CONNECTED) {
        WiFi_connected = true;
      }
    }
  } else {
    if (WiFi.status() != WL_CONNECTED) {
      WiFi_connected = false;
      WiFi.begin(ssid, password);
    }
    Serial.println("wifi connected");
    delay(1000);
    //when WiFi is connected || mqtt
  }
}

void connectToWiFi() {
  Serial.println("Connecting to WiFi...");
  // WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  unsigned long startMillis = millis();

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    delay(1000);
    if (millis() - startMillis > wifiTimeout) {
      Serial.println("\nFailed to connect to WiFi. Switching to ESP-NOW mode.");
      WiFi_connected = false;
      initESPNow();
      delay(5000);
      return;
    }
  }

  Serial.println("\nConnected to WiFi");
  WiFi_connected = true;
}

void initESPNow() {
  Serial.println("Initializing ESP-NOW");
  esp_now_init();
  esp_now_register_recv_cb(onDataReceived);
  ESP_init = true;
}

void onDataReceived(const uint8_t* mac, const uint8_t* data, int len) {
  memcpy(&message, data, sizeof(message));
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.print("dev_id: ");
  Serial.println(message.device_id);
  Serial.print("dev_val: ");
  Serial.println(message.device_val);
  Serial.println();
}
