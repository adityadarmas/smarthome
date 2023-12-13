

void button() {
  // read the state of the switch/button:
  currentState = digitalRead(BUTTON_PIN);
  if (lastState == HIGH && currentState == LOW) {
    Serial.println("tombol mqtt ditekan");
    //digitalWrite(LED_BUILTIN, HIGH);

    // Membuat objek JSON
    StaticJsonDocument<200> jsonDoc;  // Ukuran buffer JSON

    // Menambahkan data ke JSON
    jsonDoc["device_id"] = uid;
    jsonDoc["device_value"] = 0;

    // Serialize JSON ke string
    String jsonString;
    serializeJson(jsonDoc, jsonString);

    // Kirim data JSON ke broker MQTT
    client.publish(mqtt_topic, jsonString.c_str());
    
    state = 5;
  }
  //  else if (lastState == LOW && currentState == HIGH) {
  //    Serial.println("tombol mqtt dilepas");
  //    digitalWrite(LED_BUILTIN, LOW);
  //    delay(1000);
  //  }
  // save the last state
  lastState = currentState;
}
