#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <EEPROM.h>

#define EEPROM_SIZE 200

int uid = 4;

String wifiSSID = "Honor 8X";
String wifiPassword = "abcdefgh";
String mqttbroker = "test.mosquitto.org";
WiFiClient client;
const char* mqtt_service = "service";
const char* mqtt_rule = "rule";
PubSubClient mqtt(client);

StaticJsonDocument<200> serviceDoc;

int r_id[1];
int t_id[3];
int t_val[3];
int s_id[1];
int s_val[2];

int s_value;
int t_index = 0;

int getNumberOfTId() {
    return t_index;
}

void connectWifi();
void connect_mqtt();

void callback(char* topic, byte* payload, unsigned int length) {
  // Callback ini akan dipanggil ketika pesan MQTT diterima
  Serial.print("Data dari topik : ");
  Serial.println(topic);

  t_index = 0; //reset t_id count

  if (strcmp(topic, mqtt_rule) == 0) {  //menerima data "rule"
    String message_rule;
    for (int i = 0; i < length; i++) {
      message_rule += (char)payload[i];
    }

    Serial.println(message_rule);

    // Parsing pesan JSON
    StaticJsonDocument<200> jsonDoc;  
    DeserializationError error = deserializeJson(jsonDoc, message_rule);
      if (error) {
      Serial.print("Failed to parse JSON: ");
      Serial.println(error.c_str());
      return;
    }

    JsonObject trigger = jsonDoc["trigger"];
    JsonObject servis = jsonDoc["servis"];

    int rule_id = jsonDoc["rule_id"]; // 1

      // Iterate over trigger and extract values
      for (JsonPair kv : trigger) {
        t_id[t_index] = atoi(kv.key().c_str()); // Convert the key to an integer
        t_val[t_index] = kv.value().as<int>();
        t_index++;
      }

      // Iterate over servis and extract values
      int s_id[2];  // Assuming you have 2 elements in the servis object
      int s_val[2];
      int s_index = 0;
      for (JsonPair kv : servis) {
        s_id[s_index] = atoi(kv.key().c_str()); // Convert the key to an integer
        s_val[s_index] = kv.value().as<int>();
        if (s_id[s_index] == uid) {
          s_index++;
          break; // Break the loop when the first matching s_id is found
        }
      }

      if (s_index == 0) {
        Serial.println("No matching s_id found. Ignoring data.");
        return;
      }

      int eepromAddress = 0;  // Start from address 0

      // Add a break value (254) at the beginning
      EEPROM.write(eepromAddress, 254);
      EEPROM.commit();
      eepromAddress += sizeof(byte);

      // Store t_index in EEPROM
      EEPROM.writeInt(eepromAddress, t_index);
      EEPROM.commit();
      eepromAddress += sizeof(int);

      for (int i = 0; i < t_index; i++) {
        // Store t_id and t_val in EEPROM
        EEPROM.writeInt(eepromAddress, t_id[i]);
        EEPROM.writeInt(eepromAddress + sizeof(int), t_val[i]);
        EEPROM.commit();
        eepromAddress += 2 * sizeof(int);
      }

      // Store s_val in EEPROM
      EEPROM.writeInt(eepromAddress, s_val[0]);
      EEPROM.commit();



      Serial.println("Data diterima:");
      Serial.print("r_id: ");
      Serial.println(rule_id);
      for (int i = 0; i < t_index; i++) {
        Serial.print("t_id: ");
        Serial.println(t_id[i]);
        Serial.print("t_val: ");
        Serial.println(t_val[i]);
      }

      // Print matching s_id values
      for (int i = 0; i < s_index; i++) {
        if (s_id[i] == uid) {
          Serial.print("s_id: ");
          Serial.println(s_id[i]);
          Serial.print("s_val: ");
          Serial.println(s_val[i]);
        }
      }

        Serial.print("Number of t_id received: ");
        Serial.println(getNumberOfTId());
        readData();
  
  } else if (strcmp(topic, mqtt_service) == 0) {  //menerima data "service"
    String message_service;
    for (int i = 0; i < length; i++) {
      message_service += (char)payload[i];
    }

    //menampilkan data rule dalam bentuk json
    Serial.println(message_service);

    StaticJsonDocument<200> jsonDoc;
    DeserializationError error = deserializeJson(jsonDoc, message_service);
    if (error) {
      Serial.print("Failed to parse JSON: ");
      Serial.println(error.c_str());
      return;
    }
  int s_id = jsonDoc["s_id"]; // 4
  int s_val = jsonDoc["s_val"]; // 1

    // Tampilkan data yang diterima
   if(s_id != uid){ 
    Serial.println("ID tidak sama");
   }
   else{
    Serial.println("Data diterima:");
    Serial.print("s_id: ");
    Serial.println(s_id);
    Serial.print("s_val: ");
    Serial.println(s_val);
   }
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  connectWifi();
  mqtt.setServer(mqttbroker.c_str(), 1883);
  mqtt.setCallback(callback);
  connect_mqtt();

  EEPROM.begin(EEPROM_SIZE);
}

void readData() {
  int eepromAddress = 0;  // Start from address 0

  // Read the break value (254) from EEPROM
  byte breakValue = EEPROM.read(eepromAddress);
  if (breakValue != 254) {
    Serial.println("Error: Break value not found at the beginning.");
    return;  // Exit the function if the break value is not found
  }
  eepromAddress += sizeof(byte);

  // Read t_index from EEPROM
  int storedTIndex = EEPROM.readInt(eepromAddress);
  eepromAddress += sizeof(int);
  Serial.print("Stored t_index: ");
  Serial.println(storedTIndex);

  // Read t_id and t_val from EEPROM
  for (int i = 0; i < storedTIndex; i++) {
    int storedTId = EEPROM.readInt(eepromAddress);
    int storedTVal = EEPROM.readInt(eepromAddress + sizeof(int));
    
    Serial.print("Stored t_id: ");
    Serial.println(storedTId);
    Serial.print("Stored t_val: ");
    Serial.println(storedTVal);

    eepromAddress += 2 * sizeof(int);
  }

  // Read s_val from EEPROM
  int storedSVal = EEPROM.readInt(eepromAddress);
  Serial.print("Stored s_val: ");
  Serial.println(storedSVal);
}

void loop() {
  // put your main code here, to run repeatedly:
  mqtt.loop();
  /*if()
  {
  s_value = EEPROM.read(5);
    Serial.println(s_value);
  }*/
}  

void connect_mqtt(){
  while(!mqtt.connected()){
    Serial.println("Connecting.....");
    if(mqtt.connect("esp99")){
      Serial.println("MQTT Connected");
      mqtt.subscribe(mqtt_service);
      mqtt.subscribe(mqtt_rule);
    }
    delay(1000);
  }
}

void connectWifi(){
  Serial.println("Connecting To Wifi");
  WiFi.begin(wifiSSID.c_str(), wifiPassword.c_str());
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(500);
  }
   Serial.println("Wifi Connected");
    Serial.println(WiFi.SSID());
    Serial.println(WiFi.RSSI());
    Serial.println(WiFi.macAddress());
    Serial.println(WiFi.localIP());
    Serial.println(WiFi.gatewayIP());
    Serial.println(WiFi.dnsIP());
  }

