void button_def() {
  currentState = digitalRead(BUTTON_PIN);
  if (lastState == HIGH && currentState == LOW) {
    Serial.println("tombol br ditekan");
    message.device_id = uid;
    message.device_value = 1;
    esp_now_send(0, (uint8_t *)&message, sizeof(broadcast_message));
    delay(2000);
  } else if (lastState == LOW && currentState == HIGH) {
    Serial.println("tombol br dilepas");
    message.device_id = uid;
    message.device_value = 0;
    esp_now_send(0, (uint8_t *)&message, sizeof(broadcast_message));
  }
  lastState = currentState;
}