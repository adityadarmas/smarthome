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

int s_br;

const char* ssid = "UnknownDevice";
const char* password = "12345678";
const char* mqtt_server = "broker.mqtt.cool";
const int mqtt_port = 1883;
const char* mqtt_client_name = "smarthomeAditya";
const char* mqtt_topic_rule = "rule";        // Topik MQTT yang sama dengan pengirim
const char* mqtt_topic_service = "service";  // Topik MQTT yang sama dengan pengirim
const char* mqtt_topic_ack = "service_ack";  // Topik MQTT yang sama dengan pengirim

WiFiClient espClient;
PubSubClient client(espClient);

//variable for mqtt recv
int s_id_recv[1];
int s_id[1];
int s_val_recv[1];
int s_val[1];

//flag state
bool WiFi_connected = false;
bool ESP_init = false;
static unsigned int state;

//time variable
unsigned long wifiTimeout = 5000;  // 5 seconds
unsigned long previousMillis = 0;  // millis() returns an unsigned long.
const long interval = 20000;       //jalankan ESP-NOW mode selama 20 detik

// variabel button change:
int lastState = HIGH;  // the previous state from the input pin
int currentState;      // the current reading from the input pin

int s_val_broadcast;
void setup() {
  Serial.begin(115200);
  EEPROM.begin(eeprom_size);
  pinMode(LED_BUILTIN, OUTPUT);

  //inisialisasi wifi mqtt
  WiFi.begin(ssid, password);

  //inisialisasi ESP NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    digitalWrite(LED_BUILTIN, HIGH);
    return;
  }
  digitalWrite(LED_BUILTIN, LOW);

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
        state = 4;  // wifi terhubung
      }
      break;
    case 2:  // (ESP_NOW MODE) jika wifi tidak terhubung, terima dari esp now
      WiFi.disconnect();
      esp_now_register_recv_cb(data_receive);  //terima data esp-now

      //masukkan ke dalam fungsi match data untuk membaca program eeprom
      // s_br = matchData(message.device_id, message.device_val);
      // Serial.println(s_br);
      // if (s_br == 1) {
      //   Serial.println("data terdapat di eeprom");
      //   // digitalWrite(LED_BUILTIN, HIGH);
      // } else {
      //   Serial.println("data tidak ada");
      // }

      //jika telah 20 detik , check wifi
      if (currentMillis - previousMillis > interval) {
        //state check wifi = 3
        previousMillis = currentMillis;
        state = 3;
      }
      break;
    case 3:  //state check wifi
      WiFi.begin(ssid, password);

      while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(1000);
        if (millis() - startMillis > wifiTimeout) {
          Serial.println("\nFailed to connect to WiFi. Switching to ESP-NOW mode.");
          WiFi_connected = false;
          state = 2;
          return;
        }
      }
      WiFi_connected = true;
      state = 4;  // MQTT mode
      break;
    case 4:  //MQTT MODE
      client.loop();

      //jika wifi tidak terhubung kembali ke mode espnow
      if (WiFi.status() != WL_CONNECTED) {
        WiFi_connected = false;  //jika wifi terputus
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
    if (millis() - startMillis > wifiTimeout) {
      Serial.println("\nFailed to connect to WiFi. Switching to ESP-NOW mode.");
      WiFi_connected = false;
      return;
    }
  }

  Serial.println("Connected to WiFi");
  //WiFi_connected = true;
  // Menghubungkan ke broker MQTT
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);  // Mengatur callback yang akan dipanggil ketika pesan diterima

  if (client.connect(mqtt_client_name)) {
    Serial.println("Connected to MQTT Broker");
    client.subscribe(mqtt_topic_rule);     // Subscribe ke topik yang sama dengan pengirim
    client.subscribe(mqtt_topic_service);  // Subscribe ke topik yang sama dengan pengirim
    WiFi_connected = true;
  } else {
    Serial.println("Failed to connect to MQTT Broker");
    WiFi_connected = false;
  }
}
