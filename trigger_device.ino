#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>  // Library untuk bekerja dengan JSON

#define BUTTON_PIN 14  //button pin definition
#define LED_BUILTIN 2

const char* ssid = "UnknownDevice";
const char* password = "10012002";
const char* mqtt_server = "mqtt-dashboard.com";
const int mqtt_port = 1883;
const char* mqtt_client_name = "smarthomeAditya12";
const char* mqtt_topic = "topik/service";  // ganti topik trigger

WiFiClient espClient;
PubSubClient client(espClient);

// variabel button change:
int lastState = HIGH;  // the previous state from the input pin
int currentState;      // the current reading from the input pin


void setup() {
  Serial.begin(115200);

  // initialize the pushbutton pin as an pull-up input
  pinMode(BUTTON_PIN, INPUT_PULLUP);
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
  if (client.connect(mqtt_client_name)) {
    Serial.println("Connected to MQTT Broker");
  } else {
    Serial.println("Failed to connect to MQTT Broker");
  }
}

void loop() {

  if (client.connected()) {
    button();
    delay(500);
  } else {
    // Reconnect to MQTT broker if the connection is lost
    if (client.connect(mqtt_client_name)) {
      Serial.println("Reconnected to MQTT Broker");
    } else {
      Serial.println("Failed to reconnect to MQTT Broker");
    }
  }

  client.loop();
}

void button() {
  // read the state of the switch/button:
  currentState = digitalRead(BUTTON_PIN);
  if (lastState == HIGH && currentState == LOW) {
    Serial.println("The button is pressed");
    digitalWrite(LED_BUILTIN, HIGH);

    // Membuat objek JSON
    StaticJsonDocument<200> jsonDoc;  // Ukuran buffer JSON

    // Menambahkan data ke JSON
    jsonDoc["s_id"] = 3; //ganti t_id
    jsonDoc["s_val"] = 1; //ganti t_val

    // Serialize JSON ke string
    String jsonString;
    serializeJson(jsonDoc, jsonString);

    // Kirim data JSON ke broker MQTT
    client.publish(mqtt_topic, jsonString.c_str());
  }
  else if (lastState == LOW && currentState == HIGH) {
    Serial.println("The button is released");
  }
  // save the last state
  lastState = currentState;
}
