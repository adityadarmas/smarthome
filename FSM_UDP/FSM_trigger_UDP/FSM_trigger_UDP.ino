#include <WiFi.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>  // Library untuk bekerja dengan JSON
#include <esp_now.h>

#define BUTTON_PIN 0  //button pin definition
#define LED_BUILTIN 2

const char* ssid = "UnknownDevice";
const char* password = "12345678";
const char* mqtt_server = "broker.mqtt.cool-";
const int mqtt_port = 1883;
const int udpPort = 55555;
const char* mqtt_client_name = "smarthomeAditya12";
const char* mqtt_topic = "trigger";          // ganti topik trigger
const char* mqtt_topic_callback = "MS_ack";  // ganti topik trigger

WiFiUDP udp;
WiFiClient espClient;
PubSubClient client(espClient);
String message_service;

// ESP-NOW setup
uint8_t Receiver_Address1[] = { 0xB8, 0xD6, 0x1A, 0x40, 0xF8, 0x8C };  //ganti dengan slave mac
uint8_t Receiver_Address2[] = { 0x7C, 0x9E, 0xBD, 0x37, 0xCA, 0x84 };
uint8_t Receiver_Address3[] = { 0x84, 0xCC, 0xA8, 0x5E, 0x52, 0x44 };

typedef struct broadcast_message {
  int device_id;
  int device_val;
} broadcast_message;
broadcast_message message;

esp_now_peer_info_t peerInfo;
static unsigned int state;

unsigned long wifiTimeout = 5000;  // 5 seconds
unsigned long previousMillis = 0;  // millis() returns an unsigned long.
const long interval = 20000;       //jalankan ESP-NOW mode selama 20 detik

// variabel button change:
int lastState = HIGH;  // the previous state from the input pin
int currentState;      // the current reading from the input pin

// bool MQTT_connect = false;
bool WiFi_connected = false;
bool ESP_init = false;

//variabel for callback

const unsigned long timeout = 3000;
int ESPNOWMODE = 0;

//callback variable
int device_id;

//define uid device
int uid = 2;

void setup() {
  Serial.begin(115200);

  // initialize the pushbutton pin as an pull-up input
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);

  //inisialisasi wifi mqtt
  WiFi.begin(ssid, password);

  //pairing esp device
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    digitalWrite(LED_BUILTIN, HIGH);
    return;
  }
  digitalWrite(LED_BUILTIN, LOW);
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
  unsigned long startMillis = millis();
  unsigned long currentMillis = millis();
  switch (state) {
    case 1:
      if (!WiFi_connected) {
        state = 2;  //jika wifi tidak terhubung
      } else {
        state = 3;  // wifi terhubung
      }
      break;
    case 2:  // (UDP MODE) jika wifi tidak terhubung, terima dari UDP
      button_def();

      //jika mqtt tidak terhubung kembali ke mode espnow
      if (client.connect(mqtt_client_name)) {
        Serial.println("Connected to MQTT Broker");
        client.subscribe(mqtt_topic_callback);
        state = 3;
      }

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
      client.loop();
      break;
    case 4:  //callbackmode
      unsigned long startTime = 0;
      startTime = millis();
      Serial.println(ESPNOWMODE);
      while (ESPNOWMODE != 1) {  //jika data ack belum diterima
        Serial.println("waiting ack");
        delay(1000);
        if (ESPNOWMODE == 1) {
          Serial.print("device_ack :");
          Serial.println(device_id);
          break;
        } else if (millis() - startTime > timeout) {  //setelah 5 detik jika masih belum ada data
          Serial.println("masuk ke ESPNOWMODE");      //pindah esp mode
          Serial.print("device_ack :");
          Serial.println(device_id);

          //state = 2;
          return;
        }
      }
      state = 3;
      break;
  }
}

void initMQTT() {
  Serial.println("Connecting to WiFi...");

  unsigned long startMillis = millis();

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
    if (millis() - startMillis > wifiTimeout) {
      Serial.println("\nFailed to connect to WiFi.");
      //WiFi_connected = false;
      return;
    }
  }

  Serial.println("\nConnected to WiFi");
  //WiFi_connected = true;

  // Menghubungkan ke broker MQTT
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  if (client.connect(mqtt_client_name)) {
    Serial.println("Connected to MQTT Broker");
    client.subscribe(mqtt_topic_callback);
    WiFi_connected = true;
  } else {
    Serial.println("Failed to connect to MQTT Broker");
    WiFi_connected = false;
  }
}
