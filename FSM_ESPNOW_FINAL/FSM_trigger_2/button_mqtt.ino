void button() {
  currentState = digitalRead(BUTTON_PIN);
  if (lastState == HIGH && currentState == LOW) {
    Serial.println("tombol mqtt ditekan");

    //kirim json
    StaticJsonDocument<200> jsonDoc;
    jsonDoc["device_id"] = 1;
    jsonDoc["device_value"] = 1;
    String jsonString;
    serializeJson(jsonDoc, jsonString);

    // Kirim data JSON ke broker MQTT
    if (client.publish(mqtt_topic, jsonString.c_str())) {
      Serial.println("publish topik trigger");
      startTime = millis();
      LocalMode = true;
    } else {
      Serial.println("Failed to publish trigger");
    }

    // Memeriksa pesan dari topik ack selama 5 detik
    while (millis() - startTime < timeout) {
      client.loop();
      if (!LocalMode) {
        Serial.println("Data diterima");
        Serial.println(device_id);
        break;
      }
      delay(100);
    }
    if (LocalMode) {
      Serial.println("Timeout! tidak ada data yang diterima ");
    }
  } else if (lastState == LOW && currentState == HIGH) {
    Serial.println("tombol mqtt dilepas");
  }
  lastState = currentState;
}
