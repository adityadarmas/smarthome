#include <WiFi.h>
#include <WiFiUdp.h>
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
const char* mqtt_server = "broker.mqtt.cool-";
const int mqtt_port = 1883;
const int udpPort = 55555;  // Ganti dengan port yang Anda inginkan
const char* mqtt_client_name = "smarthomeAditya";
const char* mqtt_topic_rule = "rule";        // Topik MQTT yang sama dengan pengirim
const char* mqtt_topic_service = "service";  // Topik MQTT yang sama dengan pengirim
const char* mqtt_topic_ack = "service_ack";  // Topik MQTT yang sama dengan pengirim

WiFiUDP udp;
WiFiClient espClient;
PubSubClient client(espClient);

//variable UDP
int packetSize;
char packetBuffer[255];
int len;

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

//variable for save eeprom check (s_val)
int s_val_broadcast;

void setup() {
  Serial.begin(115200);
  EEPROM.begin(eeprom_size);
  pinMode(LED_BUILTIN, OUTPUT);

  //inisialisasi wifi mqtt
  WiFi.begin(ssid, password);
  udp.begin(udpPort);


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
        state = 3;  // wifi terhubung
      }
      break;
    case 2:  // (ESP_NOW MODE) jika wifi tidak terhubung, terima dari esp now

      // Menerima data UDP
      packetSize = udp.parsePacket();
      Serial.println(packetSize);
      if (packetSize) {
        len = udp.read((char*)&message, sizeof(broadcast_message));
        if (len > 0) {
          Serial.print("Received Device ID: ");
          Serial.println(message.device_id);
          Serial.print("Received Device Value: ");
          Serial.println(message.device_val);
          Serial.println();
        }
      }

      //masukkan ke dalam fungsi match data untuk membaca program eeprom
      s_br = matchData(message.device_id, message.device_val);
      Serial.print("eksekusi data dari eeprom dengan data: ");
      Serial.println(s_br);
      if (s_br == 1) {
        Serial.println("data terdapat di eeprom");
        // digitalWrite(LED_BUILTIN, HIGH);
      } else {
        Serial.println("data tidak ada");
      }

      //jika mqtt terhubung
      if (client.connect(mqtt_client_name)) {
        Serial.println("Connected to MQTT Broker");
        client.subscribe(mqtt_topic_rule);     // Subscribe ke topik yang sama dengan pengirim
        client.subscribe(mqtt_topic_service);  // Subscribe ke topik yang sama dengan pengirim
        state = 3;
      }

      // Menunggu sebentar sebelum membaca data lagi
      delay(1000);
      break;

    case 3:  //MQTT MODE
      client.loop();

      //jika wifi tidak terhubung kembali ke mode espnow
      if (!client.connect(mqtt_client_name)) {
        Serial.println("Failed to connect to MQTT Broker");
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
      Serial.println("\nFailed to connect to WiFi");
      //WiFi_connected = false;
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
