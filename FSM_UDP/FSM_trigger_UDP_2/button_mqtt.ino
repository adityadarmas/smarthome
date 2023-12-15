

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
    if (client.publish(mqtt_topic, jsonString.c_str())) {
      Serial.println("publish topik trigger");
      startTime = millis();
      ESPNOWMODE = true;
    } else {
      Serial.println("Failed to publish trigger");
    }

    // Memeriksa pesan dari topik ack selama 5 detik
    while (millis() - startTime < timeout) {
      client.loop();

      // Jika ackReceived menjadi true, keluar dari loop
      if (!ESPNOWMODE) {
        Serial.println("Data diterima");
        Serial.println(device_id);
        break;
      }

      delay(100);  // Tunggu sebentar untuk mencegah busy-waiting
    }

    // Jika timeout tercapai
    if (ESPNOWMODE) {
      Serial.println("Timeout! tidak ada data yang diterima ");
    }
  }
   else if (lastState == LOW && currentState == HIGH) {
     Serial.println("tombol mqtt dilepas");
     //digitalWrite(LED_BUILTIN, LOW);
     delay(1000);
   }
  // save the last state
  lastState = currentState;
}
