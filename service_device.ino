#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>  // Library untuk bekerja dengan JSON
#include <EEPROM.h>


// #define EEPROM_SIZE //cari tau size eeprom
#define LED_BUILTIN 2

int dev_id = 3;  // deskripsikan id esp

const char* ssid = "UnknownDevice";
const char* password = "10012002";
const char* mqtt_server = "mqtt-dashboard.com";
const int mqtt_port = 1883;
const char* mqtt_client_name = "smarthomeAditya";
const char* mqtt_topic_rule = "topik/rule";        // Topik MQTT yang sama dengan pengirim
const char* mqtt_topic_service = "topik/service";  // Topik MQTT yang sama dengan pengirim

WiFiClient espClient;
PubSubClient client(espClient);

int t_id[1];
int t_val[1];
int s_id[1];
int s_val[1];
int s_id_recv[1];
int s_val_recv[1];

void callback(char* topic, byte* payload, unsigned int length) {
  // Callback ini akan dipanggil ketika pesan MQTT diterima
  Serial.print("pesan dari topik : ");
  Serial.println(topic);

  if (strcmp(topic, mqtt_topic_rule) == 0) {  //menerima data "rule"
    // Membaca payload pesan
    String message_rule;
    for (int i = 0; i < length; i++) {
      message_rule += (char)payload[i];
    }

    //menampilkan data rule dalam bentuk json
    Serial.println(message_rule);

    // Parsing pesan JSON
    StaticJsonDocument<200> jsonDoc;  // Ukuran buffer JSON
    DeserializationError error = deserializeJson(jsonDoc, message_rule);
    if (error) {
      Serial.print("Gagal parsing JSON: ");
      Serial.println(error.c_str());
      return;
    }

    // Mendapatkan nilai dari JSON
    t_id[0] = jsonDoc["t_id"];
    t_val[0] = jsonDoc["t_val"];
    s_id[0] = jsonDoc["s_id"];
    s_val[0] = jsonDoc["s_val"];

    if (dev_id == s_id[0]) {
      // Tampilkan data yang diterima
      Serial.println("Data diterima:");
      Serial.print("t_id: ");
      Serial.println(t_id[0]);
      Serial.print("t_val: ");
      Serial.println(t_val[0]);
      Serial.print("s_id: ");
      Serial.println(s_id[0]);
      Serial.print("s_val: ");
      Serial.println(s_val[0]);

      //simpan data dalam eeprom
      // EEPROM.write(0, t_id[0]);
      // EEPROM.write(1, t_val[0]);
      // EEPROM.write(2, s_id[0]);
      // EEPROM.write(3, s_val[0]);
      // EEPROM.commit();
      //simpan data di eeprom
    }



  } else if (strcmp(topic, mqtt_topic_service) == 0) {  //menerima data "service"
    // Membaca payload pesan
    String message_service;
    for (int i = 0; i < length; i++) {
      message_service += (char)payload[i];
    }

    //menampilkan data rule dalam bentuk json
    Serial.println(message_service);

    // Parsing pesan JSON
    StaticJsonDocument<200> jsonDoc;  // Ukuran buffer JSON
    DeserializationError error = deserializeJson(jsonDoc, message_service);
    if (error) {
      Serial.print("Gagal parsing JSON: ");
      Serial.println(error.c_str());
      return;
    }

    // Mendapatkan nilai dari JSON
    s_id_recv[0] = jsonDoc["s_id"];
    s_val_recv[0] = jsonDoc["s_val"];

    // Tampilkan data yang diterima
    Serial.println("Data diterima:");
    Serial.print("s_id: ");
    Serial.println(s_id_recv[0]);
    Serial.print("s_val: ");
    Serial.println(s_val_recv[0]);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);

  // Menghubungkan ke jaringan WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Menghubungkan ke broker MQTT
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);  // Mengatur callback yang akan dipanggil ketika pesan diterima

  if (client.connect(mqtt_client_name)) {
    Serial.println("Connected to MQTT Broker");
    client.subscribe(mqtt_topic_rule);     // Subscribe ke topik yang sama dengan pengirim
    client.subscribe(mqtt_topic_service);  // Subscribe ke topik yang sama dengan pengirim
  } else {
    Serial.println("Failed to connect to MQTT Broker");
  }
}

void loop() {
  client.loop();
  if (s_val_recv[0] == 1){
    digitalWrite(LED_BUILTIN, HIGH);
  }else{
    digitalWrite(LED_BUILTIN, LOW); 
  }
}
