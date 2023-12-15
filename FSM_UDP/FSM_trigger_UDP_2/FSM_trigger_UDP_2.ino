#include <WiFi.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>  // Library untuk bekerja dengan JSON

#define BUTTON_PIN 0  //button pin definition
#define LED_BUILTIN 2

const char* ssid = "UnknownDevice";
const char* password = "12345678";
const char* mqtt_server = "broker.mqtt.cool";
const int mqtt_port = 1883;
const int udpPort = 55555;
const char* mqtt_client_name = "smarthomeAditya12";
const char* mqtt_topic = "trigger";          // ganti topik trigger
const char* mqtt_topic_callback = "MS_ack";  // ganti topik trigger

//wifi protocol init
WiFiUDP udp;
WiFiClient espClient;
PubSubClient client(espClient);

//variabel untuk menyimpan data yang diterima melalui mqtt
String message_service;

//struct untuk menyimpan data mqtt
typedef struct broadcast_message {
  int device_id;
  int device_val;
} broadcast_message;
broadcast_message message;

//definisi state
static unsigned int state;

unsigned long wifiTimeout = 5000;  // 5 seconds
unsigned long previousMillis = 0;  // millis() returns an unsigned long.
const long interval = 20000;       //jalankan ESP-NOW mode selama 20 detik

// variabel button change:
int lastState = HIGH;  // the previous state from the input pin
int currentState;      // the current reading from the input pin

//flag untuk MQTT
bool MQTT_connected = false;

//variabel for callback
unsigned long startTime = 0;
const unsigned long timeout = 5000;
bool ESPNOWMODE = true;

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

  //mulai wifi
  initMQTT();
  state = 1;
}

void loop() {
  unsigned long startMillis = millis();
  // unsigned long currentMillis = millis();
  switch (state) {
    case 1:
      if (!MQTT_connected) {
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
