#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h> 
#include <esp_now.h>

#define BUTTON_PIN 0  

const char* ssid = "UnknownDevice";
const char* password = "12345678";
const char* mqtt_server = "broker.mqtt.cool";
const int mqtt_port = 1883;
const char* mqtt_client_name = "smarthomeAditya12";
const char* mqtt_topic = "trigger";          
const char* mqtt_topic_callback = "MS_ack";  

WiFiClient espClient;
PubSubClient client(espClient);
String message_service;

// inisialisasi Mac Address Service device
uint8_t Receiver_Address1[] = { 0xB8, 0xD6, 0x1A, 0x40, 0xF8, 0x8C };
uint8_t Receiver_Address2[] = { 0x7C, 0x9E, 0xBD, 0x37, 0xCA, 0x84 };
uint8_t Receiver_Address3[] = { 0x84, 0xCC, 0xA8, 0x5E, 0x52, 0x44 };

typedef struct broadcast_message {
  int device_id;
  int device_value;
} broadcast_message;
broadcast_message message;

esp_now_peer_info_t peerInfo;
static unsigned int state;
bool MQTT_connected = false;

unsigned long timeout = 5000;
unsigned long previousMillis = 0;
unsigned long startTime = 0;

// variabel tombol
int lastState = HIGH;
int currentState;

//variabel for callback
int LocalMode = 0;
//callback variable
int device_id;
//define uid device
int uid = 2;

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  //inisialisasi wifi mqtt
  WiFi.begin(ssid, password);

  //pairing esp device
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_send_cb(data_sent);
  memcpy(peerInfo.peer_addr, Receiver_Address1, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer 1 ");
    delay(2000);
    return;
  }

  //mulai wifi
  initMQTT();
  state = 1;
}

void loop() {
  switch (state) {
    case 1:
      if (MQTT_connected) {
        state = 3;
      } else if (!MQTT_connected) {
        state = 2;
      }
      break;
    case 2:  // (ESP_NOW MODE)
      WiFi.disconnect();
      button_def();
      break;
    case 3:  //MQTT MODE
      if (client.connected()) {
        button();
      } else {
        // Reconnect to MQTT broker if the connection is lost
        if (client.connect(mqtt_client_name)) {
          Serial.println("Reconnected to MQTT Broker");
          client.subscribe(mqtt_topic_callback);
        } else {
          Serial.println("Failed to reconnect to MQTT Broker_1");
          state = 2;
        }
      }
      if (WiFi.status() != WL_CONNECTED) {
        state = 2;
      }
      client.loop();
      break;
  }
}

void initMQTT() {
  Serial.println("Connecting to WiFi...");
  unsigned long startMillis = millis();
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
    if (millis() - startMillis > timeout) {
      Serial.println("\nFailed to connect to WiFi. Switching to ESP-NOW mode.");
      MQTT_connected = false;
      return;
    }
  }
  Serial.println("\nConnected to WiFi");

  // Menghubungkan ke broker MQTT
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  if (client.connect(mqtt_client_name)) {
    Serial.println("Connected to MQTT Broker");
    client.subscribe(mqtt_topic_callback);
    MQTT_connected = true;
  } else {
    Serial.println("Failed to connect to MQTT Broker");
    MQTT_connected = false;
  }
}
