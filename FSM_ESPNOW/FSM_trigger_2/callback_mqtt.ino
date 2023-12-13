void callback(char *topic, byte *payload, unsigned int length) {
  Serial.print("Message received on topic: ");
  Serial.println(topic);

  if (strcmp(topic, mqtt_topic_callback) == 0) {  //menerima data "service"
    // Membaca payload pesan
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
    device_id = jsonDoc["device_id"];
    if (device_id == uid) {
      Serial.print("device_ack :");
      Serial.println(device_id);
      ESPNOWMODE = 1;
    }
  }
}
