void callback(char *topic, byte *payload, unsigned int length) {
  Serial.print("Message received on topic: ");
  Serial.println(topic);

  if (strcmp(topic, mqtt_topic_callback) == 0) {  //menerima data "service"
    // Membaca payload pesan
    for (int i = 0; i < length; i++) {
      message_service += (char)payload[i];
    }
    
    //parsing JSON
    StaticJsonDocument<200> jsonDoc;
    DeserializationError error = deserializeJson(jsonDoc, message_service);
    if (error) {
      Serial.print("Gagal parsing JSON: ");
      Serial.println(error.c_str());
      return;
    }

    //membaca pesan ack mqtt
    device_id = jsonDoc["device_id"];
    if (device_id == uid) {
      LocalMode = false;
    }
  }
}
