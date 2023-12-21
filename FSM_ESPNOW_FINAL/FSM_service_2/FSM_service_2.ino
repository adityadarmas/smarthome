#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include <esp_now.h>

#define LED_BUILTIN 2
#define eeprom_size 512

//struct for eeprom
struct RuleData {
  int rule_id;
  struct {
    int t_id[3];
    int t_val[3];
  } trigger;
  struct {
    int s_id[3];
    int s_val[3];
  } service;
};

//broadcast struct
typedef struct broadcast_message {
  int device_id;
  int device_val;
} broadcast_message;
broadcast_message message;

//device id
const int uid = 3;
int s_br;             //menyimpan fungsi data cocok
int s_val_broadcast;  //menyimpan s_val yang cocok

const char* ssid = "UnknownDevice";
const char* password = "12345678";
const char* mqtt_server = "broker.mqtt.cool";
const int mqtt_port = 1883;
const char* mqtt_client_name = "smarthomeAditya";
const char* mqtt_topic_rule = "rule";        
const char* mqtt_topic_service = "service";  
const char* mqtt_topic_ack = "service_ack";  

WiFiClient espClient;
PubSubClient client(espClient);

//variable for mqtt recv
int s_id_recv[1];
int s_id[1];
int s_val_recv[1];
int s_val[1];

//flag state
bool MQTT_connected = false;
static unsigned int state;

//time variable
unsigned long timeout = 5000;      
unsigned long previousMillis = 0;  

// variabel button change:
int lastState = HIGH;  // the previous state from the input pin
int currentState;      // the current reading from the input pin


void setup() {
  Serial.begin(115200);
  EEPROM.begin(eeprom_size);
  pinMode(LED_BUILTIN, OUTPUT);

  //inisialisasi wifi mqtt
  WiFi.begin(ssid, password);

  //inisialisasi ESP NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
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
      Serial.println("ESp-now mode");
      WiFi.disconnect();
      esp_now_register_recv_cb(data_receive);

      // fungsi pencocokan data eeprom
      s_br = matchData(message.device_id, message.device_val);
      if (s_br == 1) {
        Serial.println("data terdapat di eeprom");
        digitalWrite(LED_BUILTIN, HIGH);
      } else{
        digitalWrite(LED_BUILTIN, LOW);
      }

      delay(1000);
      break;
    case 3:  //MQTT MODE
      client.loop();
      if (!client.connect(mqtt_client_name)) {
        Serial.println("Failed to connect to MQTT Broker");
        state = 2;
      }
      if (WiFi.status() != WL_CONNECTED) {
        state = 2;
      }
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

  Serial.println("Connected to WiFi");
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);  // Mengatur callback yang akan dipanggil ketika pesan diterima
  if (client.connect(mqtt_client_name)) {
    Serial.println("Connected to MQTT Broker");
    client.subscribe(mqtt_topic_rule);     // Subscribe ke topik yang sama dengan pengirim
    client.subscribe(mqtt_topic_service);  // Subscribe ke topik yang sama dengan pengirim
    MQTT_connected = true;
  } else {
    Serial.println("Failed to connect to MQTT Broker");
    MQTT_connected = false;
  }
}
